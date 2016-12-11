#include <thread>
#include <mutex>
#include <condition_variable>

#include "cppthreads.h"


//#include <Defn.h>
extern uintptr_t R_CStackStart;



static std::mutex              interpreter_mutex;
static std::condition_variable interpreter_condition;


// Assumption: mutex is unlocked
void claim_R_interpreter (uintptr_t _stack)
{
  R_CStackStart = (uintptr_t)_stack;
  interpreter_mutex.lock();
}



// Assumption: mutex is locked
void release_R_interpreter ()
{
  interpreter_condition.notify_all();
  interpreter_mutex.unlock();
}


// Assumption: mutex is locked
void yield_R_interpreter ()
{
  std::unique_lock<std::mutex> lock(interpreter_mutex, std::adopt_lock);
  interpreter_condition.notify_all();
  interpreter_condition.wait(lock);
}


// http://pabercrombie.com/wordpress/2014/05/how-to-call-an-r-function-from-c/
void thread_runner (SEXP _fun, SEXP _data, SEXP _env)
{
  // pass the address of the top of this thread's stack 
  claim_R_interpreter();
  
  SEXP val;
  int errorOccurred;
  
  PROTECT(val = R_tryEval(_fun, _env, &errorOccurred));
  
  if (!errorOccurred) {
    // TODO store error info in thread's _env
    fprintf(stderr, "An error occurred when calling foo\n");
    fflush(stderr);
  } else {
    // TODO store val in thread's _env
  }
  
  UNPROTECT(1);
  
  release_R_interpreter();
}
