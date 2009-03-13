#include "vm/test/test.hpp"

#include "vm.hpp"
#include "objectmemory.hpp"
#include "gc/debug.hpp"

#include <cxxtest/TestSuite.h>

#include <map>
#include <vector>

using namespace rubinius;

class TestVM : public CxxTest::TestSuite, public VMTest {
  public:

  void setUp() {
    create();
  }

  void tearDown() {
    destroy();
  }

  void test_probe_is_nil() {
    TS_ASSERT_EQUALS(Qnil, state->probe.get());
  }

  void test_symbol_given_cstr() {
    Symbol* sym1 = state->symbol("blah");
    Symbol* sym2 = state->symbol("blah");

    TS_ASSERT_EQUALS(sym1, sym2);
  }

  void test_symbol_given_string() {
    String* str1 = String::create(state, "symbolic");
    String* str2 = String::create(state, "symbolic");

    Symbol* sym1 = state->symbol(str1);
    Symbol* sym2 = state->symbol(str2);

    TS_ASSERT_EQUALS(sym1, sym2);
  }

  void test_symbol_given_std_string() {
    std::string str1("standard");
    std::string str2("standard");

    Symbol* sym1 = state->symbol(str1);
    Symbol* sym2 = state->symbol(str2);

    TS_ASSERT_EQUALS(sym1, sym2);
  }

  void test_collection() {
    std::map<int, Object*> objs;

    int index = 0;
    Root* root = static_cast<Root*>(state->globals.roots.head());
    while(root) {
      Object* tmp = root->get();
      if(tmp->reference_p() && tmp->zone == YoungObjectZone) {
        objs[index] = tmp;
      }
      index++;

      root = static_cast<Root*>(root->next());
    }

    //std::cout << "young: " << index << " (" <<
    //  state->om->young.total_objects << ")" << std::endl;

    CallFrameLocationList frames;
    state->om->collect_young(state->globals.roots, frames);

    index = 0;
    root = static_cast<Root*>(state->globals.roots.head());
    while(root) {
      if(Object* tmp = objs[index]) {
        TS_ASSERT(root->get() != tmp);
      }
      index++;

      root = static_cast<Root*>(root->next());
    }

    HeapDebug hd(state->om);
    hd.walk(state->globals.roots);

    //std::cout << "total: " << hd.seen_objects << " (" <<
    //  state->om->young.total_objects << ")" << std::endl;
  }
};
