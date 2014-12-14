##
# Namespace for coercion functions between various ruby objects.

module Rubinius
  module Type

    ##
    # Returns an object of given class. If given object already is one, it is
    # returned. Otherwise tries obj.meth and returns the result if it is of the
    # right kind. TypeErrors are raised if the conversion method fails or the
    # conversion result is wrong.
    #
    # Uses Rubinius::Type.object_kind_of to bypass type check overrides.
    #
    # Equivalent to MRI's rb_convert_type().

    def self.coerce_to(obj, cls, meth)
      return obj if object_kind_of?(obj, cls)
      execute_coerce_to(obj, cls, meth)
    end

    def self.execute_coerce_to(obj, cls, meth)
      begin
        ret = obj.__send__(meth)
      rescue Exception => orig
        coerce_to_failed obj, cls, meth, orig
      end

      return ret if object_kind_of?(ret, cls)

      coerce_to_type_error obj, ret, meth, cls
    end

    def self.coerce_to_failed(object, klass, method, exc=nil)
      if object_respond_to? object, :inspect
        raise TypeError,
            "Coercion error: #{object.inspect}.#{method} => #{klass} failed",
            exc
      else
        raise TypeError,
            "Coercion error: #{method} => #{klass} failed",
            exc
      end
    end

    def self.coerce_to_type_error(original, converted, method, klass)
      oc = object_class original
      cc = object_class converted
      msg = "failed to convert #{oc} to #{klass}: #{oc}\##{method} returned #{cc}"
      raise TypeError, msg
    end

    ##
    # Same as coerce_to but returns nil if conversion fails.
    # Corresponds to MRI's rb_check_convert_type()
    #
    def self.check_convert_type(obj, cls, meth)
      return obj if object_kind_of?(obj, cls)
      return nil unless object_respond_to?(obj, meth, true)
      execute_check_convert_type(obj, cls, meth)
    end

    def self.execute_check_convert_type(obj, cls, meth)
      begin
        ret = obj.__send__(meth)
      rescue Exception
        return nil
      end

      return ret if ret.nil? || object_kind_of?(ret, cls)

      msg = "Coercion error: obj.#{meth} did NOT return a #{cls} (was #{object_class(ret)})"
      raise TypeError, msg
    end

    ##
    # Uses the logic of [Array, Hash, String].try_convert.
    #
    def self.try_convert(obj, cls, meth)
      return obj if object_kind_of?(obj, cls)
      return nil unless obj.respond_to?(meth)
      execute_try_convert(obj, cls, meth)
    end

    def self.execute_try_convert(obj, cls, meth)
      ret = obj.__send__(meth)

      return ret if ret.nil? || object_kind_of?(ret, cls)

      msg = "Coercion error: obj.#{meth} did NOT return a #{cls} (was #{object_class(ret)})"
      raise TypeError, msg
    end

    def self.coerce_to_comparison(a, b)
      unless cmp = (a <=> b)
        raise ArgumentError, "comparison of #{a.inspect} with #{b.inspect} failed"
      end
      cmp
    end

    def self.coerce_to_path(obj)
      StringValue(obj)
    end

    def self.coerce_to_symbol(obj)
      if object_kind_of?(obj, Fixnum)
        raise ArgumentError, "Fixnums (#{obj}) cannot be used as symbols"
      end
      obj = obj.to_str if obj.respond_to?(:to_str)

      coerce_to(obj, Symbol, :to_sym)
    end

    def self.ivar_validate(name)
      case name
      when Symbol
        # do nothing
      when String
        name = name.to_sym
      when Fixnum
        raise ArgumentError, "#{name.inspect} is not a symbol"
      else
        name = Rubinius::Type.coerce_to(name, String, :to_str)
        name = name.to_sym
      end

      unless name.is_ivar?
        raise NameError, "`#{name}' is not allowed as an instance variable name"
      end

      name
    end

    def self.coerce_to_binding(obj)
      if obj.kind_of? Binding
        binding = obj
      elsif obj.kind_of? Proc
        binding = obj.binding
      elsif obj.respond_to? :to_binding
        binding = obj.to_binding
      else
        binding = obj
      end

      unless binding.kind_of? Binding
        raise ArgumentError, "unknown type of binding"
      end

      binding
    end

    def self.coerce_to_pid(obj)
      unless obj.kind_of? Fixnum
        raise TypeError, "wrong argument type #{obj.class} (expected Fixnum)"
      end

      obj
    end

    def self.each_ancestor(mod)
      sup = mod
      while sup
        unless singleton_class_object(sup)
          if object_kind_of?(sup, IncludedModule)
            yield sup.module
          else
            yield sup if sup == sup.origin
          end
        end
        sup = sup.direct_superclass
      end
    end

    def self.coerce_to_constant_name(name)
      name = Rubinius::Type.coerce_to_symbol(name)

      unless name.is_constant?
        raise NameError, "wrong constant name #{name}"
      end

      name
    end

    def self.coerce_to_collection_index(index)
      return index if object_kind_of? index, Fixnum

      method = :to_int
      klass = Fixnum

      begin
        idx = index.__send__ method
      rescue Exception => exc
        coerce_to_failed index, klass, method, exc
      end
      return idx if object_kind_of? idx, klass

      if object_kind_of? index, Bignum
        raise RangeError, "Array index must be a Fixnum (passed Bignum)"
      else
        coerce_to_type_error index, idx, method, klass
      end
    end

    def self.coerce_to_collection_length(length)
      return length if object_kind_of? length, Fixnum

      method = :to_int
      klass = Fixnum

      begin
        size = length.__send__ method
      rescue Exception => exc
        coerce_to_failed length, klass, method, exc
      end
      return size if object_kind_of? size, klass

      if object_kind_of? size, Bignum
        raise ArgumentError, "Array size must be a Fixnum (passed Bignum)"
      else
        coerce_to_type_error length, size, :to_int, Fixnum
      end
    end

    def self.coerce_to_regexp(pattern, quote=false)
      case pattern
      when Regexp
        return pattern
      when String
        # nothing
      else
        pattern = StringValue(pattern)
      end

      pattern = Regexp.quote(pattern) if quote
      Regexp.new(pattern)
    end

    # Taint host if source is tainted.
    def self.infect(host, source)
      Rubinius.primitive :object_infect
      raise PrimitiveFailure, "Object.infect primitive failed"
    end

    def self.check_null_safe(string)
      Rubinius.invoke_primitive(:string_check_null_safe, string)
    end

    def self.const_get(mod, name, inherit = true)
      name = coerce_to_constant_name name

      current, constant = mod, undefined

      while current
        if bucket = current.constant_table.lookup(name)
          constant = bucket.constant
          constant = constant.call(current) if constant.kind_of?(Autoload)
          return constant
        end

        unless inherit
          return mod.const_missing(name)
        end

        current = current.direct_superclass
      end

      if instance_of?(Module)
        if bucket = Object.constant_table.lookup(name)
          constant = bucket.constant
          constant = constant.call(current) if constant.kind_of?(Autoload)
          return constant
        end
      end

      mod.const_missing(name)
    end

    def self.const_exists?(mod, name, inherit = true)
      name = coerce_to_constant_name name

      current = mod

      while current
        if bucket = current.constant_table.lookup(name)
          return !!bucket.constant
        end

        return false unless inherit

        current = current.direct_superclass
      end

      if instance_of?(Module)
        if bucket = Object.constant_table.lookup(name)
          return !!bucket.constant
        end
      end

      false
    end

    def self.include_modules_from(included_module, klass)
      insert_at = klass
      changed = false
      constants_changed = false

      mod = included_module

      while mod

        # Check for a cyclic include
        if mod == klass
          raise ArgumentError, "cyclic include detected"
        end

        if mod == mod.origin
          # Try and detect check_mod in klass's heirarchy, and where.
          #
          # I (emp) tried to use Module#< here, but we need to also know
          # where in the heirarchy the module is to change the insertion point.
          # Since Module#< doesn't report that, we're going to just search directly.
          #
          superclass_seen = false
          add = true

          k = klass.direct_superclass
          while k
            if k.kind_of? Rubinius::IncludedModule
              # Oh, we found it.
              if k == mod
                # ok, if we're still within the directly included modules
                # of klass, then put future things after mod, not at the
                # beginning.
                insert_at = k unless superclass_seen
                add = false
                break
              end
            else
              superclass_seen = true
            end

            k = k.direct_superclass
          end

          if add
            if mod.kind_of? Rubinius::IncludedModule
              original_mod = mod.module
            else
              original_mod = mod
            end

            im = Rubinius::IncludedModule.new(original_mod).attach_to insert_at
            insert_at = im

            changed = true
          end

          constants_changed ||= mod.constants.any?
        end

        mod = mod.direct_superclass
      end

      if changed
        included_module.method_table.each do |meth, obj, vis|
          Rubinius::VM.reset_method_cache klass, meth
        end
      end

      if constants_changed
        Rubinius.inc_global_serial
      end
    end

    def self.object_initialize_dup(obj, copy)
      Rubinius.privately do
        copy.initialize_copy obj
      end
    end

    def self.object_initialize_clone(obj, copy)
      Rubinius.privately do
        copy.initialize_copy obj
      end
    end

    def self.object_respond_to_ary?(obj)
      object_respond_to?(obj, :to_ary)
    end

    def self.binary_string(string)
      string
    end

    def self.external_string(string)
      string
    end

    def self.encode_string(string, enc)
      string
    end

    def self.ascii_compatible_encoding(string)
    end

    def self.compatible_encoding(a, b)
    end

    def self.bindable_method?(source, destination)
    end

    def self.object_respond_to__dump?(obj)
      object_respond_to? obj, :_dump
    end

    def self.object_respond_to_marshal_dump?(obj)
      object_respond_to? obj, :marshal_dump
    end

    def self.object_respond_to_marshal_load?(obj)
      object_respond_to? obj, :marshal_load
    end
  end
end
