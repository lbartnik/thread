#include "rinterpreter.h"
#include "debug.h"
#include <iostream>

std::recursive_mutex & RInterpreterLock::interpreter_mutex () {
  static std::recursive_mutex mutex;
  return mutex;
}

std::condition_variable & RInterpreterLock::interpreter_condition () {
  static std::condition_variable condition;
  return condition;
}


void RInterpreterLock::gil_enter ()
{
  interpreter_mutex().lock();
  interpreter_context & ctx = interpreter_context::get_this_context();
  
  ctx.count += 1;

  // if re-entering in the same thread, abort
  if (ctx.count > 1) {
    std::cerr << "entering gil twice\n" << std::endl;
    abort();
  }

  // restore settings
  R_PPStack     = ctx.link.stack;
  R_PPStackTop  = ctx.link.top;
  R_CStackStart = ctx.stack_start;
  R_GlobalContext = ctx.global_context;
  
  if (DEBUG_THREADS) {
    std::cerr << "claiming interpreter context for thread "
              << std::this_thread::get_id()
              << "; stack_start=" << R_CStackStart << std::endl;
  }
}


void RInterpreterLock::gil_leave (/* bool _notify */)
{
  interpreter_context & ctx = interpreter_context::get_this_context();
 
  ctx.count -= 1;
  ctx.link.top = R_PPStackTop;
  ctx.global_context = R_GlobalContext;
  interpreter_mutex().unlock();
  
  // TODO won't notify when called from library destructor
  // TODO what if there are threads that are still running?
  // if (_notify) {
    interpreter_condition().notify_all();
  // }
}


interpreter_context::interpreter_context (uintptr_t _stack_start,
                                          SEXP * _pp_stack_start,
                                          int _stack_top,
                                          int _stack_size,
                                          RCNTXT * _global_context)
  : stack_start(_stack_start),
    count(0),
    global_context(_global_context)
{
  link.size  = _stack_size;
  link.top   = _stack_top;
  link.stack = _pp_stack_start;
}


static void insert_into_chain (chained_stack & _link)
{
  _link.next = chained_stacks.next;
  chained_stacks.next = &(_link);
}

interpreter_context::interpreter_context (uintptr_t _stack_start,
                                          RCNTXT * _global_context)
  : stack_start(_stack_start),
    count(0),
    global_context(_global_context)
{
  link.size = R_PPStackSize;
  link.top = 0;

  stack_data.resize(R_PPStackSize + PP_REDZONE_SIZE);
  link.stack = stack_data.data();
}


void interpreter_context::create()
{
  ptr new_ctx(new interpreter_context(R_CStackStart,
                                      R_PPStack, R_PPStackTop,
                                      R_PPStackSize, R_GlobalContext));
  
  std::lock_guard<std::recursive_mutex> lock(container_mutex());
  contexts().insert(make_pair(std::this_thread::get_id(), new_ctx));

  insert_into_chain(new_ctx->link);
}


void interpreter_context::create(uintptr_t _stack_start)
{
  ptr new_ctx(new interpreter_context(_stack_start, R_GlobalContext));
  
  std::lock_guard<std::recursive_mutex> lock(container_mutex());
  contexts().insert(make_pair(std::this_thread::get_id(), new_ctx));

  insert_into_chain(new_ctx->link);
}


void interpreter_context::destroy ()
{
  std::lock_guard<std::recursive_mutex> lock(container_mutex());
  interpreter_context & ctx = get_this_context();

  // remote from the chained list
  chained_stack ** ptr = &chained_stacks.next;
  while (*ptr && *ptr != &ctx.link) {
    ptr = &(*ptr)->next;
  }
  if (!*ptr) {
    fprintf(stderr, "could not remove link from chain\n");
    fflush(stderr);
  }
  *ptr = (*ptr)->next;

  // remove the context object
  contexts().erase(std::this_thread::get_id());
}



interpreter_context::contexts_type & interpreter_context::contexts () {
  static contexts_type contexts;
  return contexts;
}


std::recursive_mutex & interpreter_context::container_mutex () {
  static std::recursive_mutex mutex;
  return mutex;
}



interpreter_context & interpreter_context::get_this_context ()
{
  {
    std::lock_guard<std::recursive_mutex> lock(container_mutex());
    contexts_type::iterator i;
    i = contexts().find(std::this_thread::get_id());
    if (i != contexts().end()) {
      return *(i->second);
    }
  }
  
  std::cerr << "no context for thread " << std::this_thread::get_id() << std::endl;
  static interpreter_context no_context(0, 0);
  return no_context;
}




