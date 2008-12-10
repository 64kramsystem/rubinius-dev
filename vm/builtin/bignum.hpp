#ifndef RBX_BUILTIN_BIGNUM_HPP
#define RBX_BUILTIN_BIGNUM_HPP

#include "builtin/integer.hpp"
#include "tommath.h"

namespace rubinius {
  class Array;
  class String;
  class Float;

  class Bignum : public Integer {
  public:
    const static object_type type = BignumType;

    mp_int mp_val_;

    static void init(STATE);
    static Bignum* create(STATE);

    static Bignum* from(STATE, int num);
    static Bignum* from(STATE, unsigned int num);
    static Bignum* from(STATE, long num);
    static Bignum* from(STATE, unsigned long);
    static Bignum* from(STATE, long long val);
    static Bignum* from(STATE, unsigned long long val);

    mp_int* mp_val() {
      return &mp_val_;
    }

    native_int         to_native();

    int                to_int();
    unsigned int       to_uint();
    long               to_long();
    unsigned long      to_ulong();
    long long          to_long_long();
    unsigned long long to_ulong_long();

    static Integer* normalize(STATE, Bignum* obj);
    static Integer* from_string_detect(STATE, const char* str);
    static Integer* from_string(STATE, const char* str, size_t radix);
    static Integer* from_double(STATE, double d);

    // Ruby.primitive :bignum_new
    static Bignum* create(STATE, Fixnum* f);

    // Ruby.primitive :bignum_from_float
    static Integer* from_float(STATE, Float* f);

    // Ruby.primitive! :bignum_initialize_copy
    Bignum* initialize_copy(STATE, Bignum* other);

    // Ruby.primitive! :bignum_add
    Integer* add(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_add
    Integer* add(STATE, Bignum* b);
    // Ruby.primitive! :bignum_add
    Float*  add(STATE, Float* b);

    // Ruby.primitive! :bignum_sub
    Integer* sub(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_sub
    Integer* sub(STATE, Bignum* b);
    // Ruby.primitive! :bignum_sub
    Float*  sub(STATE, Float* b);

    // Ruby.primitive! :bignum_mul
    Integer* mul(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_mul
    Integer* mul(STATE, Bignum* b);
    // Ruby.primitive! :bignum_mul
    Float*  mul(STATE, Float* b);

    Integer* divide(STATE, Fixnum* b, Integer** mod);
    Integer* divide(STATE, Bignum* b, Integer** mod);

    // Ruby.primitive! :bignum_div
    Integer* div(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_div
    Integer* div(STATE, Bignum* b);
    // Ruby.primitive! :bignum_div
    Float*  div(STATE, Float* b);

    // Ruby.primitive! :bignum_divmod
    Array* divmod(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_divmod
    Array* divmod(STATE, Bignum* b);
    // Ruby.primitive! :bignum_divmod
    Array* divmod(STATE, Float* b);

    // Ruby.primitive! :bignum_mod
    Integer* mod(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_mod
    Integer* mod(STATE, Bignum* b);
    // Ruby.primitive! :bignum_mod
    Float* mod(STATE, Float* b);

    // Ruby.primitive :bignum_invert
    Integer* invert(STATE);
    // Ruby.primitive :bignum_neg
    Integer* neg(STATE);

    // Ruby.primitive! :bignum_equal
    Object* equal(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_equal
    Object* equal(STATE, Bignum* b);
    // Ruby.primitive! :bignum_equal
    Object* equal(STATE, Float* b);

    // Ruby.primitive! :bignum_compare
    Fixnum* compare(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_compare
    Fixnum* compare(STATE, Bignum* b);
    // Ruby.primitive! :bignum_compare
    Fixnum* compare(STATE, Float* b);

    // Ruby.primitive! :bignum_and
    Integer* bit_and(STATE, Integer* b);
    // Ruby.primitive! :bignum_and
    Integer* bit_and(STATE, Float* b);

    // Ruby.primitive! :bignum_or
    Integer* bit_or(STATE, Integer* b);
    // Ruby.primitive! :bignum_or
    Integer* bit_or(STATE, Float* b);

    // Ruby.primitive! :bignum_xor
    Integer* bit_xor(STATE, Integer* b);
    // Ruby.primitive! :bignum_xor
    Integer* bit_xor(STATE, Float* b);

    // Ruby.primitive :bignum_left_shift
    Integer* left_shift(STATE, Fixnum* bits);
    // Ruby.primitive :bignum_right_shift
    Integer* right_shift(STATE, Fixnum* bits);

    // Ruby.primitive! :bignum_gt
    Object* gt(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_gt
    Object* gt(STATE, Bignum* b);
    // Ruby.primitive! :bignum_gt
    Object* gt(STATE, Float* b);

    // Ruby.primitive! :bignum_ge
    Object* ge(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_ge
    Object* ge(STATE, Bignum* b);
    // Ruby.primitive! :bignum_ge
    Object* ge(STATE, Float* b);

    // Ruby.primitive! :bignum_lt
    Object* lt(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_lt
    Object* lt(STATE, Bignum* b);
    // Ruby.primitive! :bignum_lt
    Object* lt(STATE, Float* b);

    // Ruby.primitive! :bignum_le
    Object* le(STATE, Fixnum* b);
    // Ruby.primitive! :bignum_le
    Object* le(STATE, Bignum* b);
    // Ruby.primitive! :bignum_le
    Object* le(STATE, Float* b);

    // Ruby.primitive :bignum_to_float
    Float* to_float(STATE);

    // Ruby.primitive! :bignum_coerce
    Array* coerce(STATE, Bignum* other);
    // Ruby.primitive! :bignum_coerce
    Array* coerce(STATE, Fixnum* other);

    // Ruby.primitive :bignum_to_s
    String* to_s(STATE, Integer* radix);
    void   into_string(STATE, size_t radix, char* buf, size_t sz);
    double to_double(STATE);

    // Ruby.primitive :bignum_size
    Integer* size(STATE);
    hashval hash_bignum(STATE);

    class Info : public TypeInfo {
    public:
      Info(object_type type, bool cleanup = true) : TypeInfo(type, true) { }
      virtual void mark(Object* t, ObjectMark& mark);
      virtual void cleanup(Object* obj);
      virtual void show(STATE, Object* self, int level);
      virtual void show_simple(STATE, Object* self, int level);
    };
  };

}

#endif
