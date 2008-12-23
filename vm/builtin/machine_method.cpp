#include "prelude.hpp"
#include "builtin/class.hpp"
#include "builtin/machine_method.hpp"
#include "vm/exception.hpp"

#include "detection.hpp"

// #define MM_DEBUG

namespace rubinius {

  void MachineMethod::init(STATE) {
    GO(machine_method).set(state->new_class_under("MachineMethod", G(rubinius)));
  }

  static void* adjust(void* old_base, void* new_base, void* address) {
    uintptr_t diff = reinterpret_cast<uintptr_t>(new_base) -
                     reinterpret_cast<uintptr_t>(old_base);

    return reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(address) + diff);
  }

  MachineMethod* MachineMethod::create(STATE, VMMethod* vmm, JITCompiler& jit) {
    size_t code_size = jit.assembler().used_bytes();
    MachineMethod* mm = state->new_struct<MachineMethod>(G(machine_method));

    mm->vmmethod_ = vmm;
    mm->code_size_ = code_size;

    mm->set_function(reinterpret_cast<void*>(malloc(code_size)));
    std::memcpy(mm->function(), jit.assembler().buffer(), code_size);

    assembler::Relocations& current = jit.assembler().relocations();
    mm->relocations_ = new assembler::Relocation*[current.size()];

    int j = 0;
    for(assembler::Relocations::iterator i = current.begin();
        i != current.end();
        i++, j++) {
      assembler::Relocation* rel = new assembler::Relocation(*i->second);
      rel->adjust_base(jit.assembler().buffer(), mm->function());
      mm->relocations_[j] = rel;

      rel->resolve_and_write();
    }

    mm->virtual2native_ = new CodeMap();
    CodeMap& v2n = *mm->virtual2native_;

    for(CodeMap::iterator i = jit.code_map().begin();
        i != jit.code_map().end();
        i++) {
      v2n[i->first] = adjust(jit.assembler().buffer(), mm->function(), i->second);
    }

    return mm;
  }

  void* MachineMethod::resolve_virtual_ip(opcode ip) {
    CodeMap::iterator i = virtual2native_->find(ip);
    if(i == virtual2native_->end()) return NULL;
    return i->second;
  }

  Object* MachineMethod::show() {
    std::cout << "== stats ==\n";
    std::cout << "number of bytecodes: " << vmmethod_->total << "\n";
    std::cout << " bytes of assembley: " << code_size_ << "\n";
    double ratio = (double)code_size_ / (double)vmmethod_->total;
    std::cout << "       direct ratio: " << ratio << "\n";
    ratio = (double)code_size_ / ((double)vmmethod_->total * sizeof(rubinius::opcode));
    std::cout << "       memory ratio: " << ratio << "\n";
    std::cout << "\n== x86 assembly ==\n";
    assembler_x86::AssemblerX86::show_buffer(function(), code_size_);
    return Qnil;
  }

  void MachineMethod::run_code(VMMethod* const vmm, Task* const task,
      MethodContext* const ctx) {
#ifdef IS_X86
    MachineMethod* mm = vmm->machine_method();
    void* func = mm->function();
    ((Runner)func)(vmm, task, ctx);
#else
    Assertion::raise("Only supported on x86");
#endif
  }

  Object* MachineMethod::activate() {
#ifdef IS_X86
#ifdef MM_DEBUG
    vmmethod_->run = MachineMethod::run_code;
#else
    vmmethod_->run = (Runner)function();
#endif
    vmmethod_->set_machine_method(this);
    return Qtrue;
#else
    Assertion::raise("Only supported on x86");
    return Qfalse;
#endif
  }
}
