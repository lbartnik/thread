/*
 * How to intercept calls to allocVector?
 * 
 * http://stackoverflow.com/questions/12947388/is-there-a-way-to-modify-the-code-of-a-function-in-a-linux-c-program-at-runtime
 * 
 * Don't overwrite the procedure, overwrite the symbol reference in
 * the symbol table instead. That does require dynamic linkage.
 * Alternatively you can overwrite the call(s) to the function with
 * a call to the other function, but things like NX bits may come to
 * stand in your way. Self-modifying code is generally frowned upon.
 * 
 * http://bottomupcs.sourceforge.net/csbu/x4012.htm
 * https://refspecs.linuxbase.org/elf/gabi4+/ch4.symtab.html
 * 
 * http://stackoverflow.com/questions/11254891/can-a-running-c-program-access-its-own-symbol-table
 * 
 * http://stackoverflow.com/questions/12255555/accessing-elf-symbol-table-in-c
 */


#include <cstdio>
#include <cstring>
#include <iostream>
#include <Rinternals.h>



// --- using a patched version of R ------------------------------------

#include <thread>
#include <iostream>

#include "rinterpreter.h"
#include "is_something.h"

#include <Rdefines.h>


extern "C" {

  typedef SEXP (allocVector3_t)(SEXPTYPE type, R_xlen_t length, R_allocator_t *allocator);
  
  extern allocVector3_t Rf_allocVector3_impl; // comes from the patched Rlib
  allocVector3_t allocVector3_synchronized;
  
} // extern "C"



SEXP allocVector3_synchronized (SEXPTYPE _type, R_xlen_t _len, R_allocator_t* _alloc)
{
  RInterpreterLock rInterpreter;
  rInterpreter.gil_enter();
  
  SEXP ret = Rf_allocVector3_impl(_type, _len, _alloc);
  
  rInterpreter.gil_leave();
  
  return ret;
}


static allocVector3_t * allocVector3_orig;


void set_alloc_callback ()
{
  std::cout << "setting a callback for allocVector3\n";

  allocVector3_orig = Rf_allocVector3;
  Rf_allocVector3 = allocVector3_synchronized;
}

void reset_alloc_callback ()
{
  Rf_allocVector3 = allocVector3_orig;
}


extern "C" SEXP C_is_memory_synchronized ()
{
  SEXP ans;
  PROTECT(ans = allocVector(LGLSXP, 1));
  LOGICAL_DATA(ans)[0] = 1;
  UNPROTECT(1);
  return ans;
}


