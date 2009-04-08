#ifndef RBX_BUILTIN_STATICSCOPE_HPP
#define RBX_BUILTIN_STATICSCOPE_HPP

#include "builtin/object.hpp"
#include "type_info.hpp"

namespace rubinius {

  class Module;

  class StaticScope : public Object {
  public:
    const static object_type type = StaticScopeType;

  private:
    Module* module_;      // slot

    // This is used like the ruby_class MRI variable. It lets
    // manipulate this aspect of the class lexical enclosure
    // without having to change module also.
    Module* current_module_;   // slot

    StaticScope* parent_; // slot

  public:
    /* accessors */

    attr_accessor(module, Module);
    attr_accessor(current_module, Module);
    attr_accessor(parent, StaticScope);

    /* interface */

    static void init(STATE);
    static StaticScope* create(STATE);

    // Ruby.primitive :static_scope_of_sender
    static StaticScope* of_sender(STATE, CallFrame* calling_environment);

    // The module to use when adding and removing methods
    Module* for_method_definition();

    bool top_level_p(STATE) {
      return parent_->nil_p();
    }

    class Info : public TypeInfo {
    public:
      BASIC_TYPEINFO(TypeInfo)
    };

  };
}

#endif
