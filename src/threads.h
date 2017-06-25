#ifndef THREADS_H_GUARD
#define THREADS_H_GUARD

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "Rinternals.h"
#include "R/Defn.h"
#undef length




class interpreter_context
{
public:

  typedef std::shared_ptr<interpreter_context> ptr;


private:

  typedef std::map<std::thread::id, interpreter_context::ptr> contexts_type;


  interpreter_context(const interpreter_context &) {}

  interpreter_context (uintptr_t _stack_start,
                       RCNTXT * _global_context = nullptr);


  static contexts_type & contexts ();

  static std::recursive_mutex & container_mutex ();

  std::vector<SEXP> stack_data;

public:

  uintptr_t stack_start;
  int count;
  RCNTXT * global_context;
  chained_stack link;

  
  static void create(uintptr_t _stack_base);

  static interpreter_context & get_this_context ();

  static void destroy ();  
};


#endif /* THREADS_H_GUARD */
