#include <ruby.h>
#include <string.h>

VALUE string_spec_rb_str_new(VALUE self) {
  const char *str = "hello";
  return rb_str_new(str, strlen(str));
}

VALUE string_spec_rb_str_new2(VALUE self) {
  return rb_str_new2("hello\0invisible");
}

VALUE string_spec_rb_str_new2_with_null(VALUE self) {
  return rb_str_new2(NULL);
}

VALUE string_spec_rb_str_dup(VALUE self, VALUE str) {
  return rb_str_dup(str);
}

VALUE string_spec_rb_str_append(VALUE self, VALUE str, VALUE str2) {
  return rb_str_append(str, str2);
}

VALUE string_spec_rb_str_cat(VALUE self, VALUE str) {
  return rb_str_cat(str, "?", 1);
}

VALUE string_spec_rb_str_cat2(VALUE self, VALUE str) {
  return rb_str_cat2(str, "?");
}

VALUE string_spec_rb_str_plus(VALUE self, VALUE str1, VALUE str2) {
  return rb_str_plus(str1, str2);
}

VALUE string_spec_rb_str_buf_cat(VALUE self, VALUE str) {
  const char *question_mark = "?";
  rb_str_buf_cat(str, question_mark, strlen(question_mark));
  return str;
}

VALUE string_spec_rb_str_cmp(VALUE self, VALUE str1, VALUE str2) {
  return INT2NUM(rb_str_cmp(str1, str2));
}

VALUE string_spec_rb_str_split(VALUE self, VALUE str) {
  return rb_str_split(str, ",");
}

VALUE string_spec_rb_str2inum(VALUE self, VALUE str, VALUE inum) {
  int num = FIX2INT(inum);
  return rb_str2inum(str, num);
}

VALUE string_spec_rb_cstr2inum(VALUE self, VALUE str, VALUE inum) {
  int num = FIX2INT(inum);
  return rb_cstr2inum(RSTRING_PTR(str), num);
}

VALUE string_spec_rb_str_substr(VALUE self, VALUE str, VALUE beg, VALUE len) {
  return rb_str_substr(str, FIX2INT(beg), FIX2INT(len));
}

VALUE string_spec_rb_str_to_str(VALUE self, VALUE arg) {
  return rb_str_to_str(arg);
}

VALUE string_spec_RSTRING_PTR_iterate(VALUE self, VALUE str) {
  int i;
  char* ptr;

  ptr = RSTRING_PTR(str);
  for(i = 0; i < RSTRING_LEN(str); i++) {
    rb_yield(INT2FIX(ptr[i]));
  }
  return Qnil;
}

VALUE string_spec_RSTRING_PTR_assign(VALUE self, VALUE str, VALUE chr) {
  int i;
  char c;
  char* ptr;

  ptr = RSTRING_PTR(str);
  c = FIX2INT(chr);

  for(i = 0; i < RSTRING_LEN(str); i++) {
    ptr[i] = c;
  }
  return Qnil;
}

VALUE string_spec_RSTRING_LEN(VALUE self, VALUE str) {
  return INT2FIX(RSTRING_LEN(str));
}

VALUE string_spec_RSTRING_len(VALUE self, VALUE str) {
  return INT2FIX(RSTRING(str)->len);
}

VALUE string_spec_RSTRING_ptr_iterate(VALUE self, VALUE str) {
  int i;
  char* ptr;

  ptr = RSTRING(str)->ptr;
  for(i = 0; i < RSTRING_LEN(str); i++) {
    rb_yield(INT2FIX(ptr[i]));
  }
  return Qnil;
}

VALUE string_spec_RSTRING_ptr_assign(VALUE self, VALUE str, VALUE chr) {
  int i;
  char c;
  char* ptr;

  ptr = RSTRING(str)->ptr;
  c = FIX2INT(chr);

  for(i = 0; i < RSTRING_LEN(str); i++) {
    ptr[i] = c;
  }
  return Qnil;
}

void Init_string_spec() {
  VALUE cls;
  cls = rb_define_class("CApiStringSpecs", rb_cObject);

  rb_define_method(cls, "rb_str_new", string_spec_rb_str_new, 0);
  rb_define_method(cls, "rb_str_new2", string_spec_rb_str_new2, 0);
  rb_define_method(cls, "rb_str_new2_with_null", string_spec_rb_str_new2_with_null, 0);
  rb_define_method(cls, "rb_str_dup", string_spec_rb_str_dup, 1);
  rb_define_method(cls, "rb_str_append", string_spec_rb_str_append, 2);
  rb_define_method(cls, "rb_str_plus", string_spec_rb_str_plus, 2);
  rb_define_method(cls, "rb_str_cat", string_spec_rb_str_cat, 1);
  rb_define_method(cls, "rb_str_cat2", string_spec_rb_str_cat2, 1);
  rb_define_method(cls, "rb_str_buf_cat", string_spec_rb_str_buf_cat, 1);
  rb_define_method(cls, "rb_str_cmp", string_spec_rb_str_cmp, 2);
  rb_define_method(cls, "rb_str_split", string_spec_rb_str_split, 1);
  rb_define_method(cls, "rb_str2inum", string_spec_rb_str2inum, 2);
  rb_define_method(cls, "rb_cstr2inum", string_spec_rb_cstr2inum, 2);
  rb_define_method(cls, "rb_str_substr", string_spec_rb_str_substr, 3);
  rb_define_method(cls, "rb_str_to_str", string_spec_rb_str_to_str, 1);
  rb_define_method(cls, "RSTRING_PTR_iterate", string_spec_RSTRING_PTR_iterate, 1);
  rb_define_method(cls, "RSTRING_PTR_assign", string_spec_RSTRING_PTR_assign, 2);
  rb_define_method(cls, "RSTRING_LEN", string_spec_RSTRING_LEN, 1);
  rb_define_method(cls, "RSTRING_ptr_iterate", string_spec_RSTRING_ptr_iterate, 1);
  rb_define_method(cls, "RSTRING_ptr_assign", string_spec_RSTRING_ptr_assign, 2);
  rb_define_method(cls, "RSTRING_len", string_spec_RSTRING_len, 1);
}
