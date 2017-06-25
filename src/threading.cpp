/*
 * === Starting a new thread ===
 *
 * main thread
 *    |
 *    |
 *  create new env
 *    |
 *  assign the new env into package's private namespace
 *  to make sure it won't be garbage-collected
 *    |
 *    +--- spawn new thread with the new env as param ------+
 *    |                                                     .
 *    |                                                     .
 *  hand control over to the                                .
 *  new thread (how to make sure                            .
 *  it's only this new thread that                          .
 *  can take over?)                                         .
 *    |                                                     .
 *    .                        --->                   take control
 *    .                                                     |
 *    .                                         store own id in the new env
 *    .                                         and do other book-keeping
 *    .                                                     |
 *    .                                           give the control back
 *    |                        <---                         .
 *  the new thread is ready; do                             .
 *  other stuff or simply yield                             .
 *  and let the other thread take                           .
 *  over again                                              .
 *    .                        --->               run the main function
 *    .                                           with R_tryEval in own
 *    .                                           (the "new") environment
 */

#include <thread>
#include <sstream>
#include <iostream>


#include "cppthreads.h"
#include "threading.h"
#include "is_something.h"
#include "rinterpreter.h"
#include "memory.h"



__attribute__((constructor))
static void initialize_threading ()
{
  RInterpreterHandle rInterpreter;
  rInterpreter.claim();

  set_alloc_callback();
}

__attribute__((destructor))
static void teardown_threading ()
{
  reset_alloc_callback();
  
  // TODO make sure we never exit before all threads are joined()

  // TODO is it even needed to release synchronization variables?
  //      maybe their destructors take care of everything?
  RInterpreterHandle rInterpreter;
  rInterpreter.release(false);
}




static void C_thread_finalizer(SEXP ptr);


/*
 * Creates a new thread and obtains its id.
 * 
 * Parameters passed to the new thread must be anchored in the global
 * environment outside of this function to prevent the garbage collector
 * from claiming their memory.
 */
SEXP C_create_new_thread (SEXP _fun, SEXP _data, SEXP _env)
{
  SEXP ans, ptr;
  
  void * handle_ptr = Calloc(1, std::thread);
  
  std::thread * handle = new (handle_ptr) std::thread(thread_runner, _fun, _data, _env);
  
  PROTECT(ptr = R_MakeExternalPtr(handle, install("thread_handle"), R_NilValue));
  R_RegisterCFinalizerEx(ptr, C_thread_finalizer, TRUE);
  
  /* return the thread id */
  std::stringstream ss;
  ss << handle->get_id();

  ans = PROTECT(allocVector(STRSXP, 1));
  SET_STRING_ELT(ans, 0, mkChar(ss.str().c_str()));
  setAttrib(ans, install("handle_ptr"), ptr);

  /* ptr, ans */
  UNPROTECT(2);
  
  return ans;
}



static void C_thread_finalizer (SEXP ptr)
{
  if (!R_ExternalPtrAddr(ptr)) return;
  
  RInterpreterHandle rInterpreter;
  rInterpreter.claim();
  
  std::thread * handle = (std::thread*)R_ExternalPtrAddr(ptr);

  if (handle->joinable()) {
    rInterpreter.release();
    handle->join();
    rInterpreter.claim();
  }
  handle->~thread();

  //Rf_perror("error while finalizing child process");

  R_ClearExternalPtr(ptr); /* not really needed */
  
  rInterpreter.release();
}


std::thread * extract_thread_handle (SEXP _handle)
{
  SEXP ptr = getAttrib(_handle, install("handle_ptr"));
  if (ptr == R_NilValue) {
    Rf_error("`handle_ptr` attribute not found");
  }
  
  void * c_ptr = R_ExternalPtrAddr(ptr);
  if (!c_ptr) {
    Rf_error("external C pointer is NULL");
  }
  
  return (std::thread*)c_ptr;
}





SEXP C_thread_yield ()
{
  RInterpreterHandle rInterpreter;
  rInterpreter.yield();
  return R_NilValue;
}


SEXP C_thread_join (SEXP _handle)
{
  RInterpreterHandle rInterpreter;
  rInterpreter.release();
  extract_thread_handle(_handle)->join();
  rInterpreter.claim();
  return R_NilValue;
}


// --- testing utilities -----------------------------------------------

SEXP C_thread_print (SEXP _message)
{
  if (!is_single_string(_message)) {
    Rf_error("`message` must be a single character value");
  }
  
  const char * message = translateChar(STRING_ELT(_message, 0));

  RInterpreterHandle rInterpreter;
  rInterpreter.release();
  
  std::cout << message;
  std::cout.flush();
  
  rInterpreter.claim();
  
  return R_NilValue;
}


SEXP C_thread_sleep (SEXP _timeout)
{
  if (!is_single_integer(_timeout)) {
    Rf_error("`timeout` must be a single integer value");
  }
  
  int timeout = INTEGER_DATA(_timeout)[0];

  RInterpreterHandle rInterpreter;
  rInterpreter.release();
  
  std::chrono::milliseconds ms{timeout};
  std::this_thread::sleep_for(ms);
  
  rInterpreter.claim();
  
  return R_NilValue;
}


SEXP C_thread_sum (SEXP _array, SEXP _from, SEXP _to)
{
  if (!isReal(_array)) {
    Rf_error("`array` must be numeric");
  }
  if (!is_single_integer(_from)) {
    Rf_error("`from` must be a single integer");
  }
  if (!is_single_integer(_to)) {
    Rf_error("`to` must be a single integer");
  }
  
  int from = INTEGER_DATA(_from)[0];
  int to = INTEGER_DATA(_to)[0];
  
  if (from < 0 || from >= LENGTH(_array) || to < 0 || to >= LENGTH(_array) || to < from) {
    Rf_error("`from` and `to` must be within range of `array`");
  }
  
  SEXP ans;
  PROTECT(ans = allocVector(REALSXP, 1));
  PROTECT(_array);
  
  double sum=0;
  double * array = DOUBLE_DATA(_array);
  double * ptr = array + from;
  
  RInterpreterHandle rInterpreter;
  rInterpreter.release();
  
  for (; from<=to; ++from, ++ptr) {
    sum += *ptr;
  }

  rInterpreter.claim();

  DOUBLE_DATA(ans)[0] = sum;
  UNPROTECT(2);

  return ans;
}



SEXP C_thread_benchmark (SEXP _n, SEXP _timeout)
{
  if (!is_single_integer(_n)) {
    Rf_error("`n` must be a single integer value");
  }
  if (!is_single_integer(_timeout)) {
    Rf_error("`timeout` must be a single integer value");
  }

  int n = INTEGER_DATA(_n)[0];
  int timeout = INTEGER_DATA(_timeout)[0];

  RInterpreterHandle rInterpreter;
  rInterpreter.release();

  for (int i=0; i<n; ++i)
  {
    std::cout << "thread " << std::this_thread::get_id()
              << " iteration " << i << " of " << n
              << " sleeping for " << timeout << " milliseconds"
              << '\n';
    
    std::chrono::milliseconds ms{timeout};
    std::this_thread::sleep_for(ms);
  }

  rInterpreter.claim();
  
  return R_NilValue;
}


