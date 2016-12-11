#ifndef THREADING_H_GUARD
#define THREADING_H_GUARD


#include <R.h>
#include <Rinternals.h>

#ifdef __cplusplus
extern "C" {
#endif

  
SEXP C_create_new_thread (SEXP _fun, SEXP _data, SEXP _env);
SEXP C_thread_yield ();
SEXP C_thread_benchmark (SEXP _n, SEXP _timeout);
SEXP C_thread_join (SEXP _handle);


#ifdef __cplusplus
} // extern "C"
#endif


#endif /* THREADING_H_GUARD */
