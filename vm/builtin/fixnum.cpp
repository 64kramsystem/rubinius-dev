
#include "prelude.hpp"
#include "builtin/exception.hpp"
#include "builtin/fixnum.hpp"
#include "builtin/float.hpp"
#include "builtin/array.hpp"
#include "builtin/string.hpp"

#include "primitives.hpp"
#include "vm/object_utils.hpp"

#include <iostream>

namespace rubinius {

  int Fixnum::to_int() const {
    return (int)STRIP_FIXNUM_TAG(this);
  }

  unsigned int Fixnum::to_uint() const {
    return (unsigned int)STRIP_FIXNUM_TAG(this);
  }

  long Fixnum::to_long() const {
    return (long)STRIP_FIXNUM_TAG(this);
  }

  unsigned long Fixnum::to_ulong() const {
    return (unsigned long)STRIP_FIXNUM_TAG(this);
  }

  long long Fixnum::to_long_long() const {
    return (long long)STRIP_FIXNUM_TAG(this);
  }

  unsigned long long Fixnum::to_ulong_long() const {
    return (unsigned long long)STRIP_FIXNUM_TAG(this);
  }

  Integer* Fixnum::add(STATE, Bignum* other) {
    return other->add(state, this);
  }

  Float* Fixnum::add(STATE, Float* other) {
    return other->add(state, this);
  }

  Integer* Fixnum::sub(STATE, Bignum* other) {
    return as<Bignum>(other->neg(state))->add(state, this);
  }

  Float* Fixnum::sub(STATE, Float* other) {
    return Float::coerce(state, this)->sub(state, other);
  }

  Integer* Fixnum::mul(STATE, Fixnum* other) {
    native_int a  = to_native();
    native_int b  = other->to_native();

    if(a == 0 || b == 0) return Fixnum::from(0);

    if(a > 0) {
      if(b > 0) {
        if(a > (FIXNUM_MAX / b)) {
          return Bignum::from(state, a)->mul(state, other);
        }
      } else {
        if (b < (FIXNUM_MIN / a)) {
          return Bignum::from(state, a)->mul(state, other);
        }
      }
    } else {
      if(b > 0){
        if(a < (FIXNUM_MIN / b)) {
          return Bignum::from(state, a)->mul(state, other);
        }
      } else {
        if(b < (FIXNUM_MAX / a)) {
          return Bignum::from(state, a)->mul(state, other);
        }
      }
    }

    return Fixnum::from(a * b);
  }

  Integer* Fixnum::mul(STATE, Bignum* other) {
    return other->mul(state, this);
  }

  Float* Fixnum::mul(STATE, Float* other) {
    return other->mul(state, this);
  }

  Integer* Fixnum::div(STATE, Fixnum* other) {
    if(other->to_native() == 0) {
      Exception::zero_division_error(state, "divided by 0");
    }
    native_int numerator = to_native();
    native_int denominator = other->to_native();
    native_int quotient = numerator / denominator;
    if(quotient < 0 && quotient * denominator != numerator) --quotient;
    return Fixnum::from(quotient);
  }

  Integer* Fixnum::div(STATE, Bignum* other) {
    return Bignum::from(state, to_native())->div(state, other);
  }

  Float* Fixnum::div(STATE, Float* other) {
    return Float::coerce(state, this)->div(state, other);
  }

  Integer* Fixnum::mod(STATE, Fixnum* other) {
    native_int numerator = to_native();
    native_int denominator = other->to_native();
    native_int quotient = div(state, other)->to_native();
    return Fixnum::from(numerator - denominator * quotient);
  }

  Integer* Fixnum::mod(STATE, Bignum* other) {
    return Bignum::from(state, to_native())->mod(state, other);
  }

  Float* Fixnum::mod(STATE, Float* other) {
    return Float::create(state, to_native())->mod(state, other);
  }

  Array* Fixnum::divmod(STATE, Fixnum* other) {
    if(other->to_native() == 0) {
      Exception::zero_division_error(state, "divided by 0");
    }
    native_int numerator = to_native();
    native_int denominator = other->to_native();
    native_int fraction = div(state, other)->to_native();
    Array* ary = Array::create(state, 2);
    ary->set(state, 0, Fixnum::from(fraction));
    ary->set(state, 1, Fixnum::from(numerator - denominator * fraction));
    return ary;
  }

  Array* Fixnum::divmod(STATE, Bignum* other) {
    return Bignum::from(state, to_native())->divmod(state, other);
  }

  Array* Fixnum::divmod(STATE, Float* other) {
    return Float::create(state, to_native())->divmod(state, other);
  }

  Integer* Fixnum::neg(STATE) {
    return Fixnum::from(-to_native());
  }

  Object* Fixnum::pow(STATE, Fixnum *exponent) {
    return Bignum::from(state, to_native())->pow(state, exponent);
  }

  Float* Fixnum::pow(STATE, Float *exponent) {
    return this->to_f(state)->fpow(state, exponent);
  }

  Object* Fixnum::equal(STATE, Fixnum* other) {
    return to_native() == other->to_native() ? Qtrue : Qfalse;
  }

  Object* Fixnum::equal(STATE, Bignum* other) {
    return other->equal(state, this);
  }

  Object* Fixnum::equal(STATE, Float* other) {
    return (double)to_native() == other->val ? Qtrue : Qfalse;
  }

  Fixnum* Fixnum::compare(STATE, Fixnum* other) {
    native_int left  = to_native();
    native_int right = other->to_native();
    if(left == right) {
      return Fixnum::from(0);
    } else if(left < right) {
      return Fixnum::from(-1);
    } else {
      return Fixnum::from(1);
    }
  }

  Fixnum* Fixnum::compare(STATE, Bignum* other) {
    native_int res = other->compare(state, this)->to_native();
    if(res == 0) {
      return Fixnum::from(0);
    } else if(res < 0) {
      return Fixnum::from(1);
    } else {
      return Fixnum::from(-1);
    }
  }

  Fixnum* Fixnum::compare(STATE, Float* other) {
    double left  = (double)to_native();
    double right = other->val;
    if(left == right) {
      return Fixnum::from(0);
    } else if(left < right) {
      return Fixnum::from(-1);
    } else {
      return Fixnum::from(1);
    }
  }

  Object* Fixnum::gt(STATE, Bignum* other) {
    return other->lt(state, this);
  }

  Object* Fixnum::gt(STATE, Float* other) {
    return (double) to_native() > other->val ? Qtrue : Qfalse;
  }

  Object* Fixnum::ge(STATE, Fixnum* other) {
    return to_native() >= other->to_native() ? Qtrue : Qfalse;
  }

  Object* Fixnum::ge(STATE, Bignum* other) {
    return other->le(state, this);
  }

  Object* Fixnum::ge(STATE, Float* other) {
    return (double) to_native() >= other->val ? Qtrue : Qfalse;
  }

  Object* Fixnum::lt(STATE, Bignum* other) {
    return other->gt(state, this);
  }

  Object* Fixnum::lt(STATE, Float* other) {
    return (double) to_native() < other->val ? Qtrue : Qfalse;
  }

  Object* Fixnum::le(STATE, Fixnum* other) {
    return to_native() <= other->to_native() ? Qtrue : Qfalse;
  }

  Object* Fixnum::le(STATE, Bignum* other) {
    return other->ge(state, this);
  }

  Object* Fixnum::le(STATE, Float* other) {
    return (double) to_native() <= other->val ? Qtrue : Qfalse;
  }

  Integer* Fixnum::left_shift(STATE, Fixnum* bits) {
    native_int shift = bits->to_native();
    if(shift < 0) {
      return right_shift(state, Fixnum::from(-shift));
    }

    native_int self = to_native();

    if(shift > (native_int)FIXNUM_WIDTH || self >> ((native_int)FIXNUM_WIDTH - shift) > 0) {
      return Bignum::from(state, self)->left_shift(state, bits);
    }

    return Fixnum::from(self << shift);
  }

  Integer* Fixnum::right_shift(STATE, Fixnum* bits) {
    native_int shift = bits->to_native();
    if(shift < 0) {
      return left_shift(state, Fixnum::from(-shift));
    }

    return Fixnum::from(to_native() >> shift);
  }

  Integer* Fixnum::size(STATE) {
    return Fixnum::from(sizeof(native_int));
  }

  Integer* Fixnum::bit_and(STATE, Fixnum* other) {
    return Fixnum::from(to_native() & other->to_native());
  }

  Integer* Fixnum::bit_and(STATE, Bignum* other) {
    return other->bit_and(state, this);
  }

  Integer* Fixnum::bit_and(STATE, Float* other) {
    return Fixnum::from(to_native() & (native_int)other->val);
  }

  Integer* Fixnum::bit_or(STATE, Fixnum* other) {
    return Fixnum::from(to_native() | other->to_native());
  }

  Integer* Fixnum::bit_or(STATE, Bignum* other) {
    return other->bit_or(state, this);
  }

  Integer* Fixnum::bit_or(STATE, Float* other) {
    return Fixnum::from(to_native() | (native_int)other->val);
  }

  Integer* Fixnum::bit_xor(STATE, Fixnum* other) {
    return Fixnum::from(to_native() ^ other->to_native());
  }

  Integer* Fixnum::bit_xor(STATE, Bignum* other) {
    return other->bit_xor(state, this);
  }

  Integer* Fixnum::bit_xor(STATE, Float* other) {
    return Fixnum::from(to_native() ^ (native_int)other->val);
  }

  Integer* Fixnum::invert(STATE) {
    return Fixnum::from(~to_native());
  }

  Float* Fixnum::to_f(STATE) {
    return Float::create(state, (double)to_native());
  }

  String* Fixnum::to_s(STATE) {
    return to_s(state, Fixnum::from(10));
  }

  String* Fixnum::to_s(STATE, Fixnum* base) {
    // algorithm adapted from shotgun
    static const char digitmap[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char buf[100];
    char *b = buf + sizeof(buf);
    native_int j, k, m;

    j = base->to_native();
    k = to_native();

    if(j < 2 || j > 36) {
      Exception::argument_error(state, "invalid base");
    }

    /* Algorithm taken from 1.8.4 rb_fix2str */
    if(k == 0) return String::create(state, "0");

    m = 0;
    if(k < 0) {
      k = -k;
      m = 1;
    }
    *--b = 0;
    do {
      *--b = digitmap[(int)(k % j)];
    } while(k /= j);

    if(m) {
      *--b = '-';
    }

    return String::create(state, b);
  }

  Array* Fixnum::coerce(STATE, Bignum* other) {
    Array* ary = Array::create(state, 2);

    if(Fixnum* fix = try_as<Fixnum>(Bignum::normalize(state, other))) {
      ary->set(state, 0, fix);
      ary->set(state, 1, this);
    } else {
      ary->set(state, 0, other);
      ary->set(state, 1, Bignum::create(state, this));
    }

    return ary;
  }

  Array* Fixnum::coerce(STATE, Fixnum* other) {
    Array* ary = Array::create(state, 2);

    ary->set(state, 0, other);
    ary->set(state, 1, this);

    return ary;
  }

  void Fixnum::Info::show(STATE, Object* self, int level) {
    Fixnum* f = as<Fixnum>(self);
    std::cout << f->to_native() << std::endl;
  }

  void Fixnum::Info::show_simple(STATE, Object* self, int level) {
    show(state, self, level);
  }

  void Fixnum::Info::mark(Object* t, ObjectMark& mark) { }
}
