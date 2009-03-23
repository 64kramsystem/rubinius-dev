#ifndef RBX_GC_OBJECT_MARK_HPP
#define RBX_GC_OBJECT_MARK_HPP

namespace rubinius {

  class GarbageCollector;
  class Object;

  /**
   *  Implementation in gc.cpp for now..
   */
  class ObjectMark {
  public:
    GarbageCollector* gc;

    ObjectMark(GarbageCollector* gc);
    Object* call(Object*);
    void set(Object* target, Object** pos, Object* val);
    void just_set(Object* target, Object* val);
  };

}

#endif

