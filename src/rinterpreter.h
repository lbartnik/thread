#ifndef RINTERPRETER_H_GUARD
#define RINTERPRETER_H_GUARD


#include <condition_variable>
#include <mutex>
#include <thread>
#include <map>

#include "Rinternals.h"
#include "R/Defn.h"
#undef length


struct interpreter_context {
  
  interpreter_context (uintptr_t _stack_start,
                       RCNTXT * _global_context = nullptr)
    : stack_start(_stack_start), pp_stack_top(R_PPStackTop),
      count(0), global_context(_global_context)
  {}
  
  uintptr_t stack_start;
  int pp_stack_top, count;
  RCNTXT * global_context;
};


class RInterpreterHandle {
  
  typedef std::map<std::thread::id, interpreter_context> contexts_type;
  
public:
  
  RInterpreterHandle () {}
  
  void init (uintptr_t _stack_start, RCNTXT * _global_context);

  void destroy ();
  
  interpreter_context & get_this_context ();

  // Assumption: mutex is unlocked
  void claim ();
  
  // Assumption: mutex is locked
  void release (bool _notify = true);
  
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
