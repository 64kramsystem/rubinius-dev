#include "instruments/profiler.hpp"

#include "vm/object_utils.hpp"

#include "builtin/array.hpp"
#include "builtin/class.hpp"
#include "builtin/compiledmethod.hpp"
#include "builtin/integer.hpp"
#include "builtin/lookuptable.hpp"
#include "builtin/module.hpp"
#include "builtin/string.hpp"
#include "builtin/symbol.hpp"
#include "detection.hpp"
#include "arguments.hpp"
#include "dispatch.hpp"

#include "instruments/timing.hpp"

#include <time.h>

#include <iostream>
#include <vector>
#include <algorithm>

namespace rubinius {

  namespace profiler {

    Fixnum* Edge::find_key(KeyMap& keys) {
      return method_->find_key(keys);
    }

    method_id Edge::id() {
      return method_->id();
    }

    Method::~Method() {
      for(Edges::iterator i = edges_.begin();
          i != edges_.end();
          i++) {
        delete i->second;
      }
    }

    String* Method::to_s(STATE) {
      const char *module = "";
      const char *method_name = name()->c_str(state);

      if(Symbol* klass = try_as<Symbol>(container())) {
        module = klass->c_str(state);
      }

      String* name = String::create(state, module);

      switch(kind()) {
      case kNormal:
        name->append(state, "#");
        name->append(state, method_name);
        break;
      case kSingleton:
        name->append(state, ".");
        name->append(state, method_name);
        break;
      case kBlock:
        name->append(state, "#");
        name->append(state, method_name);
        name->append(state, " {}");
        break;
      }

      return name;
    }

    Edge* Method::find_edge(Method* method) {
      Edges::iterator iter = edges_.find(method->id());

      Edge* edge;
      if(likely(iter != edges_.end())) {
        edge = iter->second;
      } else {
        edge = new Edge(method);
        edges_[method->id()] = edge;
      }

      return edge;
    }

    Fixnum* Method::find_key(KeyMap& keys) {
      KeyMap::iterator iter = keys.find(id_);

      if(iter == keys.end()) {
        Fixnum* key = Fixnum::from(keys.size());
        keys[id_] = key;
        return key;
      }

      return iter->second;
    }

    Array* Method::edges(STATE, KeyMap& keys) {
      Array* edges = Array::create(state, edges_.size());

      size_t idx = 0;
      for(Edges::iterator i = edges_.begin();
          i != edges_.end();
          i++) {
        Edge* edge = i->second;

        Array* ary = Array::create(state, 2);
        edges->set(state, idx++, ary);

        ary->set(state, 0, edge->find_key(keys));
        ary->set(state, 1, Integer::from(state, edge->total()));
      }

      return edges;
    }

    void Method::merge_edges(STATE, KeyMap& keys, Array* edges) {
      for(Edges::iterator i = edges_.begin();
          i != edges_.end();
          i++) {
        Edge* edge = i->second;
        Fixnum* key = edge->find_key(keys);

        size_t i;
        for(i = 0; i < edges->size(); i++) {
          Array* ary = as<Array>(edges->get(state, i));

          if(key != as<Fixnum>(ary->get(state, 0))) continue;

          uint64_t total = as<Integer>(ary->get(state, 1))->to_native();
          total += edge->total();
          ary->set(state, 1, Integer::from(state, total));
          break;
        }

        // We did not find an existing entry to update, create a new one
        if(i == edges->size()) {
          Array* ary = Array::create(state, 2);
          edges->append(state, ary);

          ary->set(state, 0, edge->find_key(keys));
          ary->set(state, 1, Integer::from(state, edge->total()));
        }
      }
    }

    MethodEntry::MethodEntry(STATE, Dispatch& msg, Arguments& args)
      : state_(state)
      , edge_(0)
    {
      method_ = state->profiler()->enter_method(
          msg, args, reinterpret_cast<CompiledMethod*>(Qnil));
      start();
    }

    MethodEntry::MethodEntry(STATE, Dispatch& msg, Arguments& args, CompiledMethod* cm)
      : state_(state)
      , edge_(0)
    {
      method_ = state->profiler()->enter_method(msg, args, cm);
      start();
    }

    MethodEntry::MethodEntry(STATE, Symbol* name, Module* module, CompiledMethod* cm)
      :state_(state)
      , edge_(0)
    {
      method_ = state->profiler()->enter_block(name, module, cm);
      start();
    }

    void MethodEntry::start() {
      previous_ = state_->profiler()->current();
      if(previous_) {
        edge_ = previous_->find_edge(method_);
      }
      state_->profiler()->set_current(method_);
      timer_.start();
    }

    MethodEntry::~MethodEntry() {
      if(!state_->shared.profiling()) return;

      timer_.stop();
      method_->accumulate(timer_.total(), timer_.timings());
      if(edge_) edge_->accumulate(timer_.total());
      state_->profiler()->set_current(previous_);
    }

    Profiler::~Profiler() {
      for(MethodMap::iterator i = methods_.begin();
          i != methods_.end();
          i++) {
        delete i->second;
      }
    }

    Symbol* Profiler::module_name(Module* module) {
      if(IncludedModule* im = try_as<IncludedModule>(module)) {
        return im->module()->name();
      } else {
        return module->name();
      }
    }

    Method* Profiler::enter_block(Symbol* name, Module* module, CompiledMethod* cm) {
      return get_method(cm, name, module_name(module), kBlock);
    }

    Method* Profiler::enter_method(Dispatch &msg, Arguments& args, CompiledMethod* cm) {
      if(MetaClass* mc = try_as<MetaClass>(msg.module)) {
        Object* attached = mc->attached_instance();

        if(Module* mod = try_as<Module>(attached)) {
          return get_method(cm, msg.name, mod->name(), kSingleton);
        } else {
          Symbol* name = args.recv()->to_s(state_)->to_sym(state_);
          return get_method(cm, msg.name, name, kSingleton);
        }
      } else {
        return get_method(cm, msg.name, module_name(msg.module), kNormal);
      }
    }

    Method* Profiler::get_method(CompiledMethod* cm, Symbol* name,
                                 Symbol* container, Kind kind) {
      Method* method = find_method(container, name, kind);

      if(!method->file() && !cm->nil_p()) {
        method->set_position(cm->file(), cm->start_line(state_));
      }

      return method;
    }

    method_id Profiler::create_id(Symbol* container, Symbol* name, Kind kind) {
      return ((uint64_t)(((intptr_t)container & 0xffffffff)) << 32)
              | ((intptr_t)name & 0xfffffff8) | kind;
    }

    Method* Profiler::find_method(Symbol* container, Symbol* name, Kind kind) {
      method_id id = create_id(container, name, kind);

      Method* method;
      MethodMap::iterator iter = methods_.find(id);
      if(unlikely(iter == methods_.end())) {
        method = new Method(id, name, container, kind);
        methods_[method->id()] = method;
      } else {
        method = iter->second;
      }

      return method;
    }

    // internal helper method
    static void update_method(STATE, LookupTable* profile, KeyMap& keys, Method* meth) {
      LookupTable* methods = as<LookupTable>(profile->fetch(
            state, state->symbol("methods")));

      Symbol* total_sym = state->symbol("total");
      Symbol* called_sym = state->symbol("called");
      Symbol* edges_sym = state->symbol("edges");

      LookupTable* method;
      Fixnum* key = meth->find_key(keys);
      if((method = try_as<LookupTable>(methods->fetch(state, key)))) {
        uint64_t total = as<Integer>(method->fetch(state, total_sym))->to_ulong_long();
        method->store(state, total_sym,
            Integer::from(state, total + meth->total()));
        size_t called = as<Fixnum>(method->fetch(state, called_sym))->to_native();
        method->store(state, called_sym, Fixnum::from(called + meth->called()));

        meth->merge_edges(state, keys, as<Array>(method->fetch(state, edges_sym)));
      } else {
        method = LookupTable::create(state);
        methods->store(state, key, method);

        method->store(state, state->symbol("name"), meth->to_s(state));
        method->store(state, total_sym, Integer::from(state, meth->total()));
        method->store(state, called_sym, Fixnum::from(meth->called()));

        if(meth->file()) {
          const char *file;
          if(meth->file()->nil_p()) {
            file = "unknown file";
          } else {
            file = meth->file()->c_str(state);
          }

          method->store(state, state->symbol("file"), String::create(state, file));
          method->store(state, state->symbol("line"), Fixnum::from(meth->line()));
        }

        method->store(state, edges_sym, meth->edges(state, keys));
      }
    }

    void Profiler::results(LookupTable* profile, KeyMap& keys) {
      for(MethodMap::iterator i = methods_.begin();
          i != methods_.end();
          i++) {
        Method* method = i->second;

        if(method->called() == 0) continue;
        update_method(state_, profile, keys, method);
      }
    }

    ProfilerCollection::ProfilerCollection(STATE)
      : profile_(state, (LookupTable*)Qnil)
    {
      LookupTable* profile = LookupTable::create(state);
      LookupTable* methods = LookupTable::create(state);
      profile->store(state, state->symbol("methods"), methods);
      profile->store(state, state->symbol("method"),
                     String::create(state, TIMING_METHOD));

      profile_.set(profile);
    }

    ProfilerCollection::~ProfilerCollection() {
      for(ProfilerMap::iterator iter = profilers_.begin();
          iter != profilers_.end();
          iter++) {
        iter->first->remove_profiler();
        delete iter->second;
      }
    }

    void ProfilerCollection::add_profiler(VM* vm, Profiler* profiler) {
      profilers_[vm] = profiler;
    }

    void ProfilerCollection::remove_profiler(VM* vm, Profiler* profiler) {
      ProfilerMap::iterator iter = profilers_.find(vm);
      if(iter != profilers_.end()) {
        Profiler* profiler = iter->second;
        profiler->results(profile_.get(), keys_);

        iter->first->remove_profiler();

        delete iter->second;
        profilers_.erase(iter);
      }
    }

    LookupTable* ProfilerCollection::results(STATE) {
      LookupTable* profile = profile_.get();

      for(ProfilerMap::iterator iter = profilers_.begin();
          iter != profilers_.end();
          iter++) {
        iter->second->results(profile, keys_);
      }

      return profile;
    }
  }
}
