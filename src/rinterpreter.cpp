#include "rinterpreter.h"


std::recursive_mutex & RInterpreterHandle::interpreter_mutex () {
  static std::recursive_mutex mutex;
  return mutex;
}

std::condition_variable & RInterpreterHandle::interpreter_condition () {
  static std::condition_variable condition;
  return condition;
}
