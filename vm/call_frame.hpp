#ifndef RBX_CALL_FRAME_HPP
#define RBX_CALL_FRAME_HPP

#include "vmmethod.hpp"
#include "unwind_info.hpp"
#include "jit_state.h"
#include "builtin/variable_scope.hpp"

namespace rubinius {
  class Object;
  class Symbol;
  class Module;
  class VMMethod;
  class VariableScope;

  struct CallFrame {
    enum Flags {
      cIsLambda = 1
    };

    CallFrame* previous;
    StaticScope* static_scope;

    bool is_block;
    Symbol* name;
    CompiledMethod* cm;

    int flags;
    int args;
    int ip;
    intptr_t native_ip;

    int stack_size;
    struct jit_state js;

    VariableScope* top_scope;
    VariableScope* scope;

    int current_unwind;
    UnwindInfo unwinds[kMaxUnwindInfos];

    // MUST BE AT THE END!
    Object* stk[];

    /**
     *  Initialize frame for the given stack size.
     */
    void prepare(int stack) {
      is_block = false;
      ip = 0;
      native_ip = 0;
      current_unwind = 0;
      stack_size = stack;

      for(int i = 0; i < stack; i++) {
        stk[i] = Qnil;
      }

      js.stack = stk - 1;
      js.stack_top = (stk + stack) - 1;
    }

    Object* self() {
      return scope->self();
    }

    Module* module() {
      return top_scope->module();
    }

    /* Stack manipulation functions */

    void clear_stack(size_t amount) {
      js.stack -= amount;
#ifdef EXTRA_STACK_PROTECTION
      if(amount > 0) {
        assert(js.stack >= stk - 1);
      }
#endif
    }

    Object* pop() {
#ifdef EXTRA_STACK_PROTECTION
      assert(js.stack >= stk);
#endif
      return *js.stack--;
    }

    void push(Object* value) {
      *++js.stack = value;
#ifdef EXTRA_STACK_PROTECTION
      assert(js.stack < js.stack_top);
#endif
    }

    Object* stack_back(size_t position) {
      Object** pos = js.stack - position;
      return *pos;
    }

    /**
     * Returns a pointer to the last 'position' + 1 objects
     * on the stack.
     */
    Object** stack_back_position(size_t position) {
      return js.stack - position;
    }

    Object* top() {
      return *js.stack;
    }

    void set_top(Object* val) {
      *js.stack = val;
    }

    Object* stack_at(size_t pos) {
      return stk[pos];
    }

    void stack_put(size_t pos, Object* val) {
      stk[pos] = val;
    }

    void position_stack(int pos) {
      js.stack = stk + pos;
#ifdef EXTRA_STACK_PROTECTION
      if(pos > 0) {
        assert(js.stack >= stk && js.stack < js.stack_top);
      }
#endif
    }

    int calculate_sp() {
      return js.stack - stk;
    }

    void set_ip(int new_ip) {
      ip = new_ip;
    }

    void set_native_ip(void * new_ip) {
      native_ip = reinterpret_cast<intptr_t>(new_ip);
    }

    // Manage the dynamic Unwind stack for this context
    void push_unwind(int target_ip, UnwindType type) {
      assert(current_unwind < kMaxUnwindInfos);
      UnwindInfo& info = unwinds[current_unwind++];
      info.target_ip = target_ip;
      info.stack_depth = calculate_sp();
      info.type = type;
    }

    UnwindInfo& pop_unwind() {
      assert(current_unwind > 0);
      return unwinds[--current_unwind];
    }

    bool has_unwinds_p() {
      return current_unwind > 0;
    }

    void promote_scope(STATE);

    void print_backtrace(STATE);
    int line(STATE);

    bool scope_still_valid(VariableScope* scope);
  };
};

#define ALLOCA_CALLFRAME(vmm) ((CallFrame*)alloca(sizeof(CallFrame) + (vmm->stack_size * sizeof(Object*))))

#endif
