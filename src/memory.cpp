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

#include <elf.h>

Elf64_Sym  gbl_sym_table[1] __attribute__((weak));


// --- using a patched version of R ------------------------------------

#include <thread>
#include <iostream>

#include "rinterpreter.h"
#include "is_something.h"

#include <Rdefines.h>



#ifdef ALLOC_VECTOR_3_IS_A_CALLBACK

extern "C" {

  typedef SEXP (allocVector3_t)(SEXPTYPE type, R_xlen_t length, R_allocator_t *allocator);
  
  allocVector3_t Rf_allocVector3_impl;
  allocVector3_t allocVector3_synchronized;
  
} // extern "C"



SEXP allocVector3_synchronized (SEXPTYPE _type, R_xlen_t _len, R_allocator_t* _alloc)
{
  RInterpreterHandle rInterpreter;
  rInterpreter.claim();
  
  SEXP ret = Rf_allocVector3_impl(_type, _len, _alloc);
  
  rInterpreter.release();
  
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


extern "C" SEXP C_memory_allocation_test (SEXP _n, SEXP _size, SEXP _timeout)
{
  if (!is_single_integer(_n)) {
    Rf_error("`n` must be a single integer value");
  }
  if (!is_single_integer(_size)) {
    Rf_error("`size` must be a single integer value");
  }
  if (!is_single_integer(_timeout)) {
    Rf_error("`timeout` must be a single integer value");
  }
  
  int n       = INTEGER_DATA(_n)[0];
  int size    = INTEGER_DATA(_size)[0];
  int timeout = INTEGER_DATA(_timeout)[0];
  
  
  SEXP obj;
  for (int i=0; i<n; ++i) {
    obj = allocVector(INTSXP, size);
    
    std::cout << "thread " << std::this_thread::get_id()
              << " allocated " << size << " integers, sleeping"
              << std::endl;

    std::chrono::milliseconds ms{timeout};
    std::this_thread::sleep_for(ms);
  }

  return R_NilValue;
}



#else /* ALLOC_VECTOR_3_IS_A_CALLBACK */

// noop
void set_alloc_callback ()
{
  std::cout << "memory allocation via allocVector3 is not synchronized\n";
}

// noop
void reset_alloc_callback ()
{
}


extern "C" SEXP C_is_memory_synchronized ()
{
  SEXP ans;
  PROTECT(ans = allocVector(LGLSXP, 1));
  LOGICAL_DATA(ans)[0] = 0;
  UNPROTECT(1);
  return ans;
}


#endif /* ALLOC_VECTOR_3_IS_A_CALLBACK */


