#include "builtin/exception.hpp"

#include "vm.hpp"
#include "prelude.hpp"
#include "exception.hpp"
#include "detection.hpp"

#include <cctype>
#include <vector>
#include <iostream>
#include <cxxabi.h>
#include <cassert>

#ifdef USE_EXECINFO
#include <execinfo.h>
#endif

namespace rubinius {

  void TypeError::raise(object_type type, Object* obj, const char* reason) {
    throw TypeError(type, obj, reason);
    // Not reached.
  }

  void Assertion::raise(const char* reason) {
    VM* state = VM::current_state();
    abort();
    if(!state || !state->use_safe_position) {
      throw Assertion(reason);
    }
    state->raise_assertion_safely(new Assertion(reason));
    // Not reached.
  }

  RubyException::RubyException(Exception* exception, bool make_backtrace)
      : VMException(make_backtrace), exception(exception) {
  }

  void RubyException::raise(Exception* exception, bool make_backtrace) {
    throw RubyException(exception, make_backtrace);
    // Not reached.
  }

  void RubyException::show(STATE) {
    std::cout << exception->message();
    print_backtrace();
    state->print_backtrace();
  }

  static VMException::Backtrace get_trace(size_t skip) {
#ifdef USE_EXECINFO
    const size_t max_depth = 100;
    size_t stack_depth;
    void *stack_addrs[max_depth];
    char **stack_strings;

    stack_depth = backtrace(stack_addrs, max_depth);
    stack_strings = backtrace_symbols(stack_addrs, stack_depth);

    VMException::Backtrace s;

    for (size_t i = skip; i < stack_depth; i++) {
      s.push_back(std::string(stack_strings[i]));
    }
    free(stack_strings); // malloc()ed by backtrace_symbols
#else
    VMException::Backtrace s;
    s.push_back(std::string("C++ backtrace not available"));
#endif

    return s;
  }

  static void squeeze_space(std::string& str) {
    std::string ws = "    ";

    std::string::size_type pos = str.find(ws);
    if(pos == std::string::npos) return;

    std::string::size_type i = pos + 4;
    while(std::isspace(str[i])) i++;
    str.erase(pos + 2, i - pos - 2);
  }

  static void demangle(VMException::Backtrace& s) {
    for (size_t i = 0; i < s.size(); i++) {
      squeeze_space(s[i]);
      const char* str = s[i].c_str();
      const char* pos = strstr(str, " _Z");
      /* Found a mangle. */
      if(pos) {
        size_t sz = 1024;
        char *cpp_name = 0;
        char* name = strdup(pos + 1);
        char* end = strstr(name, " + ");
        *end = 0;

        int status;
        cpp_name = abi::__cxa_demangle(name, cpp_name, &sz, &status);

        if(!status) {
          std::string full_cpp = std::string(str, pos - str) + " " + cpp_name +
            " " + (++end);
          s[i] = full_cpp;
        }
        if(cpp_name) free(cpp_name);
        free(name);
      }
    }
  }

  void abort() {
    std::cout << "Abort!" << std::endl;
    print_backtrace();
    assert(0);
  }

  void print_backtrace() {
    VMException::Backtrace s = get_trace(2);
    demangle(s);

    for(size_t i = 0; i < s.size(); i++) {
      std::cout << s[i] << std::endl;
    }
  }

  static VMException::Backtrace get_cpp_backtrace() {
    VMException::Backtrace s = get_trace(2);
    demangle(s);
    return s;
  }

  VMException::VMException(bool make_backtrace)
      : backtrace(NULL), reason(NULL) {
    if(make_backtrace) {
      backtrace = new VMException::Backtrace(get_cpp_backtrace());
    }
  }

  VMException::VMException(const char* reason, bool make_backtrace)
      : backtrace(NULL), reason(NULL) {
    if(make_backtrace) {
      backtrace = new VMException::Backtrace(get_cpp_backtrace());
    }
    if(reason) this->reason = strdup(reason);
  }

  void VMException::print_backtrace() {
    if(!backtrace) return;

    for(size_t i = 0; i < backtrace->size(); i++) {
      std::cout << backtrace->at(i) << std::endl;
    }
  }
}
