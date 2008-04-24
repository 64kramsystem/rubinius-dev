# depends on: class.rb

##
# Stores all the information about a running method.
#--
# Hey! Be careful with this! This is used by backtrace and if it doesn't work,
# you can get recursive exceptions being raised (THATS BAD, BTW).

class MethodContext

  attr_accessor :last_match

  ##
  # The Nth group of the last regexp match.

  def nth_ref(n)
    if lm = @last_match
      return lm[n]
    end

    return nil
  end

  ##
  # One of the special globals $&, $`, $' or $+.

  def back_ref(kind)
    if lm = @last_match
      res = case kind
      when :&
        lm[0]
      when :"`"
        lm.pre_match
      when :"'"
        lm.post_match
      when :+
        lm.captures.last
      end

      return res
    end

    return nil
  end

  def to_s
    "#<#{self.class}:0x#{self.object_id.to_s(16)} #{receiver}##{name} #{file}:#{line}>"
  end
  alias_method :inspect, :to_s
  # File in which associated method defined.
  def file
    return "(unknown)" unless self.method
    method.file
  end

  # See CompiledMethod#lines
  def lines
    return [] unless self.method
    method.lines
  end

  # Current line being executed by the VM.
  def line
    return 0 unless self.method
    # We subtract 1 because the ip is actually set to what it should do
    # next, not what it's currently doing.
    return self.method.line_from_ip(self.ip - 1)
  end

  # Copies context. If locals is true
  # local variable values are also
  # copied into new context.
  def copy(locals=false)
    d = self.dup
    return d unless locals

    i = 0
    lc = self.locals
    tot = lc.fields
    nl = Tuple.new(tot)
    while i < tot
      nl.put i, lc.at(i)
      i += 1
    end

    # d.put 10, nl

    return d
  end

  def location
    l = line()
    if l == 0
      "#{file}+#{ip-1}"
    else
      "#{file}:#{line}"
    end
  end

  def disable_long_return!
    # 12 => fc->flags
    # CTX_FLAG_NO_LONG_RETURN => 1
    _set_field(12, 1)
  end

  def calling_hierarchy(start=1)
    ret = []
    ctx = self
    i = 0
    until ctx.nil?
      if i >= start
        if ctx.method.name
          ret << "#{ctx.file}:#{ctx.line}:in `#{ctx.method.name}'"
        else
          ret << "#{ctx.file}:#{ctx.line}"
        end

        # In a backtrace, an eval'd context's binding shows up
        if ctx.kind_of? BlockContext
          if ctx.env.from_eval?
            home = ctx.env.home
            ret << "#{home.file}:#{home.line} in `#{home.method.name}'"
          end
        end

      end

      i += 1
      ctx = ctx.sender
    end

    return nil if start > i + 1
    ret
  end

  def describe
    if method_module.equal?(Kernel)
      str = "Kernel."
    elsif method_module.kind_of?(MetaClass)
      str = "#{receiver}."
    elsif method_module and method_module != receiver.class
      str = "#{method_module}(#{receiver.class})#"
    else
      str = "#{receiver.class}#"
    end

    if kind_of? BlockContext
      str << "#{name} {}"
    elsif name == method.name
      str << "#{name}"
    else
      str << "#{name} (#{method.name})"
    end
  end

  def const_defined?(name)
    scope = method.staticscope
    while scope
      return true if scope.module.const_defined?(name)
      scope = scope.parent
    end

    return Object.const_defined?(name)
  end

  def class_variable_get(name)
    return current_scope.class_variable_get(name)
  end

  def class_variable_set(name, val)
    if receiver.kind_of? Module
      return receiver.class_variable_set(name, val)
    end

    return current_scope.class_variable_set(name, val)
  end

  def class_variable_defined?(name)
    return current_scope.class_variable_defined?(name)
  end

  def current_scope
    if ss = method.staticscope
      return ss.module
    else
      return method_module
    end
  end

  def send_private?
    @send_private
  end

  ##
  # Look up the staticscope chain to find the one with a Script object
  # attached to it. Return that object.

  def script_object
    if ss = method.staticscope
      while ss and !ss.script
        ss = ss.parent
      end

      return ss.script if ss
    end

    return nil
  end

  ##
  # Used to implement __FILE__ properly. kernel/core/compile.rb stashes
  # the path used to load this file in the Script object located in
  # the top staticscope.

  def active_path
    if script = script_object()
      if path = script.path
        return path.dup
      end
    end

    # If for some reason that didn't work, return the compile time filename.
    method.file.to_s
  end

  ##
  # Used to set the module body toggles

  attr_accessor :method_scope

end

##
# Stores all the information about a running NativeMethod.

class NativeMethodContext
  def location
    "#{file}"
  end
end

##
# Stores all information about a running Block.
#
# Block context has no own receiver,
# static lexical scope and is unnamed
# so it uses receiver, scope and name
# of home method context, that is,
# method context that started it's execution.
class BlockContext

  def last_match
    home.last_match
  end

  def last_match=(match)
    home.last_match = match
  end

  def nth_ref(idx)
    home.nth_ref(idx)
  end

  def back_ref(idx)
    home.back_ref(idx)
  end

  # Active context (instance of MethodContext) that started
  # execution of this block context.
  def home
    env.home
  end

  # Name of home method.
  def name
    home.name
  end

  # Block context has no receiver thus uses
  # receiver from it's home method context.
  def receiver
    home.receiver
  end

  # Block context has no own module thus uses
  # module from it's home method context.
  def method_module
    home.method_module
  end

  # Static scope of home method context.
  def current_scope
    home.current_scope
  end
end

##
# Describes the environment a Block was created in.  BlockEnvironment is used
# to create a BlockContext.

class BlockEnvironment
  ivar_as_index :__ivars__ => 0, :home => 1, :initial_ip => 2, :last_ip => 3,
    :post_send => 4, :home_block => 5, :local_count => 6, :metadata_container => 7, :method => 8
  def __ivars__   ; @__ivars__   ; end
  def home        ; @home        ; end
  def initial_ip  ; @initial_ip  ; end
  def last_ip     ; @last_ip     ; end
  def post_send   ; @post_send   ; end
  def home_block  ; @home_block  ; end
  def local_count ; @local_count ; end
  def method      ; @method      ; end

  def metadata_container
    @metadata_container
  end

  def under_context(home, cmethod)
    if home.kind_of? BlockContext
      home_block = home
      home = home.home
    else
      home_block = home
    end

    @home = home
    @initial_ip = 0
    @last_ip = 0x10000000 # 2**28
    @post_send = 0
    @home_block = home_block
    @method = cmethod
    @local_count = cmethod.local_count
    return self
  end

  ##
  # Holds a Tuple of additional metadata.
  # First field of the tuple holds a boolean indicating if the context is from
  # eval
  def metadata_container=(tup)
    @metadata_container = tup
  end

  def from_eval?
    @metadata_container and @metadata_container[0]
  end

  def from_eval!
    @metadata_container = Tuple.new(1) unless @metadata_container
    @metadata_container[0] = true
  end

  ##
  # The CompiledMethod object that we were called from

  def method=(tup)
    @method = tup
  end

  ##
  #--
  # These should be safe since I'm unsure how you'd have a BlockContext
  # and have a nil CompiledMethod (something that can (and has) happened
  # with MethodContexts)

  def file
    method.file
  end

  def line
    method.line_from_ip(initial_ip)
  end

  def home=(home)
    @home = home
  end

  def scope=(tup)
    @scope = tup
  end

  def make_independent
    @home = @home.dup
  end

  def redirect_to(obj)
    env = dup
    env.make_independent
    env.home.receiver = obj
    return env
  end

  def call_on_instance(obj, *args)
    obj = redirect_to(obj)
    obj.call *args
  end

  def disable_long_return!
    @post_send = nil
  end

  def arity
    method.required
  end
end

##
# Contains stack frame objects

class Backtrace
  include Enumerable

  attr_reader :frames

  attr_accessor :first_color
  attr_accessor :kernel_color
  attr_accessor :eval_color

  def initialize
    @frames = []
    @top_context = nil
    @first_color = "\033[0;31m"
    @kernel_color = "\033[0;34m"
    @eval_color = "\033[0;33m"
  end

  def [](index)
    @frames[index]
  end

  def show(sep="\n", colorize = true)
    first = true
    color_config = Rubinius::RUBY_CONFIG["rbx.colorize_backtraces"]
    if color_config == "no" or color_config == "NO"
      colorize = false
      color = ""
      clear = ""
    else
      clear = "\033[0m"
    end

    fr2 = @frames.map do |ent|
      recv = ent[0]
      loc = ent[1]
      color = color_from_loc(loc, first) if colorize
      first = false # special handling for first line
      times = @max - recv.size
      times = 0 if times < 0
      "#{color}    #{' ' * times}#{recv} at #{loc}#{clear}"
    end
    return fr2.join(sep)
  end

  def join(sep)
    show
  end

  alias_method :to_s, :show

  def color_from_loc(loc, first)
    return @first_color if first
    if loc =~ /kernel/
      @kernel_color
    elsif loc =~ /\(eval\)/
      @eval_color
    else
      ""
    end
  end

  attr_reader :top_context

  MAX_WIDTH = 40

  def fill_from(ctx)
    @top_context = ctx

    @max = 0
    while ctx
      unless ctx.method
        ctx = ctx.sender
        next
      end

      str = ctx.describe

      if str.size > @max
        @max = str.size
      end

      @frames << [str, ctx.location]
      ctx = ctx.sender
    end
    @max = MAX_WIDTH if @max > MAX_WIDTH
  end

  def self.backtrace(ctx=nil)
    obj = new()
    unless ctx
      ctx = MethodContext.current.sender
    end
    obj.fill_from ctx
    return obj
  end

  def each
    @frames.each { |f| yield f.last }
    self
  end

  def to_mri
    return @top_context.calling_hierarchy(0)
  end
end
