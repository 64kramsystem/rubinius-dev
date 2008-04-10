#ifndef RBX_OBJECTS_HPP
#define RBX_OBJECTS_HPP

#include "object.hpp"

namespace rubinius {
  class BuiltinType : public Object {

  };
}

namespace rubinius {
  class StaticScope : public BuiltinType {
    public:
    const static size_t fields = 3;
    OBJECT instance_variables;
    OBJECT module;
    OBJECT parent;
  };
};

namespace rubinius {

  class String;
  class Symbol;
  class Hash;
  class Tuple;

  class SymbolTable : public BuiltinType {
    public:
    const static size_t fields = 3;
    OBJECT instance_variables;
    Tuple* symbols;
    Hash*  strings;

    static SymbolTable* create(STATE);
    OBJECT lookup(STATE, const char* str, size_t size = 0);
    OBJECT lookup(STATE, String* str);
    OBJECT find_string(STATE, Symbol* sym);
  };
};

namespace rubinius {
  class Tuple : public BuiltinType {
    public:
    const static size_t fields = 0;

    static Tuple* create(STATE, size_t fields);
    static bool is_a(OBJECT obj) {
      return obj->obj_type == TupleType;
    }

    OBJECT put(STATE, size_t idx, OBJECT val);

  };
};

namespace rubinius {

  class LookupTable;

  #define HASH_MINSIZE 16
  class Hash : public BuiltinType {
    public:
    const static size_t fields = 7;
    OBJECT instance_variables;
    OBJECT keys;
    Tuple* values;
    OBJECT bins;
    OBJECT entries;
    OBJECT default_value;
    OBJECT default_proc;

    static Hash* create(STATE, size_t size = HASH_MINSIZE);
    void   setup(STATE, size_t size);
    Hash*  dup(STATE);
    static OBJECT entry_new(STATE, hashval hsh, OBJECT key, OBJECT data);
    static OBJECT entry_append(STATE, OBJECT top, OBJECT nxt);
    OBJECT add_entry(STATE, hashval hsh, OBJECT ent);
    void   redistribute(STATE);
    OBJECT find_entry(STATE, hashval hsh);
    OBJECT add(STATE, hashval hsh, OBJECT key, OBJECT data);
    OBJECT set(STATE, OBJECT key, OBJECT val);
    OBJECT get(STATE, hashval hsh);
    int    lookup(STATE, OBJECT key, hashval hash, OBJECT *value);
    int    lookup2(STATE, int (*compare)(STATE, OBJECT, OBJECT),
        OBJECT key, hashval hash, OBJECT *value);
    void   assign(STATE, int (*compare)(STATE, OBJECT, OBJECT),
        OBJECT key, hashval hash, OBJECT value);
    OBJECT get_undef(STATE, hashval hsh);
    OBJECT remove(STATE, hashval hsh);

    static Hash*  from_tuple(STATE, OBJECT tup);
    static Tuple* csm_new(STATE);
    static OBJECT csm_find(STATE, Tuple* csm, OBJECT key);
    static OBJECT csm_add(STATE, Tuple* csm, OBJECT key, OBJECT val);
    static Hash*  csm_into_hash(STATE, Tuple* csm);
    static LookupTable* csm_into_lookuptable(STATE, Tuple* csm);

  };
};

namespace rubinius {
  class Blockcontext : public BuiltinType {
    public:
    const static size_t fields = 0;
  };
};

namespace rubinius {
  class RegexpData : public BuiltinType {
    public:
    const static size_t fields = 0;
  };
};

namespace rubinius {
  class MatchData : public BuiltinType {
    public:
    const static size_t fields = 5;
    OBJECT instance_variables;
    OBJECT source;
    OBJECT regexp;
    OBJECT full;
    OBJECT region;
  };
};

namespace rubinius {
  class NormalObject : public BuiltinType {
    public:
    const static size_t fields = 1;
    OBJECT instance_variables;
  };
};

namespace rubinius {
  class Array : public BuiltinType {
    public:
    const static size_t fields = 4;
    const static object_type type = ArrayType;

    OBJECT total;
    Tuple* tuple;
    OBJECT start;
    OBJECT shared;

    size_t size() {
      return total->n2i();
    }

    static Array* create(STATE, size_t size);
    void   setup(STATE, size_t size);
    OBJECT get(STATE, size_t idx);
    OBJECT set(STATE, size_t idx, OBJECT val);
    OBJECT append(STATE, OBJECT val);
    bool   includes_p(STATE, OBJECT val);
  };
};

namespace rubinius {
  class Exception : public BuiltinType {
    public:
    const static size_t fields = 3;
    OBJECT instance_variables;
    OBJECT message;
    OBJECT context;
  };
};

namespace rubinius {
  class BlockEnvironment : public BuiltinType {
    public:
    const static size_t fields = 9;
    OBJECT instance_variables;
    OBJECT home;
    OBJECT initial_ip;
    OBJECT last_ip;
    OBJECT post_send;
    OBJECT home_block;
    OBJECT local_count;
    OBJECT bonus;
    OBJECT method;
  };
};

namespace rubinius {
  class IO : public BuiltinType {
    public:
    const static size_t fields = 4;
    OBJECT instance_variables;
    OBJECT descriptor;
    OBJECT buffer;
    OBJECT mode;
  };
};

namespace rubinius {
  class Regexp : public BuiltinType {
    public:
    const static size_t fields = 4;
    OBJECT instance_variables;
    OBJECT source;
    OBJECT data;
    OBJECT names;

    static void cleanup(STATE, OBJECT data);
    static void init(STATE);
    static Regexp* create(STATE, String* pattern, OBJECT options, char* err_buf = NULL);
    static char*  version(STATE);

    OBJECT options(STATE);
    OBJECT match_region(STATE, String* string, OBJECT start, OBJECT end, OBJECT forward);

  };
};

namespace rubinius {
  class Bignum : public BuiltinType {
    public:
    const static size_t fields = 0;
    const static object_type type = BignumType;

    static bool is_a(OBJECT obj) {
      return obj->obj_type == BignumType;
    }

    static void cleanup(STATE, OBJECT obj);
    static void init(STATE);
    static Bignum* create(STATE, native_int num);
    static Bignum* new_unsigned(STATE, unsigned int num);
    static OBJECT  normalize(STATE, Bignum* obj);
    static Bignum* from_ull(STATE, unsigned long long val);
    static Bignum* from_ll(STATE, long long val);
    static OBJECT from_string_detect(STATE, char* str);
    static OBJECT from_string(STATE, const char* str, size_t radix);
    static OBJECT from_double(STATE, double d);
    
    void   debug(STATE);
    OBJECT add(STATE, OBJECT b);
    OBJECT sub(STATE, OBJECT b);
    OBJECT mul(STATE, OBJECT b);
    OBJECT div(STATE, OBJECT b, OBJECT mod);
    OBJECT divmod(STATE, OBJECT b);
    OBJECT mod(STATE, OBJECT b);
    OBJECT bit_and(STATE, OBJECT b);
    OBJECT bit_or(STATE, OBJECT b);
    OBJECT bit_xor(STATE, OBJECT b);
    OBJECT invert(STATE);
    OBJECT neg(STATE);
    OBJECT left_shift(STATE, OBJECT bits);
    OBJECT right_shift(STATE, OBJECT bits);
    OBJECT equal(STATE, OBJECT b);
    OBJECT compare(STATE, OBJECT b);
    OBJECT gt(STATE, OBJECT b);
    OBJECT ge(STATE, OBJECT b);
    OBJECT lt(STATE, OBJECT b);
    OBJECT le(STATE, OBJECT b);
    bool   is_zero(STATE);
    unsigned long to_int(STATE);
    int    to_i(STATE);
    unsigned int to_ui(STATE);
    unsigned long long to_ull(STATE);
    long long to_ll(STATE);
    OBJECT to_s(STATE, OBJECT radix);
    void   into_string(STATE, size_t radix, char* buf, size_t sz);
    double to_double(STATE);
    OBJECT size(STATE);
    hashval hash_bignum(STATE);
  };
};

namespace rubinius {
  class Symbol : public BuiltinType {
    public:
    const static size_t fields = 0;

    native_int index() {
      return DATA_STRIP_TAG(this);
    }

    static OBJECT from_index(STATE, size_t index) {
      return DATA_APPLY_TAG(index, DATA_TAG_SYMBOL);
    }

    OBJECT to_str(STATE);
  };

  typedef Symbol* SYMBOL;
};

namespace rubinius {
  class String : public BuiltinType {
    public:
    const static size_t fields = 6;
    const static object_type type = StringType;

    static bool is_a(OBJECT obj) {
      return obj->reference_p() && obj->obj_type == StringType;
    }

    OBJECT num_bytes;
    OBJECT characters;
    OBJECT encoding;
    OBJECT data;
    OBJECT hash;
    OBJECT shared;

    static String* create(STATE, const char* str, size_t bytes = 0);
    static hashval hash_str(const unsigned char *bp, unsigned int sz);
    static int string_equal_p(STATE, OBJECT self, OBJECT other);
    
    size_t size(STATE) {
      return num_bytes->n2i();
    }
    
    /* Allows the String object to be cast as a char* */
    operator const char *() {
      return (const char*)(data->bytes);
    }

    /* TODO: since we're technically say it's ok to change this, we might
     * want to copy it first. */
    operator char *() {
      return (char*)(data->bytes);
    }
  
    char* byte_address() {
      return (char*)data->bytes;
    }

    hashval hash_string(STATE);
    OBJECT to_sym(STATE);
    char* byte_address(STATE);
    String* string_dup(STATE);

  };
};

namespace rubinius {
  class ByteArray : public BuiltinType {
    public:
    const static size_t fields = 0;
  };
};

namespace rubinius {
  #define LOOKUPTABLE_MIN_SIZE 16
  class LookupTable : public BuiltinType {
    public:
    const static size_t fields = 4;
    OBJECT instance_variables;
    Tuple* values;
    OBJECT bins;
    OBJECT entries;

    /* Inline methods */
    static bool is_a(OBJECT obj) {
      return obj->obj_type == LookupTableType;
    }

    /* Prototypes */
    static LookupTable* create(STATE, size_t sz = LOOKUPTABLE_MIN_SIZE);
    void setup(STATE, size_t sz);
    OBJECT store(STATE, OBJECT key, OBJECT val);
    OBJECT fetch(STATE, OBJECT key);
    LookupTable* dup(STATE);
    static OBJECT entry_new(STATE, OBJECT key, OBJECT val);
    static OBJECT entry_append(STATE, OBJECT top, OBJECT nxt);
    void   redistribute(STATE, size_t size);
    OBJECT find_entry(STATE, OBJECT key);
    OBJECT find(STATE, OBJECT key);
    OBJECT remove(STATE, OBJECT key);
    OBJECT has_key(STATE, OBJECT key);
    static Array* collect(STATE, LookupTable* tbl, OBJECT (*action)(STATE, OBJECT));
    static OBJECT get_key(STATE, OBJECT entry);
    Array* all_keys(STATE);
    static OBJECT get_value(STATE, OBJECT entry);
    Array* all_values(STATE);
    static OBJECT get_entry(STATE, OBJECT entry);
    OBJECT all_entries(STATE);
  };

  class MethodTable : public LookupTable {
    public:
    static MethodTable* create(STATE);
  };
};

namespace rubinius {
  class MethodContext : public BuiltinType {
    public:
    const static size_t fields = 0;
  };
};

namespace rubinius {

  class Executable : public BuiltinType {
    public:
    const static size_t fields = 4;
    OBJECT instance_variables;
    OBJECT primitive;
    OBJECT required;
    OBJECT serial;
  };

  class CompiledMethod : public Executable {
    public:
    const static size_t fields = 19;
    OBJECT bytecodes;
    OBJECT name;
    OBJECT file;
    OBJECT local_count;
    OBJECT literals;
    OBJECT total_args;
    OBJECT splat;
    OBJECT exceptions;
    OBJECT lines;
    OBJECT path;
    OBJECT field14;
    OBJECT bonus;
    OBJECT compiled;
    OBJECT staticscope;
    OBJECT args;
  };
};

namespace rubinius {
  class Float : public BuiltinType {
    public:
    const static size_t fields = 0;
    const static object_type type = FloatType;

    static bool is_a(OBJECT obj) {
      return obj->obj_type == FloatType;
    }
    
    double val;

    static Float* create(STATE, double val);
    double to_double(STATE) { return val; }
  };
};

#include "builtin_class.hpp"
#include "builtin_list.hpp"
#include "builtin_selector.hpp"

#endif
