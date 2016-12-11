#ifndef CPPTHREAD_H_GUARD
#define CPPTHREAD_H_GUARD

#include <cstdint>
#include <Rinternals.h>

void thread_runner (SEXP _fun, SEXP _data, SEXP _env);

void claim_R_interpreter (uintptr_t _stack = (uintptr_t)-1);
void release_R_interpreter ();
void yield_R_interpreter ();


#endif /* CPPTHREAD_H_GUARD */
