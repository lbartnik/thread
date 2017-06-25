#include "threading.h"
#include "rinterpreter.h"
#include "memory.h"

__attribute__((constructor))
static void initialize_threading ()
{
  // version for the main thread only copies address and size
  interpreter_context::create();

  RInterpreterLock rInterpreter;
  rInterpreter.gil_enter();

  set_alloc_callback();
}

__attribute__((destructor))
static void teardown_threading ()
{
  reset_alloc_callback();
  
  // TODO make sure we never exit before all threads are joined()

  // TODO is it even needed to release synchronization variables?
  //      maybe their destructors take care of everything?
  RInterpreterLock rInterpreter;
  rInterpreter.gil_leave();
  interpreter_context::destroy();
}
