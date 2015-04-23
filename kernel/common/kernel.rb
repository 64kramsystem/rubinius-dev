module Kernel
  def __method__
    scope = Rubinius::VariableScope.of_sender

    name = scope.method.name

    return nil if scope.method.for_module_body?
    # If the name is still __block__, then it's in a script, so return nil
    return nil if name == :__block__ or name == :__script__
    name
  end
  module_function :__method__

  alias_method :send, :__send__
  alias_method :object_id, :__id__

  def `(str) #`
    str = StringValue(str) unless str.kind_of?(String)
    pid, output = Rubinius::Mirror::Process.backtick str
    Process.waitpid(pid)

    Rubinius::Type.external_string output
  end
  module_function :` # `

  def Array(obj)
    ary = Rubinius::Type.check_convert_type obj, Array, :to_ary

    return ary if ary

    if obj.respond_to? :to_a, true
      Rubinius::Type.coerce_to(obj, Array, :to_a)
    else
      [obj]
    end
  end
  module_function :Array

  def Integer(obj)
    case obj
    when Integer
      obj
    when Float
      if obj.nan? or obj.infinite?
        raise FloatDomainError, "unable to coerce #{obj} to Integer"
      else
        obj.to_int
      end
    when String
      if obj.empty?
        raise ArgumentError, "invalid value for Integer: (empty string)"
      else
        obj.to_inum(0, true)
      end
    else
      # Can't use coerce_to or try_convert because I think there is an
      # MRI bug here where it will return the value without checking
      # the return type.
      if obj.respond_to? :to_int
        if val = obj.to_int
          return val
        end
      end

      Rubinius::Type.coerce_to obj, Integer, :to_i
    end
  end
  module_function :Integer

  def String(obj)
    return obj if obj.kind_of? String

    unless obj.respond_to? :to_s
      raise TypeError, "Unable to convert to a String"
    end

    str = obj.to_s
    unless str.kind_of? String
      raise TypeError, "#to_s did not return a String"
    end

    return str
  end
  module_function :String

  ##
  # MRI uses a macro named StringValue which has essentially the same
  # semantics as obj.coerce_to(String, :to_str), but rather than using that
  # long construction everywhere, we define a private method similar to
  # String().
  #
  # Another possibility would be to change String() as follows:
  #
  #   String(obj, sym=:to_s)
  #
  # and use String(obj, :to_str) instead of StringValue(obj)

  def StringValue(obj)
    Rubinius::Type.coerce_to obj, String, :to_str
  end
  module_function :StringValue

  ##
  # MRI uses a macro named NUM2DBL which has essentially the same semantics as
  # Float(), with the difference that it raises a TypeError and not a
  # ArgumentError. It is only used in a few places (in MRI and Rubinius).
  #--
  # If we can, we should probably get rid of this.

  def FloatValue(obj)
    exception = TypeError.new 'no implicit conversion to float'

    case obj
    when String
      raise exception
    else
      begin
        Rubinius::Type.coerce_object_to_float obj
      rescue
        raise exception
      end
    end
  end
  private :FloatValue

  def Float(obj)
    case obj
    when String
      Rubinius::Type.coerce_string_to_float obj, true
    else
      Rubinius::Type.coerce_object_to_float obj
    end
  end
  module_function :Float

  def id
    Kernel.warn "Object#id IS deprecated; use Object#object_id OR ELSE."
    __id__
  end

  def initialize_copy(source)
    unless instance_of?(Rubinius::Type.object_class(source))
      raise TypeError, "initialize_copy should take same class object"
    end
  end
  private :initialize_copy

  def warn(warning)
    $stderr.write "#{warning}\n" unless $VERBOSE.nil?
    nil
  end
  module_function :warn

  def warning(message)
    $stderr.puts message if $VERBOSE
  end
  module_function :warning

  def exec(*args)
    Process.exec(*args)
  end
  module_function :exec

  def exit(code=0)
    Process.exit(code)
  end
  module_function :exit

  def exit!(code=1)
    Process.exit!(code)
  end
  module_function :exit!

  def abort(msg=nil)
    Process.abort msg
  end
  module_function :abort

  def printf(target, *args)
    case target
    when IO
      target.printf(*args)
    when String
      $stdout << Rubinius::Sprinter.get(target).call(*args)
    else
      raise TypeError, "The first arg to printf should be an IO or a String"
    end
    nil
  end
  module_function :printf

  def sprintf(str, *args)
    Rubinius::Sprinter.get(str).call(*args)
  end

  alias_method :format, :sprintf
  module_function :sprintf
  module_function :format

  def puts(*a)
    $stdout.puts(*a)
    nil
  end
  module_function :puts

  # For each object given, prints obj.inspect followed by the
  # system record separator to standard output (thus, separator
  # cannot be overridden.) Prints nothing if no objects given.
  def p(*a)
    return nil if a.empty?
    a.each { |obj| $stdout.puts obj.inspect }
    $stdout.flush

    return nil
  end
  module_function :p

  # Object oriented version of Kernel.p
  def display(port=$>)
    port.write self
  end

  def print(*args)
    args.each do |obj|
      $stdout.write obj.to_s
    end
    nil
  end
  module_function :print

  def srand(seed=undefined)
    if undefined.equal? seed
      seed = Thread.current.randomizer.generate_seed
    end

    seed = Rubinius::Type.coerce_to seed, Integer, :to_int
    Thread.current.randomizer.swap_seed seed
  end
  module_function :srand

  def block_given?
    Rubinius::VariableScope.of_sender.block != nil
  end
  module_function :block_given?

  alias_method :iterator?, :block_given?
  module_function :iterator?

  def caller(start=1, exclude_kernel=true)
    # The + 1 is to skip this frame
    Rubinius.mri_backtrace(start + 1).map do |tup|
      code = tup[0]
      line = tup[1]
      is_block = tup[2]
      name = tup[3]

      "#{code.active_path}:#{line}:in `#{name}'"
    end
  end
  module_function :caller

  def chomp(string=$/)
    raise TypeError, "$_ must be a String" unless $_.kind_of? String
    $_ = $_.chomp(string)
  end
  module_function :chomp

  def chomp!(string=$/)
    raise TypeError, "$_ must be a String" unless $_.kind_of? String
    $_.chomp!(string)
  end
  module_function :chomp!

  def chop
    raise TypeError, "$_ must be a String" unless $_.kind_of? String
    $_ = $_.chop
  end
  module_function :chop

  def chop!
    raise TypeError, "$_ must be a String" unless $_.kind_of? String
    $_.chop!
  end
  module_function :chop!

  def global_variables
    Rubinius::Type.convert_to_names Rubinius::Globals.variables
  end
  module_function :global_variables

  #
  # Sleeps the current thread for +duration+ seconds.
  #
  def sleep(duration=undefined)
    Rubinius.primitive :vm_sleep

    # The primitive will fail on arg count if sleep is called
    # without an argument, so we call it again passing undefined
    # to mean "sleep forever"
    #
    if undefined.equal? duration
      return sleep(undefined)
    end

    if duration.kind_of? Numeric
      float = Rubinius::Type.coerce_to duration, Float, :to_f
      return sleep(float)
    else
      raise TypeError, 'time interval must be a numeric value'
    end
  end
  module_function :sleep

  def at_exit(prc=nil, &block)
    if prc
      unless prc.respond_to?(:call)
        raise "Argument must respond to #call"
      end
    else
      prc = block
    end

    unless prc
      raise "must pass a #call'able or block"
    end

    Rubinius::AtExit.unshift(prc)
  end
  module_function :at_exit

  def test(cmd, file1, file2=nil)
    case cmd
    when ?d
      File.directory? file1
    when ?e
      File.exist? file1
    when ?f
      File.file? file1
    when ?l
      File.symlink? file1
    else
      raise NotImplementedError, "command ?#{cmd.chr} not implemented"
    end
  end
  module_function :test

  def trap(sig, prc=nil, &block)
    Signal.trap(sig, prc, &block)
  end
  module_function :trap

  def tap
    yield self
    self
  end

  # The "sorta" operator, also known as the case equality operator.
  # Generally while #eql? and #== are stricter, #=== is often used
  # to denote an acceptable match or inclusion. It returns true if
  # the match is considered to be valid and false otherwise. It has
  # one special purpose: it is the operator used by the case expression.
  # So in this expression:
  #
  #   case obj
  #   when /Foo/
  #     ...
  #   when "Hi"
  #     ...
  #   end
  #
  # What really happens is that `/Foo/ === obj` is attempted and so
  # on down until a match is found or the expression ends. The use
  # by Regexp is very illustrative: while obj may satisfy the pattern,
  # it may not be the only option.
  #
  # The default #=== operator checks if the other object is #equal?
  # to this one (i.e., is the same object) or if #== returns true.
  # If neither is true, false is returned instead. Many classes opt
  # to override this behaviour to take advantage of its use in a
  # case expression and to implement more relaxed matching semantics.
  # Notably, the above Regexp as well as String, Module and many others.
  def ===(other)
    equal?(other) || self == other
  end

  ##
  # Regexp matching fails by default but may be overridden by subclasses,
  # notably Regexp and String.

  def =~(other)
    false
  end

  def extend(*modules)
    raise ArgumentError, "wrong number of arguments (0 for 1+)" if modules.empty?
    Rubinius.check_frozen

    modules.reverse_each do |mod|
      Rubinius.privately do
        mod.extend_object self
      end

      Rubinius.privately do
        mod.extended self
      end
    end
    self
  end

  alias_method :__extend__, :extend

  def inspect
    prefix = "#<#{self.class}:0x#{self.__id__.to_s(16)}"

    # The protocol here seems odd, but it's to match MRI.
    #
    # MRI side-calls to the C function that implements Kernel#to_s. If that
    # method is overridden, the new Ruby method is never called. So, we inline
    # the code for Kernel#to_s here because we simply dispatch to Ruby
    # methods.
    ivars = __instance_variables__

    if ivars.empty?
      return Rubinius::Type.infect "#{prefix}>", self
    end

    # Otherwise, if it's already been inspected, return the ...
    return "#{prefix} ...>" if Thread.guarding? self

    # Otherwise, gather the ivars and show them.
    parts = []

    Thread.recursion_guard self do
      ivars.each do |var|
        parts << "#{var}=#{__instance_variable_get__(var).inspect}"
      end
    end

    if parts.empty?
      str = "#{prefix}>"
    else
      str = "#{prefix} #{parts.join(' ')}>"
    end

    Rubinius::Type.infect(str, self)

    return str
  end

  ##
  # Returns true if this object is an instance of the given class, otherwise
  # false. Raises a TypeError if a non-Class object given.
  #
  # Module objects can also be given for MRI compatibility but the result is
  # always false.

  def instance_of?(cls)
    Rubinius.primitive :object_instance_of

    arg_class = Rubinius::Type.object_class(cls)
    if arg_class != Class and arg_class != Module
      # We can obviously compare against Modules but result is always false
      raise TypeError, "instance_of? requires a Class argument"
    end

    Rubinius::Type.object_class(self) == cls
  end

  def __all_instance_variables__
    Rubinius.primitive :object_ivar_names

    raise PrimitiveFailure, "Object#instance_variables failed"
  end
  private :__all_instance_variables__

  def instance_variable_get(sym)
    Rubinius.primitive :object_get_ivar

    sym = Rubinius::Type.ivar_validate sym
    instance_variable_get sym
  end

  alias_method :__instance_variable_get__, :instance_variable_get

  def instance_variable_set(sym, value)
    Rubinius.primitive :object_set_ivar

    sym = Rubinius::Type.ivar_validate sym
    instance_variable_set sym, value
  end

  alias_method :__instance_variable_set__, :instance_variable_set

  def remove_instance_variable(sym)
    Rubinius.primitive :object_del_ivar

    # If it's already a symbol, then we're here because it doesn't exist.
    if sym.kind_of? Symbol
      raise NameError, "instance variable '#{sym}' not defined"
    end

    # Otherwise because sym isn't a symbol, coerce it and try again.
    remove_instance_variable Rubinius::Type.ivar_validate(sym)
  end
  private :remove_instance_variable

  def instance_variables
    ary = []
    __all_instance_variables__.each do |sym|
      ary << sym if sym.is_ivar?
    end

    Rubinius::Type.convert_to_names ary
  end

  alias_method :__instance_variables__, :instance_variables

  def instance_variable_defined?(name)
    Rubinius.primitive :object_ivar_defined

    instance_variable_defined? Rubinius::Type.ivar_validate(name)
  end

  # Both of these are for defined? when used inside a proxy obj that
  # may undef the regular method. The compiler generates __ calls.
  alias_method :__instance_variable_defined_p__, :instance_variable_defined?
  alias_method :__respond_to_p__, :respond_to?

  def singleton_method_added(name)
  end
  private :singleton_method_added

  def singleton_method_removed(name)
  end
  private :singleton_method_removed

  def singleton_method_undefined(name)
  end
  private :singleton_method_undefined

  alias_method :is_a?, :kind_of?

  def nil?
    false
  end

  def method(name)
    name = Rubinius::Type.coerce_to_symbol name
    code = Rubinius.find_method(self, name)

    if code
      Method.new(self, code[1], code[0], name)
    else
      raise NameError, "undefined method `#{name}' for class #{self.class}"
    end
  end

  alias_method :send, :__send__
  alias_method :object_id, :__id__

  def methods(all=true)
    methods = singleton_methods(all)

    if all
      # We have to special case these because unlike true, false, nil,
      # Type.object_singleton_class raises a TypeError.
      case self
      when Fixnum, Symbol
        methods |= Rubinius::Type.object_class(self).instance_methods(true)
      else
        methods |= Rubinius::Type.object_singleton_class(self).instance_methods(true)
      end
    end

    return methods if kind_of?(ImmediateValue)

    undefs = []
    Rubinius::Type.object_singleton_class(self).method_table.filter_entries do |entry|
      undefs << entry.name.to_s if entry.visibility == :undef
    end

    return methods - undefs
  end

  def private_methods(all=true)
    private_singleton_methods() | Rubinius::Type.object_class(self).private_instance_methods(all)
  end

  def private_singleton_methods
    sc = Rubinius::Type.object_singleton_class self
    methods = sc.method_table.private_names

    m = sc

    while m = m.direct_superclass
      unless Rubinius::Type.object_kind_of?(m, Rubinius::IncludedModule) or
             Rubinius::Type.singleton_class_object(m)
        break
      end

      methods.concat m.method_table.private_names
    end

    Rubinius::Type.convert_to_names methods
  end
  private :private_singleton_methods

  def protected_methods(all=true)
    protected_singleton_methods() | Rubinius::Type.object_class(self).protected_instance_methods(all)
  end

  def protected_singleton_methods
    m = Rubinius::Type.object_singleton_class self
    methods = m.method_table.protected_names

    while m = m.direct_superclass
      unless Rubinius::Type.object_kind_of?(m, Rubinius::IncludedModule) or
             Rubinius::Type.singleton_class_object(m)
        break
      end

      methods.concat m.method_table.protected_names
    end

    Rubinius::Type.convert_to_names methods
  end
  private :protected_singleton_methods

  def public_methods(all=true)
    public_singleton_methods | Rubinius::Type.object_class(self).public_instance_methods(all)
  end

  def public_singleton_methods
    m = Rubinius::Type.object_singleton_class self
    methods = m.method_table.public_names

    while m = m.direct_superclass
      unless Rubinius::Type.object_kind_of?(m, Rubinius::IncludedModule) or
             Rubinius::Type.singleton_class_object(m)
        break
      end

      methods.concat m.method_table.public_names
    end

    Rubinius::Type.convert_to_names methods
  end
  private :public_singleton_methods

  def singleton_methods(all=true)
    m = Rubinius::Type.object_singleton_class self
    mt = m.method_table
    methods = mt.public_names + mt.protected_names

    if all
      while m = m.direct_superclass
        unless Rubinius::Type.object_kind_of?(m, Rubinius::IncludedModule) or
               Rubinius::Type.singleton_class_object(m)
          break
        end

        mt = m.method_table
        methods.concat mt.public_names
        methods.concat mt.protected_names
      end
    end

    Rubinius::Type.convert_to_names methods.uniq
  end

  def to_a
    if self.kind_of? Array
      self
    else
      [self]
    end
  end

  def to_s
    Rubinius::Type.infect("#<#{self.class}:0x#{self.__id__.to_s(16)}>", self)
  end

  ##
  # Loads the given file as executable code and returns true. If
  # the file cannot be found, cannot be compiled or some other
  # error occurs, LoadError is raised with an explanation.
  #
  # Unlike #require, the file extension if any must be present but
  # is not restricted to .rb, .rbc or .<platform shared lib ext>.
  # Any other extensions (or no extension) are assumed to be plain
  # Ruby files. The only exceptions to this rule are:
  #
  # 1.  if given a .rb or no/any-extensioned file and there is a
  #     compiled version of the same file that is not older than
  #     the source file (based on File.mtime), the compiled one
  #     is loaded directly to avoid the compilation overhead.
  # 2.  if a .rb file is given but it does not exist, the system
  #     will try to load the corresponding .rbc instead (to allow
  #     distributing just .rbc files.)
  #
  # If the path given starts with ./, ../, ~/ or /, it is treated
  # as a "qualified" file and will be loaded directly (after path
  # expansion) instead of matching against $LOAD_PATH. The relative
  # paths use Dir.pwd.
  #
  # If the filename is plain (unqualified) then it is sequentially
  # prefixed with each path in $LOAD_PATH ($:) to locate the file,
  # using the first one that exists. If none of the resulting paths
  # exist, LoadError is raised. Unqualified names may contain path
  # elements so directories are valid targets and can be used with
  # $LOAD_PATH.
  #
  # A few extra options are supported. If the second parameter is
  # true, then the module is wrapped inside an anonymous module for
  # loading to avoid polluting the namespace. This is actually a
  # shorthand for passing in :wrap => true-ish in the second arg
  # which may be an option Hash.
  #
  # If :recompile in option Hash is true-ish then the file in
  # question is recompiled each time. If the source file is not
  # present when recompiling is requested, a LoadError is raised.
  #
  # TODO: Support non-UNIX paths.
  #
  def load(name, wrap=false)
    cl = Rubinius::CodeLoader.new(name)
    cl.load(wrap)

    Rubinius.run_script cl.compiled_code

    Rubinius::CodeLoader.loaded_hook.trigger!(name)

    return true
  end
  module_function :load

  def lambda(&prc)
    raise ArgumentError, "block required" unless prc
    prc.lambda_style!
    return prc
  end

  module_function :lambda
  alias_method :proc, :lambda
  module_function :proc

  def loop
    raise LocalJumpError, "no block given" unless block_given?

    begin
      while true
        yield
      end
    rescue StopIteration
    end
  end
  module_function :loop

  def open(path, *rest, &block)
    path = StringValue(path)

    if path.kind_of? String and path.prefix? '|'
      return IO.popen(path[1..-1], *rest, &block)
    end

    File.open(path, *rest, &block)
  end
  module_function :open

  def rand(limit=0)
    if limit == 0
      return Thread.current.randomizer.random_float
    end

    limit = Integer(limit).abs

    if limit == 0
      Thread.current.randomizer.random_float
    else
      Thread.current.randomizer.random_integer(limit - 1)
    end
  end
  module_function :rand

  # Attempt to load the given file, returning true if successful.
  # If the file has already been successfully loaded and exists
  # in $LOADED_FEATURES, it will not be re-evaluated and false
  # is returned instead. If the filename cannot be resolved,
  # a LoadError is raised.
  #
  # The file can have one of the following extensions:
  #
  # [.rb]                   Plain Ruby source file.
  # [.rbc]                  Compiled Ruby source file.
  # [.o, .so, .dylib, .dll] Shared library (platform-specific.)
  # [<none>]                Filename without extension.
  #
  # (.rba files should be loaded using CodeArchive.load_everything.)
  #
  # If the file does not have an extension, #require attempts to
  # match it using .rb, .rbc and .<shared extension> as extensions,
  # in that order, instead. If foo.rb does not exist but foo.rbc
  # does, the latter will be loaded even if called with foo.rb.
  #
  # If the path given starts with ./, ../, ~/ or /, it is treated
  # as a "qualified" file and will be loaded directly (after path
  # expansion) instead of matching against $LOAD_PATH. The relative
  # paths use Dir.pwd.
  #
  # If the filename is plain (unqualified) then it is sequentially
  # prefixed with each path in $LOAD_PATH ($:) to locate the file,
  # using the first one that exists. If none of the resulting paths
  # exist, LoadError is raised. Unqualified names may contain path
  # elements so directories are valid targets and can be used with
  # $LOAD_PATH.
  #
  # TODO: Support non-UNIX paths.
  #
  # TODO: See if we can safely use 1.9 rules with $LOADED_FEATURES,
  #       i.e. expand paths into it. This should be possible if it
  #       is completely transparent to the user in all normal cases.
  #
  # Each successfully loaded file is added to $LOADED_FEATURES
  # ($"), using the original unexpanded filename (with the
  # exception that the file extension is added.)
  #
  def require(name)
    Rubinius::CodeLoader.require name
  end
  module_function :require

  def autoload(name, file)
    Object.autoload(name, file)
  end
  private :autoload

  def autoload?(name)
    Object.autoload?(name)
  end
  private :autoload?

  def set_trace_func(*args)
    raise NotImplementedError
  end
  module_function :set_trace_func

  def syscall(*args)
    raise NotImplementedError
  end
  module_function :syscall

  def type
    Kernel.warn "Object#type IS fully deprecated; use Object#class OR ELSE."
    self.class
  end

  def trace_var(name, cmd = nil, &block)
    if !cmd && !block
      raise(
        ArgumentError,
        'The 2nd argument should be a Proc/String, alternatively use a block'
      )
    end

    # We have to use a custom proc since set_hook passes in both the variable
    # name and value.
    set = proc do |_, value|
      if cmd.is_a?(String)
        eval(cmd)

      # In MRI if one passes both a proc in `cmd` and a block the latter will
      # be ignored.
      elsif cmd.is_a?(Proc)
        cmd.call(value)

      elsif block
        block.call(value)
      end
    end

    Rubinius::Globals.set_hook(name, :[], set)
  end
  module_function :trace_var

  # In MRI one can specify a 2nd argument to remove a specific tracer.
  # Rubinius::Globals however only supports one hook per variable, hence the
  # 2nd dummy argument.
  def untrace_var(name, *args)
    Rubinius::Globals.remove_hook(name)
  end
  module_function :untrace_var

  def fork(&block)
    Process.fork(&block)
  end
  module_function :fork

  def getc
    $stdin.getc
  end
  module_function :getc

  def gsub(pattern, rep=nil, &block)
    target = $_
    raise TypeError, "$_ must be a String, but is #{target.inspect}" unless target.kind_of? String
    $_ = target.gsub(pattern, rep, &block)
  end
  module_function :gsub

  def gsub!(pattern, rep=nil, &block)
    target = $_
    raise TypeError, "$_ must be a String, but is #{target.inspect}" unless target.kind_of? String
    target.gsub!(pattern, rep, &block)
  end
  module_function :gsub!

  def putc(int)
    $stdout.putc(int)
  end
  module_function :putc

  def gets(sep=$/)
    ARGF.gets(sep)
  end
  module_function :gets

  def readline(sep=$/)
    ARGF.readline(sep)
  end
  module_function :readline

  def readlines(sep=$/)
    ARGF.readlines(sep)
  end
  module_function :readlines

  def scan(pattern, &block)
    target = $_
    raise TypeError, "$_ must be a String, but is #{target.inspect}" unless target.kind_of? String
    target.scan(pattern, &block)
  end
  module_function :scan

  def select(*args)
    IO.select(*args)
  end
  module_function :select

  def split(*args)
    target = $_
    raise TypeError, "$_ must be a String, but is #{target.inspect}" unless target.kind_of? String
    target.split(*args)
  end
  module_function :split

  def sub(pattern, rep=nil, &block)
    target = $_
    raise TypeError, "$_ must be a String, but is #{target.inspect}" unless target.kind_of? String
    $_ = target.sub(pattern, rep, &block)
  end
  module_function :sub

  def sub!(pattern, rep=nil, &block)
    target = $_
    raise TypeError, "$_ must be a String, but is #{target.inspect}" unless target.kind_of? String
    target.sub!(pattern, rep, &block)
  end
  module_function :sub!

  def system(command, *args)
    begin
      pid = Rubinius::Mirror::Process.spawn(command, args)
    rescue SystemCallError
      return false
    end

    Process.waitpid pid
    $?.exitstatus == 0
  end
  module_function :system

end
