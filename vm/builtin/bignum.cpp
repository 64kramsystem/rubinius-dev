/* The implementation of Bignum, providing infinite size integers */

#include <ctype.h>
#include <math.h>
#include <cmath>
#include <iostream>

#include "vm/object_utils.hpp"
#include "vm.hpp"
#include "objectmemory.hpp"
#include "builtin/array.hpp"
#include "builtin/class.hpp"
#include "builtin/exception.hpp"
#include "builtin/fixnum.hpp"
#include "builtin/float.hpp"
#include "builtin/string.hpp"

#define BASIC_CLASS(blah) G(blah)
#define NEW_STRUCT(obj, str, kls, kind) \
  obj = (typeof(obj))Bignum::create(state); \
  str = (kind *)(obj->mp_val())
#define DATA_STRUCT(obj, type) ((type)(obj->mp_val()))

#define NMP mp_int *n; Bignum* n_obj; \
  NEW_STRUCT(n_obj, n, BASIC_CLASS(bignum), mp_int);

#define MMP mp_int *m; Bignum* m_obj; \
  NEW_STRUCT(m_obj, m, BASIC_CLASS(bignum), mp_int);


#define BDIGIT_DBL long long
#define DIGIT_RADIX (1UL << DIGIT_BIT)

namespace rubinius {

  /*
   * LibTomMath actually stores stuff internally in longs.
   * The problem with it's API is that it's int based, so
   * this will create problems on 64 bit platforms if
   * everything is cut down to 32 bits. Hence the existence
   * of the mp_set_long, mp_init_set_long and mp_get_long
   * functions here.
   */
  static int mp_set_long (mp_int * a, unsigned long b)
  {
    int     err;
    // @todo Move these two values to bignum.h
    size_t  x = 0;
    size_t  count = sizeof(unsigned long) * 2;
    size_t  shift_width = (sizeof(unsigned long) * 8) - 4;

    mp_zero (a);

    /* set four bits at a time */
    for (x = 0; x < count; x++) {
      /* shift the number up four bits */
      if ((err = mp_mul_2d (a, 4, a)) != MP_OKAY) {
        return err;
      }

      /* OR in the top four bits of the source */
      a->dp[0] |= (b >> shift_width) & 15;

      /* shift the source up to the next four bits */
      b <<= 4;

      /* ensure that digits are not clamped off */
      a->used += 1;
    }
    mp_clamp (a);
    return MP_OKAY;
  }

  static unsigned long mp_get_long (mp_int * a)
  {
      int i;
      unsigned long res;

      if (a->used == 0) {
         return 0;
      }

      /* get number of digits of the lsb we have to read */
      i = MIN(a->used,(int)((sizeof(unsigned long)*CHAR_BIT+DIGIT_BIT-1)/DIGIT_BIT))-1;

      /* get most significant digit of result */
      res = DIGIT(a,i);

      while (--i >= 0) {
        res = (res << DIGIT_BIT) | DIGIT(a,i);
      }

      /* force result to 32-bits always so it is consistent on non 32-bit platforms */
      return res;
  }

  static void twos_complement(mp_int *a)
  {
    long i = a->used;
    BDIGIT_DBL num;

    while (i--) {
      DIGIT(a,i) = (~DIGIT(a,i)) & (DIGIT_RADIX-1);
    }

    i = 0; num = 1;
    do {
      num += DIGIT(a,i);
      DIGIT(a,i++) = num & (DIGIT_RADIX-1);
      num = num >> DIGIT_BIT;
    } while (i < a->used);

  }

#define BITWISE_OP_AND 1
#define BITWISE_OP_OR  2
#define BITWISE_OP_XOR 3

  static void bignum_bitwise_op(int op, mp_int *x, mp_int *y, mp_int *n)
  {
    mp_int   a,   b;
    mp_int *d1, *d2;
    int i, sign,  l1,  l2;
    mp_init(&a) ; mp_init(&b);

    if (y->sign == MP_NEG) {
      mp_copy(y, &b);
      twos_complement(&b);
      y = &b;
    }

    if (x->sign == MP_NEG) {
      mp_copy(x, &a);
      twos_complement(&a);
      x = &a;
    }

    if (x->used > y->used) {
      l1 = y->used;
      l2 = x->used;
      d1 = y;
      d2 = x;
      sign = y->sign;
    } else {
      l1 = x->used;
      l2 = y->used;
      d1 = x;
      d2 = y;
      sign = x->sign;
    }

    mp_grow(n, l2);
    n->used = l2;
    n->sign = MP_ZPOS;
    switch(op) {
      case BITWISE_OP_AND:
        if (x->sign == MP_NEG && y->sign == MP_NEG) n->sign = MP_NEG;
        for (i=0; i < l1; i++) {
          DIGIT(n,i) = DIGIT(d1,i) & DIGIT(d2,i);
        }
        for (; i < l2; i++) {
          DIGIT(n,i) = (sign == MP_ZPOS)?0:DIGIT(d2,i);
        }
        break;
      case BITWISE_OP_OR:
        if (x->sign == MP_NEG || y->sign == MP_NEG) n->sign = MP_NEG;
        for (i=0; i < l1; i++) {
          DIGIT(n,i) = DIGIT(d1,i) | DIGIT(d2,i);
        }
        for (; i < l2; i++) {
          DIGIT(n,i) = (sign == MP_ZPOS)?DIGIT(d2,i):(DIGIT_RADIX-1);
        }
        break;
      case BITWISE_OP_XOR:
        if (x->sign != y->sign) n->sign = MP_NEG;
        for (i=0; i < l1; i++) {
          DIGIT(n,i) = DIGIT(d1,i) ^ DIGIT(d2,i);
        }
        for (; i < l2; i++) {
          DIGIT(n,i) = (sign == MP_ZPOS)?DIGIT(d2,i):~DIGIT(d2,i);
        }
        break;
    }

    if (n->sign == MP_NEG) twos_complement(n);

    /* free allocated resources for twos complement copies */
    mp_clear(&a);
    mp_clear(&b);
  }

  void Bignum::Info::cleanup(Object* obj) {
    Bignum* big = as<Bignum>(obj);
    mp_int *n = big->mp_val();
    mp_clear(n);
  }

  void Bignum::Info::mark(Object* obj, ObjectMark& mark) { }

  void Bignum::Info::show(STATE, Object* self, int level) {
    Bignum* b = as<Bignum>(self);
    std::cout << b->to_s(state, Fixnum::from(10))->c_str() << std::endl;
  }

  void Bignum::Info::show_simple(STATE, Object* self, int level) {
    show(state, self, level);
  }

  void Bignum::init(STATE) {
    GO(bignum).set(state->new_class("Bignum", G(integer)));
    G(bignum)->set_object_type(state, BignumType);
    state->add_type_info(new Bignum::Info(Bignum::type));
  }

  Bignum* Bignum::create(STATE) {
    Bignum* o;
    o = (Bignum*)state->new_struct(G(bignum), sizeof(mp_int));
    mp_init(o->mp_val());
    return o;
  }

  Bignum* Bignum::initialize_copy(STATE, Bignum* other) {
    mp_copy(mp_val(), other->mp_val());
    return this;
  }

  Bignum* Bignum::from(STATE, int num) {
    mp_int *a;
    Bignum* o;
    o = Bignum::create(state);
    a = o->mp_val();

    if(num < 0) {
      mp_set_int(a, (unsigned int)-num);
      a->sign = MP_NEG;
    } else {
      mp_set_int(a, (unsigned int)num);
    }
    return o;
  }

  Bignum* Bignum::from(STATE, unsigned int num) {
    Bignum* o;
    o = Bignum::create(state);
    mp_set_int(o->mp_val(), num);
    return o;
  }

  Bignum* Bignum::from(STATE, long num) {
    mp_int *a;
    Bignum* o;
    o = Bignum::create(state);
    a = o->mp_val();

    if(num < 0) {
      mp_set_long(a, (unsigned long)-num);
      a->sign = MP_NEG;
    } else {
      mp_set_long(a, (unsigned long)num);
    }
    return o;
  }

  Bignum* Bignum::from(STATE, unsigned long num) {
    Bignum* o;
    o = Bignum::create(state);
    mp_set_long(o->mp_val(), num);
    return o;
  }

  Bignum* Bignum::from(STATE, unsigned long long val) {
    mp_int low, high;
    mp_int* ans;
    mp_init_set_int(&low, val & 0xffffffff);
    mp_init_set_int(&high, val >> 32);
    mp_mul_2d(&high, 32, &high);

    Bignum* ret = Bignum::create(state);
    ans = ret->mp_val();
    mp_or(&low, &high, ans);

    mp_clear(&low);
    mp_clear(&high);

    return ret;
  }

  Bignum* Bignum::from(STATE, long long val) {
    Bignum* ret;

    if(val < 0) {
      ret = Bignum::from(state, (unsigned long long)-val);
      ret->mp_val()->sign = MP_NEG;
    } else {
      ret = Bignum::from(state, (unsigned long long)val);
    }

    return ret;
  }

  Bignum* Bignum::create(STATE, Fixnum* val) {
    return Bignum::from(state, val->to_native());
  }

  native_int Bignum::to_native() {
    return (mp_val()->sign == MP_NEG) ? -mp_get_long(mp_val()) : mp_get_long(mp_val());
  }

  int Bignum::to_int() {
    return (mp_val()->sign == MP_NEG) ? -mp_get_int(mp_val()) : mp_get_int(mp_val());
  }

  unsigned int Bignum::to_uint() {
    return mp_get_int(mp_val());
  }

  long Bignum::to_long() {
    return (mp_val()->sign == MP_NEG) ? -mp_get_long(mp_val()) : mp_get_long(mp_val());
  }

  unsigned long Bignum::to_ulong() {
    return mp_get_long(mp_val());
  }

  long long Bignum::to_long_long() {
    mp_int *s = mp_val();
    return (s->sign == MP_NEG) ? -to_ulong_long() : to_ulong_long();
  }

  unsigned long long Bignum::to_ulong_long() {
    mp_int t;
    mp_int *s = mp_val();
    unsigned long long out, tmp;

    /* mp_get_int() gets only the lower 32 bits, on any platform. */
    out = mp_get_int(s);

    mp_init(&t);
    mp_div_2d(s, 32, &t, NULL);

    tmp = mp_get_int(&t);
    out |= tmp << 32;

    mp_clear(&t);
    return out;
  }

  Integer* Bignum::normalize(STATE, Bignum* b) {
    mp_clamp(b->mp_val());

    if((size_t)mp_count_bits(b->mp_val()) <= FIXNUM_WIDTH) {
      native_int val;
      val = (native_int)b->to_native();
      return Fixnum::from(val);
    }
    return b;
  }

  Integer* Bignum::add(STATE, Fixnum* b) {
    NMP;
    native_int bi = b->to_native();
    if(bi > 0) {
      mp_add_d(mp_val(), bi, n);
    } else {
      mp_sub_d(mp_val(), -bi, n);
    }
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::add(STATE, Bignum* b) {
    NMP;
    mp_add(mp_val(), b->mp_val(), n);
    return Bignum::normalize(state, n_obj);
  }

  Float* Bignum::add(STATE, Float* b) {
    return b->add(state, this);
  }

  Integer* Bignum::sub(STATE, Fixnum* b) {
    NMP;
    native_int bi = b->to_native();
    if(bi > 0) {
      mp_sub_d(mp_val(), bi, n);
    } else {
      mp_add_d(mp_val(), -bi, n);
    }
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::sub(STATE, Bignum* b) {
    NMP;
    mp_sub(mp_val(), b->mp_val(), n);
    return Bignum::normalize(state, n_obj);
  }

  Float* Bignum::sub(STATE, Float* b) {
    return Float::coerce(state, this)->sub(state, b);
  }

  Integer* Bignum::mul(STATE, Fixnum* b) {
    NMP;

    native_int bi = b->to_native();
    if(bi == 2) {
      mp_mul_2(mp_val(), n);
    } else {
      if(bi > 0) {
        mp_mul_d(mp_val(), bi, n);
      } else {
        mp_mul_d(mp_val(), -bi, n);
        mp_neg(n, n);
      }
    }
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::mul(STATE, Bignum* b) {
    NMP;
    mp_mul(mp_val(), b->mp_val(), n);
    return Bignum::normalize(state, n_obj);
  }

  Float* Bignum::mul(STATE, Float* b) {
    return b->mul(state, this);
  }

  Integer* Bignum::divide(STATE, Fixnum* denominator, Integer** remainder) {
    if(denominator->to_native() == 0) {
      Exception::zero_division_error(state, "divided by 0");
    }

    NMP;

    native_int bi  = denominator->to_native();
    mp_digit r;
    if(bi < 0) {
      mp_div_d(mp_val(), -bi, n, &r);
      mp_neg(n, n);
    } else {
      mp_div_d(mp_val(), bi, n, &r);
    }

    if(remainder) {
      if(mp_val()->sign == MP_NEG) {
        *remainder = Fixnum::from(-(native_int)r);
      } else {
        *remainder = Fixnum::from((native_int)r);
      }
    }

    if(r != 0 && mp_cmp_d(n, 0) == MP_LT) {
      if(remainder) {
        *remainder = Fixnum::from(as<Fixnum>(*remainder)->to_native() + bi);
      }
      mp_sub_d(n, 1, n);
    }
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::divide(STATE, Bignum* b, Integer** remainder) {
    if(mp_cmp_d(b->mp_val(), 0) == MP_EQ) {
      Exception::zero_division_error(state, "divided by 0");
    }

    NMP;
    MMP;
    mp_div(mp_val(), b->mp_val(), n, m);
    if(mp_cmp_d(n, 0) == MP_LT && mp_cmp_d(m, 0) != MP_EQ) {
      mp_sub_d(n, 1, n);
      mp_mul(b->mp_val(), n, m);
      mp_sub(mp_val(), m, m);
    }
    if(remainder) {
      *remainder = Bignum::normalize(state, m_obj);
    }
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::div(STATE, Fixnum* denominator) {
    return divide(state, denominator, NULL);
  }

  Integer* Bignum::div(STATE, Bignum* denominator) {
    return divide(state, denominator, NULL);
  }

  Float* Bignum::div(STATE, Float* other) {
    return Float::coerce(state, this)->div(state, other);
  }

  Array* Bignum::divmod(STATE, Fixnum* denominator) {
    Integer* mod = Fixnum::from(0);
    Integer* quotient = divide(state, denominator, &mod);

    Array* ary = Array::create(state, 2);
    ary->set(state, 0, quotient);
    ary->set(state, 1, mod);

    return ary;
  }

  Array* Bignum::divmod(STATE, Bignum* denominator) {
    Integer* mod = Fixnum::from(0);
    Integer* quotient = divide(state, denominator, &mod);
    Array* ary = Array::create(state, 2);
    ary->set(state, 0, quotient);
    ary->set(state, 1, mod);
    return ary;
  }

  Array* Bignum::divmod(STATE, Float* denominator) {
    return Float::coerce(state, this)->divmod(state, denominator);
  }

  Integer* Bignum::mod(STATE, Fixnum* denominator) {
    Integer* mod = Fixnum::from(0);
    divide(state, denominator, &mod);
    return mod;
  }

  Integer* Bignum::mod(STATE, Bignum* denominator) {
    Integer* mod = Fixnum::from(0);
    divide(state, denominator, &mod);
    return mod;
  }

  Float* Bignum::mod(STATE, Float* denominator) {
    return Float::coerce(state, this)->mod(state, denominator);
  }

  Integer* Bignum::bit_and(STATE, Integer* b) {
    NMP;

    if(kind_of<Fixnum>(b)) {
      b = Bignum::from(state, b->to_native());
    }

    /* Perhaps this should use mp_and rather than our own version */
    bignum_bitwise_op(BITWISE_OP_AND, mp_val(), as<Bignum>(b)->mp_val(), n);
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::bit_and(STATE, Float* b) {
    return bit_and(state, Bignum::from_double(state, b->val));
  }

  Integer* Bignum::bit_or(STATE, Integer* b) {
    NMP;

    if(kind_of<Fixnum>(b)) {
      b = Bignum::from(state, b->to_native());
    }
    /* Perhaps this should use mp_or rather than our own version */
    bignum_bitwise_op(BITWISE_OP_OR, mp_val(), as<Bignum>(b)->mp_val(), n);
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::bit_or(STATE, Float* b) {
    return bit_or(state, Bignum::from_double(state, b->val));
  }

  Integer* Bignum::bit_xor(STATE, Integer* b) {
    NMP;

    if(kind_of<Fixnum>(b)) {
      b = Bignum::from(state, b->to_native());
    }
    /* Perhaps this should use mp_xor rather than our own version */
    bignum_bitwise_op(BITWISE_OP_XOR, mp_val(), as<Bignum>(b)->mp_val(), n);
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::bit_xor(STATE, Float* b) {
    return bit_xor(state, Bignum::from_double(state, b->val));
  }

  Integer* Bignum::invert(STATE) {
    NMP;

    mp_int a; mp_init(&a);
    mp_int b; mp_init_set_int(&b, 1);

    /* inversion by -(a)-1 */
    mp_neg(mp_val(), &a);
    mp_sub(&a, &b, n);

    mp_clear(&a); mp_clear(&b);
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::neg(STATE) {
    NMP;

    mp_neg(mp_val(), n);
    return Bignum::normalize(state, n_obj);
  }

  /* These 2 don't use mp_lshd because it shifts by internal digits,
     not bits. */

  Integer* Bignum::left_shift(STATE, Fixnum* bits) {
    NMP;
    int shift = bits->to_native();
    if(shift < 0) {
      return right_shift(state, Fixnum::from(-bits->to_native()));
    }
    mp_int *a = mp_val();

    mp_mul_2d(a, shift, n);
    n->sign = a->sign;
    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::right_shift(STATE, Fixnum* bits) {
    NMP;
    int shift = bits->to_native();
    if(shift < 0) {
      return left_shift(state, Fixnum::from(-bits->to_native()));
    }

    mp_int * a = mp_val();
    if ((shift / DIGIT_BIT) >= a->used) {
      if (a->sign == MP_ZPOS)
        return Fixnum::from(0);
      else
        return Fixnum::from(-1);
    }

    if (shift == 0) {
      mp_copy(a, n);
    } else {
      mp_div_2d(a, shift, n, NULL);
      if ((a->sign == MP_NEG) && (DIGIT(a, 0) & 1)) {
        mp_sub_d(n, 1, n);
      }
    }

    return Bignum::normalize(state, n_obj);
  }

  Object* Bignum::equal(STATE, Fixnum* b) {
    native_int bi = b->to_native();
    mp_int* a = mp_val();
    if(bi < 0) {
      bi = -bi;
      mp_int n;
      mp_init(&n);
      mp_copy(a, &n);
      mp_neg(&n, &n);
      a = &n;
    }
    if(mp_cmp_d(a, bi) == MP_EQ) {
      return Qtrue;
    }
    return Qfalse;
  }

  Object* Bignum::equal(STATE, Bignum* b) {
    if(mp_cmp(mp_val(), b->mp_val()) == MP_EQ) {
      return Qtrue;
    }
    return Qfalse;
  }

  Object* Bignum::equal(STATE, Float* b) {
    return Float::coerce(state, this)->equal(state, b);
  }

  Fixnum* Bignum::compare(STATE, Fixnum* b) {
    native_int bi = b->to_native();
    mp_int* a = mp_val();
    if(bi < 0) {
      mp_int n;
      mp_init(&n);
      mp_copy(a, &n);
      mp_neg(&n, &n);

      switch(mp_cmp_d(&n, -bi)) {
        case MP_LT:
          return Fixnum::from(1);
        case MP_GT:
          return Fixnum::from(-1);
      }

    } else {
      switch(mp_cmp_d(a, bi)) {
        case MP_LT:
          return Fixnum::from(-1);
        case MP_GT:
          return Fixnum::from(1);
      }
    }
    return Fixnum::from(0);
  }

  Fixnum* Bignum::compare(STATE, Bignum* b) {
    switch(mp_cmp(mp_val(), b->mp_val())) {
      case MP_LT:
        return Fixnum::from(-1);
      case MP_GT:
        return Fixnum::from(1);
    }
    return Fixnum::from(0);
  }

  Fixnum* Bignum::compare(STATE, Float* b) {
    return Float::coerce(state, this)->compare(state, b);
  }

  Object* Bignum::gt(STATE, Fixnum* b) {
    native_int bi = b->to_native();

    mp_int* a = mp_val();
    if(bi < 0) {
      mp_int n;
      mp_init(&n);
      mp_copy(a, &n);
      mp_neg(&n, &n);

      if(mp_cmp_d(&n, -bi) == MP_LT) {
        return Qtrue;
      }
      return Qfalse;
    } else {
      if(mp_cmp_d(a, bi) == MP_GT) {
        return Qtrue;
      }
      return Qfalse;
    }
  }

  Object* Bignum::gt(STATE, Bignum* b) {
    if(mp_cmp(mp_val(), b->mp_val()) == MP_GT) {
      return Qtrue;
    }
    return Qfalse;
  }

  Object* Bignum::gt(STATE, Float* b) {
    return Float::coerce(state, this)->gt(state, b);
  }

  Object* Bignum::ge(STATE, Fixnum* b) {
    native_int bi = b->to_native();

    mp_int* a = mp_val();
    if(bi < 0) {
      mp_int n;
      mp_init(&n);
      mp_copy(a, &n);
      mp_neg(&n, &n);
      int r = mp_cmp_d(&n, -bi);
      if(r == MP_EQ || r == MP_LT) {
        return Qtrue;
      }
      return Qfalse;
    } else {
      int r = mp_cmp_d(a, bi);
      if(r == MP_EQ || r == MP_GT) {
        return Qtrue;
      }
      return Qfalse;
    }
  }

  Object* Bignum::ge(STATE, Float* b) {
    return Float::coerce(state, this)->ge(state, b);
  }

  Object* Bignum::ge(STATE, Bignum* b) {
    int r = mp_cmp(mp_val(), b->mp_val());
    if(r == MP_GT || r == MP_EQ) {
      return Qtrue;
    }
    return Qfalse;
  }

  Object* Bignum::lt(STATE, Fixnum* b) {
    native_int bi = b->to_native();

    mp_int* a = mp_val();
    if(bi < 0) {
      mp_int n;
      mp_init(&n);
      mp_copy(a, &n);
      mp_neg(&n, &n);

      if(mp_cmp_d(&n, -bi) == MP_GT) {
        return Qtrue;
      }
      return Qfalse;
    } else {
      if(mp_cmp_d(a, bi) == MP_LT) {
        return Qtrue;
      }
      return Qfalse;
    }
  }

  Object* Bignum::lt(STATE, Bignum* b) {
    if(mp_cmp(mp_val(), b->mp_val()) == MP_LT) {
      return Qtrue;
    }
    return Qfalse;
  }

  Object* Bignum::lt(STATE, Float* b) {
    return Float::coerce(state, this)->lt(state, b);
  }

  Object* Bignum::le(STATE, Fixnum* b) {
    native_int bi = b->to_native();

    mp_int* a = mp_val();
    if(bi < 0) {
      mp_int n;
      mp_init(&n);
      mp_copy(a, &n);
      mp_neg(&n, &n);
      int r = mp_cmp_d(&n, -bi);
      if(r == MP_EQ || r == MP_GT) {
        return Qtrue;
      }
      return Qfalse;
    } else {
      int r = mp_cmp_d(a, bi);
      if(r == MP_EQ || r == MP_LT) {
        return Qtrue;
      }
      return Qfalse;
    }
  }

  Object* Bignum::le(STATE, Bignum* b) {
    int r = mp_cmp(mp_val(), b->mp_val());
    if(r == MP_LT || r == MP_EQ) {
      return Qtrue;
    }
    return Qfalse;
  }

  Object* Bignum::le(STATE, Float* b) {
    return Float::coerce(state, this)->le(state, b);
  }

  Float* Bignum::to_float(STATE) {
    return Float::coerce(state, this);
  }

  String* Bignum::to_s(STATE, Integer* radix) {
    char *buf;
    int sz = 1024;
    int k;
    String* obj;

    for(;;) {
      buf = ALLOC_N(char, sz);
      mp_toradix_nd(mp_val(), buf, radix->to_native(), sz, &k);
      if(k < sz - 2) {
        obj = String::create(state, buf);
        FREE(buf);
        return obj;
      }
      FREE(buf);
      sz += 1024;
    }
  }

  Integer* Bignum::from_string_detect(STATE, const char *str) {
    const char *s;
    int radix;
    int sign;
    NMP;
    s = str;
    sign = 1;
    while(isspace(*s)) { s++; }
    if(*s == '+') {
      s++;
    } else if(*s == '-') {
      sign = 0;
      s++;
    }
    radix = 10;
    if(*s == '0') {
      switch(s[1]) {
        case 'x': case 'X':
          radix = 16; s += 2;
          break;
        case 'b': case 'B':
          radix = 2; s += 2;
          break;
        case 'o': case 'O':
          radix = 8; s += 2;
          break;
        case 'd': case 'D':
          radix = 10; s += 2;
          break;
        default:
          radix = 8; s += 1;
      }
    }
    mp_read_radix(n, s, radix);

    if(!sign) {
      n->sign = MP_NEG;
    }

    return Bignum::normalize(state, n_obj);
  }

  Integer* Bignum::from_string(STATE, const char *str, size_t radix) {
    NMP;
    mp_read_radix(n, str, radix);
    return Bignum::normalize(state, n_obj);
  }

  void Bignum::into_string(STATE, size_t radix, char *buf, size_t sz) {
    int k;
    mp_toradix_nd(mp_val(), buf, radix, sz, &k);
  }

  double Bignum::to_double(STATE) {
    int i;
    double res;
    double m;
    mp_int *a;

    a = mp_val();

    if (a->used == 0) {
      return 0;
    }

    /* get number of digits of the lsb we have to read */
    i = a->used;
    m = DIGIT_RADIX;

    /* get most significant digit of result */
    res = DIGIT(a,i);

    while (--i >= 0) {
      res = (res * m) + DIGIT(a,i);
    }

    if(std::isinf(res)) {
      /* Bignum out of range */
      res = HUGE_VAL;
    }

    if(a->sign == MP_NEG) res = -res;

    return res;
  }

  Integer* Bignum::from_float(STATE, Float* f) {
    return Bignum::from_double(state, f->val);
  }

  Integer* Bignum::from_double(STATE, double d) {
    NMP;

    long i = 0;
    BDIGIT_DBL c;
    double value;

    value = (d < 0) ? -d : d;

    if(std::isinf(d)) {
      Exception::float_domain_error(state, d < 0 ? "-Infinity" : "Infinity");
    } else if(std::isnan(d)) {
      Exception::float_domain_error(state, "NaN");
    }

    while (!(value <= (LONG_MAX >> 1)) || 0 != (long)value) {
      value = value / (double)(DIGIT_RADIX);
      i++;
    }

    mp_grow(n, i);

    while (i--) {
      value *= DIGIT_RADIX;
      c = (BDIGIT_DBL) value;
      value -= c;
      DIGIT(n,i) = c;
      n->used += 1;
    }

    if (d < 0) {
      mp_neg(n, n);
    }

    return Bignum::normalize(state, n_obj);
  }

  Array* Bignum::coerce(STATE, Bignum* other) {
    Array* ary = Array::create(state, 2);

    ary->set(state, 0, other);
    ary->set(state, 1, this);

    return ary;
  }

  Array* Bignum::coerce(STATE, Fixnum* other) {
    Array* ary = Array::create(state, 2);

    if(Fixnum* fix = try_as<Fixnum>(Bignum::normalize(state, this))) {
      ary->set(state, 0, other);
      ary->set(state, 1, fix);
    } else {
      ary->set(state, 0, Bignum::create(state, other));
      ary->set(state, 1, this);
    }

    return ary;
  }

  Integer* Bignum::size(STATE)
  {
    int bits = mp_count_bits(mp_val());
    int bytes = (bits + 7) / 8;

    /* MRI returns this in words, but thats an implementation detail as far
       as I'm concerned. */
    return Fixnum::from(bytes);
  }

  hashval Bignum::hash_bignum(STATE)
  {
    mp_int *a = mp_val();

    /* Apparently, a couple bits of each a->dp[n] aren't actually used,
       (e.g. when DIGIT_BIT is 60) so this hash is actually including
       that unused memory.  This might only be a problem if calculations
       are leaving cruft in those unused bits.  However, since Bignums
       are immutable, this shouldn't happen to us. */
    return String::hash_str((unsigned char *)a->dp, a->used * sizeof(mp_digit));
  }

}
