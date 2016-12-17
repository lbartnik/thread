#include "rinterpreter.h"
#include "debug.h"
#include <iostream>

std::recursive_mutex & RInterpreterHandle::interpreter_mutex () {
  static std::recursive_mutex mutex;
  return mutex;
}

std::condition_variable & RInterpreterHandle::interpreter_condition () {
  static std::condition_variable condition;
  return condition;
}

RInterpreterHandle::contexts_type & RInterpreterHandle::interpreter_contexts () {
  static contexts_type contexts;
  return contexts;
}


interpreter_context & RInterpreterHandle::get_this_context ()
{
  contexts_type::iterator i;
  i = interpreter_contexts().find(std::this_thread::get_id());
  if (i != interpreter_contexts().end()) {
    return i->second;
  }
  
  std::cerr << "no context for thread " << std::this_thread::get_id() << std::endl;
  static interpreter_context no_context(0, 0);
  return no_context;
}


void RInterpreterHandle::init (uintptr_t _stack_start, RCNTXT * _global_context, SEXP * _pp_stack)
{
  if (DEBUG_THREADS) {
    std::cerr << "initializing interpreter context for thread "
              << std::this_thread::get_id()
              << "; stack_start=" << _stack_start << std::endl;
  }

  interpreter_context context(_stack_start, _global_context, _pp_stack);
  
  std::lock_guard<std::recursive_mutex> lock(interpreter_mutex());
  interpreter_contexts().insert(make_pair(std::this_thread::get_id(), context));
}


void RInterpreterHandle::destroy ()
{
  std::lock_guard<std::recursive_mutex> lock(interpreter_mutex());
 
  contexts_type::iterator i = interpreter_contexts().find(std::this_thread::get_id());
  if (i != interpreter_contexts().end()) {
    interpreter_contexts().erase(i);
  }
}


void RInterpreterHandle::claim ()
{
  interpreter_mutex().lock();
  get_this_context().count += 1;

  // if re-entering in the same thread, refresh settings
  if (get_this_context().count > 1) {
    get_this_context().pp_stack_top = R_PPStackTop;
  }
  // if entering from another thread, restore settings
  else {  
    R_PPStackTop  = get_this_context().pp_stack_top;
    R_CStackStart = get_this_context().stack_start;
    R_GlobalContext = get_this_context().global_context;
    R_PPStack = get_this_context().pp_stack;
  }
  
  if (DEBUG_THREADS) {
    std::cerr << "claiming interpreter context for thread "
              << std::this_thread::get_id()
              << "; stack_start=" << R_CStackStart << std::endl;
  }
}


void RInterpreterHandle::release (bool _notify)
{
  get_this_context().count -= 1;
  get_this_context().pp_stack_top = R_PPStackTop;
  get_this_context().global_context = R_GlobalContext;
  interpreter_mutex().unlock();
  
  // won't notify when called from library destructor
  // TODO what if there are threads that are still running?
  if (_notify) {
    interpreter_condition().notify_all();
  }
}
