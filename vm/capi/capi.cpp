#include "builtin/array.hpp"
#include "builtin/data.hpp"
#include "builtin/fixnum.hpp"
#include "builtin/lookuptable.hpp"
#include "builtin/module.hpp"
#include "builtin/nativemethod.hpp"
#include "builtin/object.hpp"
#include "builtin/string.hpp"
#include "builtin/symbol.hpp"

#include "exception_point.hpp"
#include "global_cache.hpp"
#include "message.hpp"

#include "capi/capi.hpp"
#include "capi/ruby.h"

#include <string>
#include <vector>
#include <tr1/unordered_map>

namespace rubinius {
  namespace capi {

    typedef std::vector<std::string> CApiConstantNameMap;
    typedef std::tr1::unordered_map<int, Handle> CApiConstantHandleMap;

    /**
     * This looks like a complicated scheme but there is a reason for
     * doing it this way. In MRI, rb_cObject, etc. are all global data.
     * We need to avoid global data to better support embedding and
     * other features like MVM. @see capi_get_constant().
     */
    std::string& capi_get_constant_name(int type) {
      static CApiConstantNameMap map;

      if(map.empty()) {
        map.resize(cCApiMaxConstant + 1);

        map[cCApiArray]      = "Array";
        map[cCApiBignum]     = "Bignum";
        map[cCApiClass]      = "Class";
        map[cCApiComparable] = "Comparable";
        map[cCApiData]       = "Data";
        map[cCApiEnumerable] = "Enumerable";
        map[cCApiFalse]      = "FalseClass";
        map[cCApiFixnum]     = "Fixnum";
        map[cCApiFloat]      = "Float";
        map[cCApiHash]       = "Hash";
        map[cCApiInteger]    = "Integer";
        map[cCApiIO]         = "IO";
        map[cCApiKernel]     = "Kernel";
        map[cCApiModule]     = "Module";
        map[cCApiNil]        = "NilClass";
        map[cCApiObject]     = "Object";
        map[cCApiRegexp]     = "Regexp";
        map[cCApiString]     = "String";
        map[cCApiSymbol]     = "Symbol";
        map[cCApiThread]     = "Thread";
        map[cCApiTrue]       = "TrueClass";

        map[cCApiArgumentError]       = "ArgumentError";
        map[cCApiEOFError]            = "EOFError";
        map[cCApiErrno]               = "Errno";
        map[cCApiException]           = "Exception";
        map[cCApiFatal]               = "Fatal";
        map[cCApiFloatDomainError]    = "FloatDomainError";
        map[cCApiIndexError]          = "IndexError";
        map[cCApiInterrupt]           = "Interrupt";
        map[cCApiIOError]             = "IOError";
        map[cCApiLoadError]           = "LoadError";
        map[cCApiLocalJumpError]      = "LocalJumpError";
        map[cCApiNameError]           = "NameError";
        map[cCApiNoMemoryError]       = "NoMemoryError";
        map[cCApiNoMethodError]       = "NoMethodError";
        map[cCApiNotImplementedError] = "NotImplementedError";
        map[cCApiRangeError]          = "RangeError";
        map[cCApiRegexpError]         = "RegexpError";
        map[cCApiRuntimeError]        = "RuntimeError";
        map[cCApiScriptError]         = "ScriptError";
        map[cCApiSecurityError]       = "SecurityError";
        map[cCApiSignalException]     = "SignalException";
        map[cCApiStandardError]       = "StandardError";
        map[cCApiSyntaxError]         = "SyntaxError";
        map[cCApiSystemCallError]     = "SystemCallError";
        map[cCApiSystemExit]          = "SystemExit";
        map[cCApiSystemStackError]    = "SystemStackError";
        map[cCApiTypeError]           = "TypeError";
        map[cCApiThreadError]         = "ThreadError";
        map[cCApiZeroDivisionError]   = "ZeroDivisionError";
      }

      if(type < 0 || type >= cCApiMaxConstant) {
        NativeMethodEnvironment* env = NativeMethodEnvironment::get();
        rb_raise(env->get_handle_global(env->state()->globals.exception.get()),
              "C-API: invalid constant index");
      }

      return map[type];
    }

    /**
     *  Common implementation for rb_funcall*
     *
     *  @todo   Set up permanent SendSites through macroing?
     *  @todo   Stricter action check?
     */
    VALUE capi_funcall_backend(const char* file, int line,
        VALUE receiver, ID method_name, std::size_t arg_count, VALUE* arg_array) {
      NativeMethodEnvironment* env = NativeMethodEnvironment::get();
      Array* args = Array::create(env->state(), arg_count);

      for(size_t i = 0; i < arg_count; i++) {
        args->set(env->state(), i, env->get_object(arg_array[i]));
      }

      Object* recv = env->get_object(receiver);
      Object* ret = recv->send(env->state(), env->current_call_frame(),
          reinterpret_cast<Symbol*>(method_name), args, RBX_Qnil);

      // An exception occurred
      if(!ret) env->current_ep()->return_to(env);

      return env->get_handle(ret);
    }

    /** Make sure the name has the given prefix. */
    Symbol* prefixed_by(std::string prefix, std::string name) {
      NativeMethodEnvironment* env = NativeMethodEnvironment::get();

      if(name.compare(0UL, prefix.size(), prefix) != 0) {
        std::ostringstream str;
        str << prefix << name;
        name.assign(str.str());
      }

      /* @todo Need to strdup here to not point to junk but can it leak? */
      return env->state()->symbol(strdup(name.c_str()));
    }

    /** Make sure the name has the given prefix. */
    Symbol* prefixed_by(std::string prefix, ID name) {
      NativeMethodEnvironment* env = NativeMethodEnvironment::get();

      return prefixed_by(prefix, reinterpret_cast<Symbol*>(name)->c_str(env->state()));
    }

    void capi_raise_runtime_error(const char* reason) {
      rb_raise(rb_eRuntimeError, reason);
    }

    void capi_raise_type_error(object_type type, Object* object) {
      NativeMethodEnvironment* env = NativeMethodEnvironment::get();

      TypeInfo* expected = env->state()->find_type(type);
      TypeInfo* actual = env->state()->find_type(object->type_id());

      rb_raise(rb_eTypeError, "wrong argument type %s (expected %s)",
          actual->type_name.c_str(), expected->type_name.c_str());
    }
  }
}

using namespace rubinius;
using namespace rubinius::capi;

extern "C" {

  VALUE capi_get_constant(CApiConstant type) {
    static CApiConstantHandleMap map;

    CApiConstantHandleMap::iterator entry = map.find(type);
    if(entry == map.end()) {
      NativeMethodEnvironment* env = NativeMethodEnvironment::get();
      Object* obj = env->state()->globals.object.get()->get_const(env->state(),
          capi_get_constant_name(type).c_str());

      Handle handle = env->get_handle_global(obj);
      map[type] = handle;
      return handle;
    } else {
      return entry->second;
    }
  }

  VALUE capi_rb_funcall(const char* file, int line,
      VALUE receiver, ID method_name, int arg_count, ...) {
    va_list varargs;
    va_start(varargs, arg_count);

    VALUE* args = new VALUE[arg_count];

    for (int i = 0; i < arg_count; ++i) {
      args[i] = va_arg(varargs, VALUE);
    }

    va_end(varargs);

    VALUE ret = capi_funcall_backend(file, line, receiver,
        method_name, arg_count, args);

    delete[] args;
    return ret;
  }

  VALUE capi_rb_funcall2(const char* file, int line,
      VALUE receiver, ID method_name, int arg_count, VALUE* args) {
    return capi_funcall_backend(file, line, receiver, method_name, arg_count, args);
  }

  VALUE capi_id2sym(ID id) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    return env->get_handle(reinterpret_cast<Symbol*>(id));
  }

  void capi_infect(VALUE obj1, VALUE obj2) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    Object* object1 = env->get_object(obj1);
    Object* object2 = env->get_object(obj2);

    object1->infect(env->state(), object2);
  }

  int capi_nil_p(VALUE expression_result) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    return RBX_NIL_P(env->get_object(expression_result));
  }

  long capi_rstring_len(VALUE string_handle) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    String* string = c_as<String>(env->get_object(string_handle));

    return string->size();
  }

  char* capi_rstring_ptr(VALUE string_handle) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    String* string = c_as<String>(env->get_object(string_handle));

    return string->byte_address();
  }

  int capi_rtest(VALUE expression_result) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    return RBX_RTEST(env->get_object(expression_result));
  }

  ID capi_sym2id(VALUE symbol_handle) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    return reinterpret_cast<ID>(env->get_object(symbol_handle));
  }

  void capi_define_method(const char* file, VALUE target,
      const char* name, CApiGenericFunction fptr, int arity, CApiMethodKind kind) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    VM* state = env->state();
    Symbol* method_name = state->symbol(name);

    Module* module = NULL;

    if (kind == cCApiSingletonMethod) {
      module = c_as<Module>(env->get_object(target)->metaclass(env->state()));
    }
    else {
      module = c_as<Module>(env->get_object(target));
    }

    NativeMethod* method = NULL;
    method = NativeMethod::create(state,
                                  String::create(state, file),
                                  module,
                                  method_name,
                                  fptr,
                                  Fixnum::from(arity));

    MethodVisibility* visibility = MethodVisibility::create(state);
    visibility->method(state, method);

    switch(kind) {
    case cCApiPrivateMethod:
      visibility->visibility(state, state->symbol("private"));
      break;

    case cCApiProtectedMethod:
      visibility->visibility(state, state->symbol("protected"));
      break;

    default:  /* Also catches singletons for now. @todo Verify OK. --rue */
      visibility->visibility(state, state->symbol("public"));
      break;
    }

    module->method_table()->store(state, method_name, visibility);
    state->global_cache->clear(module, method_name);
  }

  VALUE capi_class_superclass(VALUE class_handle) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    Module* module = c_as<Module>(env->get_object(class_handle));
    Module* super = module->superclass();

    if(super->nil_p()) {
      /* In MRI, the superclass chain terminates with a NULL pointer.
       * Since VALUE is an intptr_t in Rubinius, we use 0 instead.
       */
      return 0;
    } else {
      return env->get_handle(super);
    }
  }

  /* For debugging. */
  void __show_subtend__(VALUE obj_handle) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    Object* object = env->get_object(obj_handle);
    if(!object) {
      std::cout << "the object is NULL, check if an exception was raised." << std::endl;
      return;
    }
    object->show(env->state());
  }
}
