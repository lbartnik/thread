#ifndef THREADING_H_GUARD
#define THREADING_H_GUARD


#include <R.h>
#include <Rinternals.h>


extern "C" SEXP C_create_new_thread (SEXP _fun, SEXP _data, SEXP _env);
extern "C" SEXP C_thread_yield ();
extern "C" SEXP C_thread_benchmark (SEXP _n, SEXP _timeout);

#endif /* THREADING_H_GUARD */
