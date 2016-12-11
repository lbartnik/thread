#include <thread>

#include "cppthreads.h"
#include "rinterpreter.h"



/*
 * http://pabercrombie.com/wordpress/2014/05/how-to-call-an-r-function-from-c/
 *
 * R_tryEval evaluates the call via R_ToplevelContext; this in turn
 * (according to main/context.c) will not be left by a long jump.
 * 
 * R_ToplevelExec - call fun(data) within a top level context to
 * insure that this functin cannot be left by a LONGJMP.  R errors in
 * the call to fun will result in a jump to top level. The return
 * value is TRUE if fun returns normally, FALSE if it results in a
 * jump to top level.
 */
void thread_runner (SEXP _fun, SEXP _data, SEXP _env)
{
  // pass the address of the top of this thread's stack
  RInterpreterHandle rInterpreter;
  rInterpreter.claim();
  
  SEXP val, call;
  int errorOccurred;
  
  
  PROTECT(call = lang2(_fun, _data));
  PROTECT(val = R_tryEval(call, _env, &errorOccurred));
  
  if (errorOccurred) {
    // TODO store error info in thread's _env
    fprintf(stderr, "An error occurred when calling `fun`\n");
    fflush(stderr);
  } else {
    defineVar(install("result"), val, _env);
  }
  
  UNPROTECT(2);
  
  rInterpreter.release();
}
