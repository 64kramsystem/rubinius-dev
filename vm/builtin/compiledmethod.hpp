#ifndef RBX_BUILTIN_COMPILEDMETHOD_HPP
#define RBX_BUILTIN_COMPILEDMETHOD_HPP

#include "builtin/executable.hpp"
#include "vm.hpp"

namespace rubinius {

  class InstructionSequence;
  class MemoryPointer;
  class VMMethod;
  class StaticScope;
  class MachineMethod;

  class CompiledMethod : public Executable {
  public:
    const static object_type type = CompiledMethodType;

  private:
    Symbol* name_;               // slot
    InstructionSequence* iseq_; // slot
    Fixnum* stack_size_;         // slot
    Fixnum* local_count_;        // slot
    Fixnum* required_args_;      // slot
    Fixnum* total_args_;         // slot
    Object* splat_;              // slot
    Tuple* exceptions_;         // slot
    Tuple* lines_;              // slot
    Tuple* local_names_;        // slot
    Symbol* file_;               // slot
    StaticScope* scope_;        // slot

  public:
    // Access directly from assembly, so has to be public.
    Tuple* literals_;           // slot

    /* accessors */

    VMMethod* backend_method_;


    attr_accessor(name, Symbol);
    attr_accessor(iseq, InstructionSequence);
    attr_accessor(stack_size, Fixnum);
    attr_accessor(local_count, Fixnum);
    attr_accessor(required_args, Fixnum);
    attr_accessor(total_args, Fixnum);
    attr_accessor(splat, Object);
    attr_accessor(literals, Tuple);
    attr_accessor(exceptions, Tuple);
    attr_accessor(lines, Tuple);
    attr_accessor(local_names, Tuple);
    attr_accessor(file, Symbol);
    attr_accessor(scope, StaticScope);

    /* interface */

    static void init(STATE);

    // Ruby.primitive :compiledmethod_allocate
    static CompiledMethod* create(STATE);

    static Object* primitive_failed(STATE, CallFrame* call_frame, Dispatch& msg, Arguments& args);

    int start_line(STATE);
    int line(STATE, int ip);

    void post_marshal(STATE);
    size_t number_of_locals();
    VMMethod* formalize(STATE, bool ondemand=true);
    void specialize(STATE, TypeInfo* ti);

    static Object* default_executor(STATE, CallFrame*, Dispatch&, Arguments& args);

    // Ruby.primitive :compiledmethod_compile
    Object* compile(STATE);

    // Ruby.primitive :compiledmethod_make_machine_method
    MachineMethod* make_machine_method(STATE);

    // Ruby.primitive? :compiledmethod_activate
    Object* activate(STATE, Executable* exec, CallFrame* call_frame, Dispatch& msg, Arguments& args);

    bool is_rescue_target(STATE, int ip);

    // Ruby.primitive :compiledmethod_set_breakpoint
    Object* set_breakpoint(STATE, Fixnum* ip);

    // Ruby.primitive :compiledmethod_clear_breakpoint
    Object* clear_breakpoint(STATE, Fixnum* ip);

    // Ruby.primitive :compiledmethod_is_breakpoint
    Object* is_breakpoint(STATE, Fixnum* ip);

    // Ruby.primitive :compiledmethod_of_sender
    static CompiledMethod* of_sender(STATE, CallFrame* calling_environment);

    class Info : public TypeInfo {
    public:
      BASIC_TYPEINFO(TypeInfo)
      virtual void show(STATE, Object* self, int level);
    };
  };

};


#endif
