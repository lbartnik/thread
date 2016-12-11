#ifndef IS_SOMETHING_H_GUARD
#define IS_SOMETHING_H_GUARD

#include <R.h>
#include <Rdefines.h>


#ifdef __cplusplus
extern "C" {
#endif


int is_single_string (SEXP _obj);
int is_nonempty_string (SEXP _obj);
int is_single_string_or_NULL (SEXP _obj);
int is_single_integer (SEXP _obj);
  

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* IS_SOMETHING_H_GUARD */
