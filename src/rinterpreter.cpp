#include "rinterpreter.h"
#include "debug.h"
#include <iostream>
#include <algorithm>

std::recursive_mutex & RInterpreterLock::interpreter_mutex () {
  static std::recursive_mutex mutex;
  return mutex;
}

std::condition_variable & RInterpreterLock::interpreter_condition () {
  static std::condition_variable condition;
  return condition;
}


RInterpreterLock::context_guard RInterpreterLock::gil_enter ()
{
  interpreter_mutex().lock();
  interpreter_context & ctx = interpreter_context::get_this_context();
  
  // if re-entering in the same thread, dump state
  //
  // it seems that re-entering occurs when a thread owns GIL and
  // then issues a memory allocation request
  if (ctx.count > 0) {
    interpreter_context::assert_single_thread_in_gil();
    // abort();
  }
  ctx.enter();
  
  if (DEBUG_THREADS) {
    std::cerr << "claiming interpreter context for thread "
              << std::this_thread::get_id()
              << "; stack_start=" << R_CStackStart << std::endl;
  }

  return context_guard(ctx.global_context, ctx.link.top);
}


void RInterpreterLock::gil_leave ()
{
  interpreter_context & ctx = interpreter_context::get_this_context();
  ctx.leave();
  interpreter_mutex().unlock();
}

void RInterpreterLock::gil_leave (const RInterpreterLock::context_guard & _guard)
{
  interpreter_context & ctx = interpreter_context::get_this_context();
  if (context_guard(ctx.global_context, ctx.link.top) != _guard) {
    std::cerr << "contexts differ on gil_leave():\n"
              << "local vs. guard\n"
              << ctx.link.top << " vs. " << _guard.pp_stack_top << "\n"
              << ctx.global_context << " vs. " << _guard.global_context
              << std::endl;
    abort();
  }

  gil_leave();
}


interpreter_context::interpreter_context (uintptr_t _stack_start,
                                          chained_stack & _global_link,
                                          RCNTXT * _global_context,
                                          int _context_no)
  : stack_start(_stack_start),
    count(0),
    global_context(_global_context),
    link(_global_link),
    thread_id(std::this_thread::get_id()),
    context_number(_context_no)
{ }


static void insert_into_chain (chained_stack & _link)
{
  _link.next = chained_stacks.next;
  chained_stacks.next = &(_link);
}

interpreter_context::interpreter_context (uintptr_t _stack_start,
                                          RCNTXT * _global_context,
                                          int _context_no)
  : stack_start(_stack_start),
    count(0),
    global_context(_global_context),
    link(local_link),
    thread_id(std::this_thread::get_id()),
    context_number(_context_no)
{
  link.size = R_PPStackSize;
  link.top = 0;

  stack_data.resize(R_PPStackSize + PP_REDZONE_SIZE);
  link.stack = stack_data.data();
}


static int thread_count = 0;

void interpreter_context::create()
{
  ptr new_ctx(new interpreter_context(R_CStackStart,
                                      chained_stacks,
                                      R_GlobalContext,
                                      ++thread_count));
  
  chained_stacks.top = R_PPStackTop;

  std::lock_guard<std::recursive_mutex> lock(container_mutex());
  contexts().insert(make_pair(std::this_thread::get_id(), new_ctx));

  // IMPORTANT! don't put the main context into the chain, it already
  // there because it is the head
  // insert_into_chain(new_ctx->link);
}


void interpreter_context::create(uintptr_t _stack_start)
{
  ptr new_ctx(new interpreter_context(_stack_start, R_GlobalContext, ++thread_count));
  
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
  else {
    *ptr = (*ptr)->next;
  }

  // remove the context object
  auto i = contexts().find(std::this_thread::get_id());
  if (i != contexts().end()) {
    contexts().erase(i);
  }
}


void interpreter_context::dump_change ()
{
  std::cerr //<< "count = " << count << "\n"
            << "thread[ " << context_number << " ]\n"
            << "  global: "
            << "R_PPStack = " << R_PPStack << ", "
            << "R_PPStackTop = " << R_PPStackTop << ", "
            << "R_GlobalContext = " << std::hex << R_GlobalContext << std::dec << "\n"
            << "  this:   "
            << "R_PPStack = " << link.stack << ", "
            << "R_PPStackTop = " << link.top << ", "
            << "R_GlobalContext = " << std::hex << global_context << std::dec << "\n"
            << std::endl;
}


void interpreter_context::enter ()
{
  count += 1;
  // restore settings
  //
  // we can only enter this section if the thread does not own GIL
  // yet; if it does, the context is out-of-date, and R_PPStackTop
  // might be already updated; R_PPStack is correct, though
  //
  // TODO turn R_PPStack* into pointers to values in the current
  // link from "chained_stacks" 
  if (count == 1) {
    std::cerr << "\nenter()\n";
    dump_change();

    R_PPStack     = link.stack;
    R_PPStackTop  = link.top;
    R_CStackStart = stack_start;
    R_GlobalContext = global_context;
  }
}



void interpreter_context::leave ()
{
  count -= 1;
  if (count == 0)
  {
    std::cerr << "\nleave()\n";
    dump_change();

    link.top = R_PPStackTop;
    global_context = R_GlobalContext;
  }
}



interpreter_context::contexts_type & interpreter_context::contexts () {
  // it seems that this static variable is destroyed before
  // the library destructor is executed; so let's acllocate
  // it on the heap and pray
  static contexts_type * contexts = new contexts_type();
  return *contexts;
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
    auto id = std::this_thread::get_id(); // valgrind complains about invalid reads
    i = contexts().find(id);
    if (i != contexts().end()) {
      return *(i->second);
    }
  }
  
  std::cerr << "no context for thread " << std::this_thread::get_id() << std::endl;
  static interpreter_context no_context(0, 0, 0);
  return no_context;
}


void interpreter_context::dump_all ()
{
  std::lock_guard<std::recursive_mutex> lock(container_mutex());
  std::for_each(contexts().begin(), contexts().end(), [](auto & pair) {
    std::cout << "thread " << pair.second->thread_id
              << ": count=" << pair.second->count << std::endl; 
  });  
}

void interpreter_context::assert_single_thread_in_gil ()
{
  std::lock_guard<std::recursive_mutex> lock(container_mutex());

  int thread_count = 0;
  std::for_each(contexts().begin(), contexts().end(), [&](auto & pair) {
    if (pair.second->count) ++thread_count; 
  });
  
  if (thread_count < 2) return;

  std::cerr << "multiple threads own GIL: ";
  std::for_each(contexts().begin(), contexts().end(), [&](auto & pair) {
    if (pair.second->count) std::cerr << pair.first << " "; 
  });
  std::cerr << std::endl;
}

