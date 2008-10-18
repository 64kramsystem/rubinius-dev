#ifndef RBX_REGEXP_HPP
#define RBX_REGEXP_HPP

#include "builtin/object.hpp"
#include "type_info.hpp"

// HACK gross.
// Forward declare ONLY if we haven't already included onig.h
// We do this because onigurama seems to have regex_t be a weird
// typedef. It's easier to just not bother with trying to duplicate
// what it does and do this.
#ifndef ONIGURUMA_H
struct regex_t;
#endif

namespace rubinius {
  class String;
  class Tuple;
  class LookupTable;

  class Regexp : public Object {
  public:
    const static size_t fields = 3;
    const static object_type type = RegexpType;

  private:
    String* source_;     // slot
    LookupTable* names_; // slot
    regex_t* onig_data;

  public:
    /* accessors */

    attr_accessor(source, String);
    attr_accessor(names, LookupTable);

    /* interface */

    static void init(STATE);
    static Regexp* create(STATE);
    static char*  version(STATE);

    // Ruby.primitive :regexp_initialize
    Regexp* initialize(STATE, String* pattern, Integer* options, Object* lang);

    // Ruby.primitive :regexp_options
    Object* options(STATE);

    // Ruby.primitive :regexp_search_region
    Object* match_region(STATE, String* string, Integer* start, Integer* end, Object* forward);

    // Ruby.primitive :regexp_match_start
    Object* match_start(STATE, String* string, Integer* start);

    // Ruby.primitive :regexp_allocate
    static Regexp* allocate(STATE, Object* self);

    class Info : public TypeInfo {
    public:
      BASIC_TYPEINFO_WITH_CLEANUP(TypeInfo)
    };

  };

  class MatchData : public Object {
  public:
    const static size_t fields = 4;
    const static object_type type = MatchDataType;

  private:
    String* source_; // slot
    Regexp* regexp_; // slot
    Tuple* full_;    // slot
    Tuple* region_;  // slot

  public:
    /* accessors */

    attr_accessor(source, String);
    attr_accessor(regexp, Regexp);
    attr_accessor(full, Tuple);
    attr_accessor(region, Tuple);

    /* interface */

    class Info : public TypeInfo {
    public:
      BASIC_TYPEINFO(TypeInfo)
    };
  };

}

#endif
