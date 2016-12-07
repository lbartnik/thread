
#include <stdint.h>

#include <R.h>
#include <Rdefines.h>
#include <Rinternals.h>

//#include <Defn.h>
extern uintptr_t R_CStackStart;



static SEXP static_result_sexp;


SEXP get_static_result ()
{
  return static_result_sexp;
}


SEXP callFoo()
{
  
  R_CStackStart = (uintptr_t)-1;

  SEXP e, val;
  int errorOccurred;
  //int result = -1;
  
  // http://pabercrombie.com/wordpress/2014/05/how-to-call-an-r-function-from-c/
  //PROTECT(e = allocVector(LANGSXP, 1));
  //SETCAR(e, Rf_install("foo"));
  
  PROTECT(e = lang2(install("summary"), install("iris")));
  R_tryEval(e, R_GlobalEnv, NULL);
  
  val = R_tryEval(e, R_GlobalEnv, &errorOccurred);
  
  if(!errorOccurred) {
    static_result_sexp = val;
    UNPROTECT(1);
    return val;
    //PROTECT(val);
    //result = INTEGER(val)[0];
    //UNPROTECT(1);
  } else {
    fprintf(stderr, "An error occurred when calling foo\n");
    fflush(stderr);
  }
  
  /* Assume we have an INTSXP here. */
  
  UNPROTECT(1); /* e */
  
  return R_NilValue;
}
