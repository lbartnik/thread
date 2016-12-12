#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include "rinterpreter.h"



// from #include <Rdynpriv.h>
// implemented after memory.c
DL_FUNC R_ExternalPtrAddrFn(SEXP s)
{
  return (DL_FUNC)EXTPTR_PTR(s);
}


#define MAX_NATIVE_ARGS 4

static void execute_native (void * _data);

struct native_call {
  typedef SEXP (*VarFun)(...);
  
  native_call() {
    memset(this, 0, sizeof(native_call));
  }
  
  VarFun fun;
  SEXP args[MAX_NATIVE_ARGS];
  int no_args;
  SEXP ret;
};


/*
* Look at checkValidSymbolId() in dotcode.c
*/
extern "C" SEXP C_thread_run_native (SEXP _address, SEXP _args)
{
  static SEXP native_symbol = install("native symbol");
  native_call call;
  
  // extract native function address
  if (TYPEOF(_address) != EXTPTRSXP) {
    Rf_error("`address` must be an external pointer");
  }
  if(R_ExternalPtrTag(_address) != native_symbol) {
    Rf_error("can call only native symbols");
  }
  call.fun = (native_call::VarFun)R_ExternalPtrAddrFn(_address);
  
  // extract arguments
  if (!isVector(_args)) {
    Rf_error("`args` must be a list");
  }
  int len = LENGTH(_args);
  if (len > MAX_NATIVE_ARGS) {
    Rf_error("the number of arguments for the native function is not supported in thread execution");
  }
  
  SEXP pargs;
  for(call.no_args = 0, pargs = _args ; call.no_args < len; call.no_args++) {
    call.args[call.no_args] = VECTOR_ELT(pargs, call.no_args);
    call.no_args++;
  }
  
  // this is run outside of R interpreter; drop Global Interpreter Lock
  RInterpreterHandle rInterpreter;
  rInterpreter.release();
  
  // call the function
  Rboolean rc = R_ToplevelExec(execute_native, (void*)&call);
  if (rc == FALSE) {
    Rf_error("error when calling native function");
  }

  UNPROTECT(1);
  
  rInterpreter.claim();
  
  return call.ret;
}


void execute_native (void * _data)
{
  native_call * call = (native_call*)_data;
  SEXP * cargs = call->args;
  
  switch (call->no_args) {
  case 0:
    call->ret = (SEXP)call->fun();
    break;
  case 1:
    call->ret = (SEXP)call->fun(cargs[0]);
    break;
  case 2:
    call->ret = (SEXP)call->fun(cargs[0], cargs[1]);
    break;
  case 3:
    call->ret = (SEXP)call->fun(cargs[0], cargs[1], cargs[2]);
    break;
  case 4:
    call->ret = (SEXP)call->fun(cargs[0], cargs[1], cargs[2], cargs[3]);
    break;
  default:
    Rf_error("too many arguments");
  }
  
  PROTECT(call->ret);
}


/*
 * Used in tests to verify whether C_thread_native_call() works
 * properly.
 */
extern "C" SEXP C_sample_call (SEXP arg)
{
  return arg;
}

