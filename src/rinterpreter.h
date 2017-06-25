#ifndef RINTERPRETER_H_GUARD
#define RINTERPRETER_H_GUARD


#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "Rinternals.h"
#include "R/Defn.h"
#undef length

#include "threading.h"



class interpreter_context
{
public:

  typedef std::shared_ptr<interpreter_context> ptr;


private:

  typedef std::map<std::thread::id, interpreter_context::ptr> contexts_type;


  interpreter_context(const interpreter_context &) {}

  interpreter_context (uintptr_t _stack_start,
                       SEXP * _pp_stack_start,
                       int _stack_top,
                       int _stack_size,
                       RCNTXT * _global_context);

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

  // main thread, called when DLL is loaded
  static void create();
  
  // threads created within the package
  static void create(uintptr_t _stack_base);

  static interpreter_context & get_this_context ();

  static void destroy ();  
};





class RInterpreterLock {
  
  
public:
  
  RInterpreterLock () {}
  
  // Assumption: mutex is unlocked
  void gil_enter ();
  
  // Assumption: mutex is locked
  void gil_leave ();
  
  // Assumption: mutex is locked
  void __yield ()
  {
    gil_leave();
    std::this_thread::yield();
    gil_enter();
  }
  
  
private:

  std::recursive_mutex & interpreter_mutex ();

  std::condition_variable & interpreter_condition ();
};


#endif /* RINTERPRETER_H_GUARD */
