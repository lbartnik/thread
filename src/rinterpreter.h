#ifndef RINTERPRETER_H_GUARD
#define RINTERPRETER_H_GUARD


#include <condition_variable>
#include <mutex>
#include <thread>
#include <map>
#include <vector>

#include "Rinternals.h"
#include "R/Defn.h"
#undef length

#include "threads.h"


class RInterpreterHandle {
  
  
public:
  
  RInterpreterHandle () {}
  
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
};


#endif /* RINTERPRETER_H_GUARD */
