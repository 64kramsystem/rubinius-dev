##
# A linked list that details the static, lexical scope the method was created
# in.
#
# TODO: document

class StaticScope

  #
  # @todo  Verify the recursion here does not cause problems. --rue
  #
  def initialize(mod, par = nil)
    @module = mod
    @parent = par
  end

  # Source code of this scope.
  attr_accessor :script

  # Module or class this lexical scope enclosed into.
  attr_reader   :module

  # Static scope object this scope enclosed into.
  attr_reader   :parent

  # Module or class representing the 'current class'. MRI manipulates
  # this outside of the lexical scope and uses it for undef and method
  # definition.
  attr_accessor :current_module

  def inspect
    "#<#{self.class.name}:0x#{self.object_id.to_s(16)} parent=#{@parent.inspect} module=#{@module}>"
  end

  def to_s
    self.inspect
  end

  # Use the same info as the current StaticScope, but set current_module to
  # +mod+. Chains off the current StaticScope.
  def using_current_as(mod)
    ss = StaticScope.new @module, self
    ss.current_module = mod
    return ss
  end

  def for_method_definition
    return @current_module if @current_module
    return @module
  end

  def __const_set__(name, value)
    @module.__const_set__(name, value)
  end

  def alias_method(name, original)
    for_method_definition.__send__ :alias_method, name, original
  end

  def __undef_method__(name)
    mod = for_method_definition()
    mod.undef_method name
  end

  def active_path
    scope = self
    while scope and !scope.script
      scope = scope.parent
    end

    if script = scope.script
      if path = script.path
        return path.dup
      end
    end

    return "__unknown__.rb"
  end

  def const_defined?(name)
    scope = self
    while scope and scope.module != Object
      return true if scope.module.const_defined?(name)
      scope = scope.parent
    end

    return Object.const_defined?(name)
  end

  def const_path_defined?(path)
    if path.prefix? "::"
      return Object.const_path_defined?(path[2..-1])
    end

    parts = path.split("::")
    top = parts.shift

    scope = self

    while scope
      mod = top.to_s !~ /self/ ? scope.module.__send__(:recursive_const_get, top, false) : scope.module
      return mod.const_path_defined?(parts.join("::")) if mod

      scope = scope.parent
    end

    return Object.const_path_defined?(parts.join("::"))
  end

  def class_variable_get(name)
    return @module.class_variable_get(name)
  end

  def class_variable_set(name, val)
    return @module.class_variable_set(name, val)
  end

  def class_variable_defined?(name)
    return @module.class_variable_defined?(name)
  end

end
