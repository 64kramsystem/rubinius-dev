#ifndef RBX_THREAD_STATE_HPP
#define RBX_THREAD_STATE_HPP

#include "raise_reason.hpp"
#include "gc/root.hpp"

namespace rubinius {
  class Object;
  class Exception;
  class VariableScope;
  class VM;

  class ThreadState {
    TypedRoot<Object*> raise_value_;
    RaiseReason raise_reason_;
    TypedRoot<VariableScope*> destination_scope_;

  public:
    ThreadState(VM* vm)
      : raise_value_(vm, Qnil)
      , raise_reason_(cNone)
      , destination_scope_(vm, (VariableScope*)Qnil)
    {}

    Object* raise_value() {
      return raise_value_.get();
    }

    RaiseReason raise_reason() {
      return raise_reason_;
    }

    VariableScope* destination_scope() {
      return destination_scope_.get();
    }

    void clear_exception() {
      raise_value_.set(Qnil);
      raise_reason_ = cNone;
      destination_scope_.set(Qnil);
    }

    void set_exception(Object* obj) {
      raise_value_.set(obj);
      raise_reason_ = cException;
      destination_scope_.set(Qnil);
    }

    void raise_exception(Exception* exc);
    void raise_return(Object* value, VariableScope* dest);
    void raise_break(Object* value, VariableScope* dest);
    void raise_exit(Object* code);
  };
};

#endif
