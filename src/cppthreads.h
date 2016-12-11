#ifndef CPPTHREAD_H_GUARD
#define CPPTHREAD_H_GUARD

#include <cstdint>
#include <Rinternals.h>

void thread_runner (SEXP _fun, SEXP _data, SEXP _env);

#endif /* CPPTHREAD_H_GUARD */
