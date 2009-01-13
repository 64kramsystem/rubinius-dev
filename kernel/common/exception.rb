# depends on: class.rb module.rb

class Exception

  attr_writer :message
  attr_accessor :context

  def initialize(message = nil)
    @message = message
    @context = nil
    @backtrace = nil
  end

  def backtrace
    if @backtrace
      if @backtrace.kind_of? Array
        return @backtrace
      end

      return @backtrace.to_mri
    end

    return nil unless @context
    awesome_backtrace.to_mri
  end

  def awesome_backtrace
    return nil unless @context
    @backtrace = Backtrace.backtrace(@context)
  end

  def set_backtrace(bt)
    @backtrace = bt
  end

  def to_s
    @message || self.class.to_s
  end

  alias_method :message, :to_s
  alias_method :to_str, :to_s

  def inspect
    "#<#{self.class.name}: #{self.to_s}>"
  end

  def self.exception(message=nil)
    self.new(message)
  end

  def exception(message=nil)
    if message
      self.class.new(message)
    else
      self
    end
  end

  def location
    [context.file.to_s, context.line]
  end
end

##
# Primitive fails from opcode "send_primitive"

class PrimitiveFailure < Exception
end

class ScriptError < Exception
end

class StandardError < Exception
end

class SignalException < Exception
end

class NoMemoryError < Exception
end

class ZeroDivisionError < StandardError
end

class ArgumentError < StandardError
  def message
    return @message if @message
    "given #{@given}, expected #{@expected}"
  end
end

class IndexError < StandardError
end

class RangeError < StandardError
end

class FloatDomainError < RangeError
end

class LocalJumpError < StandardError
end

class NameError < StandardError
  attr_reader :name
  def initialize(*args)
    super(args.shift)
    @name = args.shift
  end
end

class NoMethodError < NameError
  attr_reader :name
  attr_reader :args
  def initialize(*arguments)
    super(arguments.shift)
    @name = arguments.shift
    @args = arguments.shift
  end
end

class RuntimeError < StandardError
end

class SecurityError < StandardError
end

class SystemStackError < StandardError
end

class ThreadError < StandardError
end

class TypeError < StandardError
end

class FloatDomainError < RangeError
end

class RegexpError < StandardError
end

class LoadError < ScriptError
end

class NotImplementedError < ScriptError
end

class Interrupt < SignalException
end

class IOError < StandardError
end

class EOFError < IOError
end

class LocalJumpError < StandardError
end

class NotImplementedError < ScriptError
end

class SyntaxError < ScriptError
  attr_accessor :column
  attr_accessor :line
  attr_accessor :file
  attr_accessor :code

  def import_position(c,l, code)
    @column = c
    @line = l
    @code = code
  end

  def message
    msg = super
    msg = "#{file}:#{@line}: #{msg}" if file && @line
    msg
  end
end

class SystemCallError < StandardError

  attr_reader :errno

  def self.errno_error(message, errno)
    Ruby.primitive :exception_errno_error
    raise PrimitiveFailure, "SystemCallError.errno_error failed"
  end

  def self.new(message, errno = nil)
    if message.is_a? Integer
      errno   = message
      message = nil
    elsif message
      message = StringValue message
    end

    if errno and error = errno_error(message, errno)
      return error
    end

    msg = "unknown error"
    msg << " - #{message}" if message
    super(msg)
  end
end

##
# Base class for various exceptions raised in the VM.

class Rubinius::VMException < Exception
end

##
# Raised in the VM when an assertion fails.

class Rubinius::AssertionError < Rubinius::VMException
end

##
# Raised in the VM when attempting to read/write outside
# the bounds of an object.

class Rubinius::ObjectBoundsExceededError < Rubinius::VMException
end

##
# Raised when you try to return from a block when not allowed.  Never seen by
# ruby code.

class IllegalLongReturn < LocalJumpError
  attr_reader :return_value
end
