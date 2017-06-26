#ifndef DEBUG_H_GUARD
#define DEBUG_H_GUARD

#include <iostream>

//#define DEBUG_THREADS 1
#define DEBUG_THREADS 0

struct trace_guard {
  const char * name;
  trace_guard (const char * _name);
  ~trace_guard();
};

#define TRACE_INOUT trace_guard guard(__FUNCTION__);

#endif /* DEBUG_H_GUARD */
