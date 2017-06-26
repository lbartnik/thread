#include "debug.h"
#include <thread>

__thread int indent = 0;

trace_guard::trace_guard (const char * _name) : name(_name) {
  std::cerr << std::this_thread::get_id() << " ";
  for (int i=0; i<indent; ++i) std::cerr << "  ";
  std::cerr << "-> " << name << "()" << std::endl;
  ++indent;
}

trace_guard::~trace_guard() {
  std::cerr << std::this_thread::get_id() << " ";
  --indent;
  for (int i=0; i<indent; ++i) std::cerr << "  ";
  std::cerr << "<- " << name << "()" << std::endl;
}
