#ifndef RBX_BUILTIN_LOOKUPTABLE_HPP
#define RBX_BUILTIN_LOOKUPTABLE_HPP

#include "builtin/object.hpp"
#include "type_info.hpp"

namespace rubinius {

  class Tuple;
  class Array;

  class LookupTableBucket : public Object {
  public:
    const static object_type type = LookupTableBucketType;

  private:
    Object *key_;   // slot
    Object *value_; // slot
    LookupTableBucket *next_;  // slot

  public:
    attr_accessor(key, Object);
    attr_accessor(value, Object);
    attr_accessor(next, LookupTableBucket);

    static LookupTableBucket* create(STATE, Object* key, Object* value);

    Object* append(STATE, LookupTableBucket *nxt);

    class Info : public TypeInfo {
    public:
      BASIC_TYPEINFO(TypeInfo)
    };
  };

  class LookupTableAssociation : public Object {
  public:
    const static object_type type = LookupTableAssociationType;

  private:
    Object* key_;   // slot

  public: // public because accessed directly via assembly
    Object* value_; // slot

  public:
    attr_accessor(key, Object);
    attr_accessor(value, Object);

    // Ruby.primitive :lookuptableassociation_allocate
    static LookupTableAssociation* create(STATE, Object* key, Object* value);

    class Info : public TypeInfo {
    public:
      BASIC_TYPEINFO(TypeInfo)
    };
  };

  #define LOOKUPTABLE_MIN_SIZE 16
  class LookupTable : public Object {
  public:
    const static object_type type = LookupTableType;

  private:
    Tuple* values_;   // slot
    Integer* bins_;    // slot
    Integer* entries_; // slot

  public:
    /* accessors */

    attr_accessor(values, Tuple);
    attr_accessor(bins, Integer);
    attr_accessor(entries, Integer);

    /* interface */

    static LookupTable* create(STATE, size_t sz = LOOKUPTABLE_MIN_SIZE);
    void setup(STATE, size_t sz);

    // Ruby.primitive :lookuptable_allocate
    static LookupTable* allocate(STATE, Object* self);

    // Ruby.primitive :lookuptable_store
    Object* store(STATE, Object* key, Object* val);

    // Ruby.primitive :lookuptable_aref
    Object* aref(STATE, Object* key);
    /** Compatibility, this is the same as aref(). */
    Object* fetch(STATE, Object* key);

    // Ruby.primitive :lookuptable_fetch
    Object* fetch(STATE, Object* key, Object* return_on_failure);

    Object* fetch(STATE, Object* key, bool* found);

    // Ruby.primitive :lookuptable_dup
    LookupTable* dup(STATE);
    void   redistribute(STATE, size_t size);
    LookupTableBucket* find_entry(STATE, Object* key);
    Object* find(STATE, Object* key);
    // Ruby.primitive :lookuptable_delete
    Object* remove(STATE, Object* key);
    // Ruby.primitive :lookuptable_has_key
    Object* has_key(STATE, Object* key);
    static Array* collect(STATE, LookupTable* tbl, Object* (*action)(STATE, LookupTableBucket*));
    static Object* get_key(STATE, LookupTableBucket* entry);
    // Ruby.primitive :lookuptable_keys
    Array* all_keys(STATE);
    static Object* get_value(STATE, LookupTableBucket* entry);
    // Ruby.primitive :lookuptable_values
    Array* all_values(STATE);
    static Object* get_entry(STATE, LookupTableBucket* entry);
    // Ruby.primitive :lookuptable_entries
    Array* all_entries(STATE);

    class Info : public TypeInfo {
    public:
      BASIC_TYPEINFO(TypeInfo)
      virtual void show(STATE, Object* self, int level);
    };
  };
};

#endif
