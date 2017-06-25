#include "threads.h"

#include <iostream>


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


void interpreter_context::create(uintptr_t _stack_start)
{
  ptr new_ctx(new interpreter_context(_stack_start, R_GlobalContext));
  
  std::lock_guard<std::recursive_mutex> lock(container_mutex());
  contexts().insert(make_pair(std::this_thread::get_id(), new_ctx));

  // insert yourself on the list
  new_ctx->link.next = chained_stacks.next;
  chained_stacks.next = &(new_ctx->link);
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



