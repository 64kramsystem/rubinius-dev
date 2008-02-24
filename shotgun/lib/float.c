#include <string.h>
#include <math.h>

/* Begin borrowing from MRI 1.8.6 stable */
#if defined(__FreeBSD__) && __FreeBSD__ < 4
#include <floatingpoint.h>
#endif

#ifdef HAVE_FLOAT_H
#include <float.h>
#endif

#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif

#ifdef WORDS_BIGENDIAN
#define word0(x) ((unsigned int *)&x)[0]
#define word1(x) ((unsigned int *)&x)[1]
#else
#define word0(x) ((unsigned int *)&x)[1]
#define word1(x) ((unsigned int *)&x)[0]
#endif     

#include "shotgun/lib/shotgun.h"
#include "shotgun/lib/string.h"
#include "shotgun/lib/array.h"
#include "shotgun/lib/bignum.h"
#include "shotgun/lib/float.h"

int float_radix()      { return FLT_RADIX; }
int float_rounds()     { return FLT_ROUNDS; }
double float_min()     { return DBL_MIN; }
double float_max()     { return DBL_MAX; }
int float_min_exp()    { return DBL_MIN_EXP; }
int float_max_exp()    { return DBL_MAX_EXP; }
int float_min_10_exp() { return DBL_MIN_10_EXP; }
int float_max_10_exp() { return DBL_MAX_10_EXP; }
int float_dig()        { return DBL_DIG; }
int float_mant_dig()   { return DBL_MANT_DIG; }
double float_epsilon() { return DBL_EPSILON; }

OBJECT float_new(STATE, double dbl) {
  double *value;
  OBJECT o;
  o = object_memory_new_opaque(state, BASIC_CLASS(floatpoint), sizeof(double));
  value = (double *)BYTES_OF(o);
  *value = dbl;
  return o;
}

/* This functions is only used when unmarshalling. 
 * The assumptions made here are therefore safe.
 * String#to_f uses string_to_double
 */
OBJECT float_from_string(STATE, char *str) {
  char *endp;
  double d;  
  d = strtod(str, &endp);
  if (str != endp && *endp == '\0') {
    return float_new(state, d);
  }
  /* When we get here, we might have a system that doesn't conform to
     C99 (OpenBSD is at least one) that can't handle Infinity / NaN.
     We test the strings here manually and fix it if needed. */
  
  int sign = 0;
	
  if (*str == '-') {
    sign = 1;
    str++;
  } else if (*str == '+') {
    str++;
  }
  
  if (*str == 'I' || *str == 'i') {
    return float_new(state, sign ? -HUGE_VAL : HUGE_VAL);
  } 
  if (*str == 'N' || *str == 'n') {
    word0(d) = 0x7ff80000;
    word1(d) = 0;
    return float_new(state, d);
  }
  
  return Qnil;
}

void float_into_string(STATE, OBJECT self, char *buf, int sz) {
  snprintf(buf, sz, "%+.17e", FLOAT_TO_DOUBLE(self));
}

/* TODO: change float_to_i_prim name to float_to_i once
 * the stables no longer depend on the FFI implementation
 */
OBJECT float_to_i_prim(STATE, double value) {
  return bignum_from_double(state, float_truncate(value));
}

/* TODO: change float_compare_prim name to float_compare once
 * the stables no longer depend on the FFI implementation
 */
OBJECT float_compare_prim(STATE, double a, double b) {
  if(a < b) {
    return I2N(-1);
  } else if(a > b) {
    return I2N(1);
  }
  return I2N(0);
}

OBJECT float_coerce(STATE, OBJECT value) {
  if(FIXNUM_P(value)) {
    return float_new(state, (double)N2I(value));
  } else if(BIGNUM_P(value)) {
    return float_new(state, bignum_to_double(state, value));
  }
  return value;
}

/* TODO: Remove all the following methods once the stables
 * no longer depend on the FFI implementation of Float.
 */
double float_add(double a, double b) {
  return a + b;
}

double float_sub(double a, double b) {
  return a - b;
}

double float_mul(double a, double b) {
  return a * b;
}

double float_div(double a, double b) {
  return a / b;
}

double float_uminus(double a) {
  return -a;
}

int float_equal(double a, double b) {
  return a == b;
}

int float_compare(double a, double b) {
  if (a < b)
    return -1;
  else if (a > b)
    return 1;
  return 0;
}

OBJECT float_lt(double a, double b) {
  return a < b ? Qtrue : Qfalse;
}

OBJECT float_lte(double a, double b) {
  return a <= b ? Qtrue : Qfalse;
}

OBJECT float_gt(double a, double b) {
  return a > b ? Qtrue : Qfalse;
}

OBJECT float_gte(double a, double b) {
  return a >= b ? Qtrue : Qfalse;
}

int float_to_i(double value) {
  if (value > 0.0) value = floor(value);
  if (value < 0.0) value = ceil(value);
  return (int) value;
}

int float_round(double value) {
  if (value > 0.0) value = floor(value+0.5);
  if (value < 0.0) value = ceil(value-0.5);
  return (int)value;
}
