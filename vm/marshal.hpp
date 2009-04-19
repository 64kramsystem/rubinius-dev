#ifndef RBX_MARSHAL_HPP
#define RBX_MARSHAL_HPP

#include <iostream>
#include <sstream>

#include "prelude.hpp"

namespace rubinius {

  class Object;

  class SendSite;
  class InstructionSequence;
  class CompiledMethod;
  class String;
  class Array;
  class Bignum;
  class Float;
  class Symbol;
  class Tuple;

  class UnMarshaller {
  public:
    STATE;
    std::istream& stream;

    UnMarshaller(STATE, std::istream& stream) :
      state(state), stream(stream) { }

    Object* unmarshal();

    unsigned long get_varint(bool* done);
    unsigned long get_varint();
    Object* get_positive_varint();
    Object* get_negative_varint();
    String* get_string();
    Symbol* get_symbol();
    SendSite* get_sendsite();
    Tuple* get_tuple();

    Float* get_float();
    InstructionSequence* get_iseq();
    CompiledMethod* get_cmethod();

    class Error {
      const char* message_;

    public:
      Error(const char* msg)
        : message_(msg)
      {}

      const char* message() {
        return message_;
      }
    };
  };
}

#endif
