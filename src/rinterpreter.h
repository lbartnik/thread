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


  interpreter_context(const interpreter_context &);

  // master thread
  interpreter_context (uintptr_t _stack_start,
                       chained_stack & _global_link,
                       RCNTXT * _global_context,
                       int _context_no);

  // thread pool
  interpreter_context (uintptr_t _stack_start,
                       RCNTXT * _global_context,
                       int _context_no);

  void dump_change();

  static contexts_type & contexts ();

  static std::recursive_mutex & container_mutex ();

  std::vector<SEXP> stack_data;

public:

  uintptr_t stack_start;
  int count;
  RCNTXT * global_context;
  chained_stack & link;
  chained_stack local_link;
  std::thread::id thread_id;
  int context_number;

  void enter ();

  void leave ();


  // main thread, called when DLL is loaded
  static void create();
  
  // threads created within the package
  static void create(uintptr_t _stack_base);

  static interpreter_context & get_this_context ();

  static void destroy ();

  static void dump_all ();

  static void assert_single_thread_in_gil ();
};





class RInterpreterLock {
public:
  
  struct context_guard {
    context_guard(RCNTXT * _global_context, int _pp_stack_top)
      : global_context(_global_context), pp_stack_top(_pp_stack_top)
    {}

    bool operator == (const context_guard & _other)
    {
      return global_context == _other.global_context &&
             pp_stack_top == _other.pp_stack_top;
    }

    bool operator != (const context_guard & _other) {
      return !(*this == _other);
    }

    RCNTXT * global_context;
    int pp_stack_top;
  };

  RInterpreterLock () {}
  
  // Assumption: mutex is unlocked
  context_guard gil_enter ();
  
  // Assumption: mutex is locked
  void gil_leave ();
  void gil_leave (const context_guard &);
  
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
