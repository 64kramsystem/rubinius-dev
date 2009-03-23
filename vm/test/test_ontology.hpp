#include "vm/test/test.hpp"

class TestObjects : public CxxTest::TestSuite, public VMTest {
public:

#define check_const(obj, name) TS_ASSERT_EQUALS(G(object)->get_const(state,name), G(obj))

  void setUp() {
    create();
  }

  void tearDown() {
    destroy();
  }

  void test_object() {
    TS_ASSERT_EQUALS(G(object)->class_object(state), G(klass));
    TS_ASSERT_EQUALS(G(object)->superclass(), Qnil);
    check_const(object, "Object");
  }

  void test_class() {
    Class *cls;

    cls = G(klass);
    Class* o = cls->class_object(state);
    TS_ASSERT_EQUALS(cls, o);
    TS_ASSERT_EQUALS(cls->superclass(), G(module));
    check_const(klass, "Class");
  }

  void test_metaclass_is_completely_setup() {
    Class *cls;
    MetaClass *meta;

    cls = (Class*)G(klass);
    meta = (MetaClass*)cls->klass();
    TS_ASSERT(kind_of<MetaClass>(G(object)->klass()));
    TS_ASSERT(kind_of<LookupTable>(meta->method_table()));
    TS_ASSERT(kind_of<LookupTable>(meta->constants()));
  }

  void test_module() {
    Class *mod;

    mod = (Class*)G(module);
    TS_ASSERT_EQUALS(mod->class_object(state), G(klass));
    TS_ASSERT_EQUALS(mod->superclass(), G(object));
    check_const(module, "Module");
  }

  void test_metaclass() {
    Class *meta;

    meta = G(metaclass);

    TS_ASSERT_EQUALS(meta->class_object(state), G(klass));
    TS_ASSERT_EQUALS(meta->superclass(), G(klass));
    TS_ASSERT_EQUALS(meta->instance_type(), Fixnum::from(MetaClassType));
    check_const(metaclass, "MetaClass");
  }

  void test_tuple() {
    Class *tup;

    tup = G(tuple);

    TS_ASSERT_EQUALS(tup->class_object(state), G(klass));
    TS_ASSERT_EQUALS(tup->superclass(), G(object));
    TS_ASSERT_EQUALS(tup->instance_type(), Fixnum::from(TupleType));
    check_const(tuple, "Tuple");
  }

  void test_lookuptable() {
    Class *lt;

    lt = G(lookuptable);

    TS_ASSERT_EQUALS(lt->class_object(state), G(klass));
    TS_ASSERT_EQUALS(lt->superclass(), G(object));
    TS_ASSERT_EQUALS(lt->instance_type(), Fixnum::from(LookupTableType));
    check_const(lookuptable, "LookupTable");
  }

  void test_methtbl() {
    Class *cls;

    cls = G(methtbl);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(lookuptable));

    TS_ASSERT_EQUALS((object_type)cls->instance_type()->to_native(), MethodTableType);
    check_const(methtbl, "MethodTable");
  }

  void test_symbol() {
    Class *cls;

    cls = G(symbol);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(object));
    check_const(symbol, "Symbol");
  }

  void test_array() {
    Class *cls;

    cls = G(array);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(object));
    check_const(array, "Array");
  }

  void test_bytearray() {
    Class *cls;

    cls = G(bytearray);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(object));
    TS_ASSERT_EQUALS((object_type)cls->instance_type()->to_native(), ByteArrayType);
    check_const(bytearray, "ByteArray");
  }

  void test_string() {
    Class *cls;

    cls = G(string);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(object));
    TS_ASSERT_EQUALS((object_type)cls->instance_type()->to_native(), StringType);
    check_const(string, "String");
  }

  void test_cmethod() {
    Class *cls;

    cls = G(cmethod);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(executable));
    check_const(cmethod, "CompiledMethod");
  }

  void test_dir() {
    Class *cls;

    cls = G(dir);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(object));
    TS_ASSERT_EQUALS(cls->instance_type(), Fixnum::from(DirType));
    check_const(dir, "Dir");
  }

  void test_compactlookuptable() {
    Class *cls;

    cls = G(compactlookuptable);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(tuple));
    TS_ASSERT_EQUALS(cls->instance_type(), Fixnum::from(CompactLookupTableType));
    check_const(compactlookuptable, "CompactLookupTable");
  }

  void test_time_class() {
    Class *cls;

    cls = G(time_class);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(object));
    TS_ASSERT_EQUALS(cls->instance_type(), Fixnum::from(TimeType));
    check_const(time_class, "Time");
  }

  void test_integer_class() {
    check_const(integer, "Integer");
  }

  void test_memory_pointer() {
    Class *cls;

    cls = G(memory_pointer);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(object));

    Module* ffi = as<Module>(G(object)->get_const(state, "FFI"));
    TS_ASSERT_EQUALS(cls, ffi->get_const(state, "MemoryPointer"));
  }

  void test_taskprobe() {
    Class *cls;

    cls = G(taskprobe);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(object));
    TS_ASSERT_EQUALS(G(rubinius)->get_const(state, "TaskProbe"), G(taskprobe));
  }

  void test_exception() {
    Class *cls;

    cls = G(exception);

    TS_ASSERT_EQUALS(cls->class_object(state), G(klass));
    TS_ASSERT_EQUALS(cls->superclass(), G(object));
    check_const(exception, "Exception");
  }
};
