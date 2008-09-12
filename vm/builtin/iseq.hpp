#ifndef RBX_BUILTIN_ISEQ_HPP
#define RBX_BUILTIN_ISEQ_HPP

#include "builtin/object.hpp"
#include "type_info.hpp"

namespace rubinius {
  class Tuple;

  class InstructionSequence : public Object {
  public:
    const static size_t fields = 2;
    const static object_type type = ISeqType;

  private:
    Tuple* opcodes_;     // slot
    FIXNUM stack_depth_; // slot

  public:
    /* accessors */

    attr_accessor(opcodes, Tuple);
    attr_accessor(stack_depth, Fixnum);

    /* interface */

    static void init(STATE);
    static InstructionSequence* create(STATE, size_t instructions);

    static size_t instruction_width(size_t op);

    void post_marshal(STATE);

    class Info : public TypeInfo {
    public:
      BASIC_TYPEINFO(TypeInfo)
    };

#include "gen/iseq_instruction_names.hpp"
  };

}

#endif
