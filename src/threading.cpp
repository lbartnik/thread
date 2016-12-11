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

#include "cppthreads.h"
#include "threading.h"
#include "is_something.h"

#include <thread>
#include <sstream>
#include <iostream>


__attribute__((constructor))
static void initialize_threading ()
{
  claim_R_interpreter();
}

__attribute__((destructor))
static void teardown_threading ()
{
  release_R_interpreter();
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
  
  std::thread * handle = (std::thread*)R_ExternalPtrAddr(ptr);

  handle->join();
  handle->~thread();

  //Rf_perror("error while finalizing child process");

  R_ClearExternalPtr(ptr); /* not really needed */
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
  yield_R_interpreter();
}


SEXP C_thread_join (SEXP _handle)
{
  extract_thread_handle(_handle)->join();
  return R_NilValue;
}



// --- testing utilities -----------------------------------------------

SEXP C_thread_printf (SEXP _message)
{
  if (!is_single_string(_message)) {
    Rf_error("`message` must be a single character value");
  }
  
  const char * message = translateChar(STRING_ELT(_message, 0));

  release_R_interpreter();
  
  std::cout << message;
  
  claim_R_interpreter();
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

  release_R_interpreter();

  for (int i=0; i<n; ++i)
  {
    std::cout << "thread " << std::this_thread::get_id()
              << " iteration " << i << " of " << n
              << " sleeping for " << timeout << " milliseconds"
              << '\n';
    
    std::chrono::milliseconds ms{timeout};
    std::this_thread::sleep_for(ms);
  }

  claim_R_interpreter();
  
  return R_NilValue;
}


