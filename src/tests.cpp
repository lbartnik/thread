#include "Rinternals.h"
#undef length
//#include "R/Defn.h"

#include "rinterpreter.h"
#include "is_something.h"
#include "debug.h"
#include <iostream>
#include <thread>


extern "C" SEXP C_memory_allocation_test (SEXP _n, SEXP _size, SEXP _timeout)
{
  TRACE_INOUT;

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


SEXP C_single_thread_print_test (SEXP _n, SEXP _timeout)
{
  TRACE_INOUT;

  if (!is_single_integer(_n)) {
    Rf_error("`n` must be a single integer value");
  }
  if (!is_single_integer(_timeout)) {
    Rf_error("`timeout` must be a single integer value");
  }

  int n = INTEGER_DATA(_n)[0];
  int timeout = INTEGER_DATA(_timeout)[0];

  RInterpreterLock rInterpreter;
  rInterpreter.gil_leave();

  for (int i=0; i<n; ++i)
  {
    std::cout << "thread " << std::this_thread::get_id()
              << " iteration " << i << " of " << n
              << " sleeping for " << timeout << " milliseconds"
              << '\n';
    
    std::chrono::milliseconds ms{timeout};
    std::this_thread::sleep_for(ms);
  }

  rInterpreter.gil_enter();
  
  return R_NilValue;
}

