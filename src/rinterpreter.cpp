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


void RInterpreterHandle::claim ()
{
  interpreter_mutex().lock();
  interpreter_context & ctx = interpreter_context::get_this_context();
  
  ctx.count += 1;

  // if re-entering in the same thread, refresh settings
  if (ctx.count > 1) {
    ctx.link.top = R_PPStackTop;
  }
  // if entering from another thread, restore settings
  else {  
    R_PPStack     = ctx.link.stack;
    R_PPStackTop  = ctx.link.top;
    R_CStackStart = ctx.stack_start;
    R_GlobalContext = ctx.global_context;
  }
  
  if (DEBUG_THREADS) {
    std::cerr << "claiming interpreter context for thread "
              << std::this_thread::get_id()
              << "; stack_start=" << R_CStackStart << std::endl;
  }
}


void RInterpreterHandle::release (bool _notify)
{
  interpreter_context & ctx = interpreter_context::get_this_context();
 
  ctx.count -= 1;
  ctx.link.top = R_PPStackTop;
  ctx.global_context = R_GlobalContext;
  interpreter_mutex().unlock();
  
  // won't notify when called from library destructor
  // TODO what if there are threads that are still running?
  if (_notify) {
    interpreter_condition().notify_all();
  }
}

