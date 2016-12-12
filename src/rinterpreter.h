#ifndef RINTERPRETER_H_GUARD
#define RINTERPRETER_H_GUARD


#include <condition_variable>
#include <mutex>
#include <thread>
#include <map>



//#include <Defn.h>
extern uintptr_t R_CStackStart;
extern int R_PPStackTop;


struct interpreter_context {
  
  interpreter_context (uintptr_t _stack_start, int _pp_stack_top)
    : stack_start(_stack_start), pp_stack_top(_pp_stack_top)
  {}
  
  uintptr_t stack_start;
  int pp_stack_top;
};


class RInterpreterHandle {
  
  typedef std::map<std::thread::id, interpreter_context> contexts_type;
  
public:
  
  RInterpreterHandle () {}
  
  void init (uintptr_t _stack_start);

  interpreter_context & get_this_context ();

  // Assumption: mutex is unlocked
  void claim ();
  
  // Assumption: mutex is locked
  void release (bool _notify = true)
  {
    get_this_context().pp_stack_top = R_PPStackTop;
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

  std::recursive_mutex & interpreter_mutex ();

  std::condition_variable & interpreter_condition ();
  
  contexts_type & interpreter_contexts ();

};


#endif /* RINTERPRETER_H_GUARD */
