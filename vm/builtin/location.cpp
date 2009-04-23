#include "builtin/location.hpp"
#include "builtin/class.hpp"
#include "builtin/compiledmethod.hpp"
#include "vm.hpp"

#include "call_frame.hpp"

namespace rubinius {
  void Location::init(STATE) {
    GO(location).set(state->new_class("Location", G(object)));
    G(location)->set_object_type(state, LocationType);
  }

  Location* Location::create(STATE, CallFrame* call_frame) {
    Location* loc = state->new_object<Location>(G(location));
    loc->method_module(state, call_frame->module());
    loc->receiver(state, call_frame->self());
    loc->method(state, call_frame->cm);
    loc->ip(state, Fixnum::from(call_frame->ip - 1));

    if(call_frame->is_block) {
      loc->name(state, call_frame->top_scope->method()->name());
      loc->is_block(state, Qtrue);
    } else {
      loc->name(state, call_frame->name);
      loc->is_block(state, Qfalse);
    }

    return loc;
  }
}
