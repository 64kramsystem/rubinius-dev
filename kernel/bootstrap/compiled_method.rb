class CompiledMethod < Executable

  def self.allocate
    Ruby.primitive :compiledmethod_allocate
    raise PrimitiveFailure, "CompiledMethod.allocate primitive failed"
  end

  def compile
    Ruby.primitive :compiledmethod_compile
    raise PrimitiveFailure, "CompiledMethod#compile primitive failed"
  end

  def make_machine_method
    Ruby.primitive :compiledmethod_make_machine_method
    raise PrimitiveFailure, "CompiledMethod#make_machine_method primitive failed"
  end

  def activate(recv, mod, args)
    Ruby.primitive :compiledmethod_activate
    raise PrimitiveFailure, "CompiledMethod#activate failed"
  end

  ##
  # An instance of Visibility is stored in a class's or module's
  # method table and records a method's visibility. The Visibility
  # instance contains a reference to the actual compiled method.

  class Visibility
    attr_accessor :method
    attr_accessor :visibility

    # Is this method private?
    def private?
      @visibility == :private
    end

    ##
    # Is this method protected?
    def protected?
      @visibility == :protected
    end

    ##
    # Is this method public?
    def public?
      @visibility == :public
    end
  end
end
