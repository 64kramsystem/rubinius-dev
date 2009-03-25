#include "builtin/object.hpp"
#include "builtin/module.hpp"
#include "builtin/symbol.hpp"

#include "helpers.hpp"

#include "capi/capi.hpp"
#include "capi/ruby.h"

using namespace rubinius;
using namespace rubinius::capi;

extern "C" {
  int rb_const_defined(VALUE module_handle, ID const_id) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    VALUE result = rb_funcall(module_handle, rb_intern("const_defined?"), 1, ID2SYM(const_id));
    return RBX_RTEST(env->get_object(result));
  }

  VALUE rb_const_get(VALUE module_handle, ID name) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    Module* module = c_as<Module>(env->get_object(module_handle));

    return env->get_handle(module->get_const(env->state(),
                                                 reinterpret_cast<Symbol*>(name)));
  }

  void rb_define_alias(VALUE module_handle, const char* new_name, const char* old_name) {
    ID id_new = rb_intern(new_name);
    ID id_old = rb_intern(old_name);

    rb_funcall(module_handle, rb_intern("alias_method"), 2, id_new, id_old);
  }

  void rb_define_alloc_func(VALUE class_handle, CApiAllocFunction allocator) {
    rb_define_singleton_method(class_handle, "allocate", allocator, 0);
  }

  void rb_define_attr(VALUE module_handle, const char* attr_name, int readable, int writable) {
    if(readable) {
      rb_funcall(module_handle, rb_intern("attr_reader"), 1, rb_intern(attr_name));
    }

    if(writable) {
      rb_funcall(module_handle, rb_intern("attr_writer"), 1, rb_intern(attr_name));
    }
  }

  void rb_define_const(VALUE module_handle, const char* name, VALUE obj_handle) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    Module* module = c_as<Module>(env->get_object(module_handle));
    Object* object = env->get_object(obj_handle);

    module->set_const(env->state(), name,  object);
  }

  VALUE rb_define_module(const char* name) {
    return rb_define_module_under(rb_cObject, name);
  }

  void rb_define_module_function(VALUE module_handle, const char* name,
      CApiGenericFunction func, int args) {
    rb_define_private_method(module_handle, name, func, args);
    rb_define_singleton_method(module_handle, name, func, args);
  }

  /** @note   Shares code with rb_define_class_under, change there too. --rue */
  VALUE rb_define_module_under(VALUE parent_handle, const char* name) {
    NativeMethodEnvironment* env = NativeMethodEnvironment::get();

    Module* parent = c_as<Module>(env->get_object(parent_handle));
    Symbol* constant = env->state()->symbol(name);

    Module* module = rubinius::Helpers::open_module(env->state(),
        env->current_call_frame(), parent, constant);

    return env->get_handle_global(module);
  }

  void rb_include_module(VALUE includer_handle, VALUE includee_handle) {
    rb_funcall(includer_handle, rb_intern("include"), 1, includee_handle);
  }

  void rb_undef_method(VALUE module_handle, const char* name) {
    rb_funcall(module_handle, rb_intern("undef_method"), 1, ID2SYM(rb_intern(name)));
  }
}
