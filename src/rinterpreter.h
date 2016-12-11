#ifndef RINTERPRETER_H_GUARD
#define RINTERPRETER_H_GUARD


#include <condition_variable>
#include <mutex>
#include <thread>


//#include <Defn.h>
extern uintptr_t R_CStackStart;
extern int R_PPStackTop;


class RInterpreterHandle {
public:
  
  RInterpreterHandle ()
    : my_stack_top(R_PPStackTop)
  {}
  
  // Assumption: mutex is unlocked
  void claim (uintptr_t _stack = (uintptr_t)-1)
  {
    R_CStackStart = (uintptr_t)_stack;
    interpreter_mutex().lock();
    
    if (my_stack_top) {
      R_PPStackTop = my_stack_top;
    }
  }
  
  
  // Assumption: mutex is locked
  void release (bool _notify = true)
  {
    my_stack_top = R_PPStackTop;
    interpreter_mutex().unlock();
    
    // won't notify when called from library destructor
    // TODO what if there are threads that are still running?
    if (_notify) {
      interpreter_condition().notify_all();
    }
  }
  
  
  // Assumption: mutex is locked
  void yield ()
  {
    release();
    std::this_thread::yield();
    claim();
  }
  
  
private:
  
  int my_stack_top;
  
  std::recursive_mutex & interpreter_mutex ();

  std::condition_variable & interpreter_condition ();
  
};


#endif /* RINTERPRETER_H_GUARD */
