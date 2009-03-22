#ifndef RBX_BUILTIN_TUPLE_HPP
#define RBX_BUILTIN_TUPLE_HPP

#include "builtin/object.hpp"
#include "builtin/exception.hpp"
#include "type_info.hpp"

namespace rubinius {
  class Tuple : public Object {
  public:
    const static object_type type = TupleType;

    native_int full_size_;

    /* Body access */
    Object* field[];

  public:
    uint32_t num_fields() const {
      return (full_size_ - sizeof(Tuple)) / sizeof(Object*);
    }

    static Tuple* create(STATE, size_t fields);
    static Tuple* from(STATE, size_t fields, ...);

    // Ruby.primitive :tuple_allocate
    static Tuple* allocate(STATE, Fixnum* fields);

    // Ruby.primitive :tuple_at
    Object* at_prim(STATE, Fixnum* pos);

    Object* put(STATE, size_t idx, Object* val);

    // Ruby.primitive :tuple_put
    Object* put_prim(STATE, Fixnum* idx, Object* val);

    // Ruby.primitive :tuple_fields
    Object* fields_prim(STATE);

    // Ruby.primitive :tuple_pattern
    static Tuple* pattern(STATE, Fixnum* size, Object* val);

    // Ruby.primitive :tuple_copy_from
    Tuple* copy_from(STATE, Tuple* other, Fixnum *start, Fixnum *length, Fixnum *dest);

    // Ruby.primitive :tuple_delete_inplace
    Fixnum* delete_inplace(STATE, Fixnum *start, Fixnum *length, Object *obj);

    // Ruby.primitive :tuple_create_weakref
    static Tuple* create_weakref(STATE, Object* obj);

  public: // Inline Functions
    Object* at(STATE, size_t index) {
      if(num_fields() <= index) {
        Exception::object_bounds_exceeded_error(state, this, index);
        return NULL;
      }
      return field[index];
    }

  public: // Rubinius Type stuff
    class Info : public TypeInfo {
    public:
      Info(object_type type, bool cleanup = false) : TypeInfo(type, cleanup) { }
      virtual void mark(Object* t, ObjectMark& mark);
      virtual void show(STATE, Object* self, int level);
      virtual void show_simple(STATE, Object* self, int level);
      virtual void visit(Object*, ObjectVisitor& visit);

      virtual void auto_mark(Object* obj, ObjectMark& mark) {}
      virtual size_t object_size(const ObjectHeader* object);
    };
  };
};

#endif
