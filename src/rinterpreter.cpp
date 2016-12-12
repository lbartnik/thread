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


void RInterpreterHandle::init (uintptr_t _stack_start)
{
  if (DEBUG_THREADS) {
    std::cerr << "initializing interpreter context for thread "
              << std::this_thread::get_id()
              << "; stack_start=" << _stack_start << std::endl;
  }

  std::lock_guard<std::recursive_mutex> lock(interpreter_mutex());
  interpreter_contexts().insert(make_pair(std::this_thread::get_id(),
                                interpreter_context(_stack_start, R_PPStackTop)));
}


void RInterpreterHandle::claim ()
{
  interpreter_mutex().lock();
    
  R_PPStackTop  = get_this_context().pp_stack_top;
  R_CStackStart = get_this_context().stack_start;

  if (DEBUG_THREADS) {
    std::cerr << "claiming interpreter context for thread "
              << std::this_thread::get_id()
              << "; stack_start=" << R_CStackStart << std::endl;
  }
}

