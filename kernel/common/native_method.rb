#depends on class.rb executable.rb

##
# A wrapper for a calling a function in a shared library that has been
# attached via rb_define_method().
#
# The primitive slot for a NativeMethod points to the nmethod_call primitive
# which dispatches to the underlying C function.

#
# TODO: Rework exceptions.
#
class NativeMethod < Executable

  #
  # Returns true if library loads successfully.
  #
  # TODO: Fix up to properly add method, clear cache etc.
  #
  def self.load_extension(library_path, extension_name)
    library = library_path.sub /#{Rubinius::LIBSUFFIX}$/, ''
    symbol = "Init_#{extension_name}"

    entry_point = load_entry_point library, symbol

    Rubinius.metaclass.method_table[symbol.to_sym] = entry_point
    Rubinius.send symbol.to_sym

    true
  end

  #
  # Load extension and generate a NativeMethod for its entry point.
  #
  def self.load_entry_point(library_path, name)
    Ruby.primitive :nativemethod_load_extension_entry_point
  end

  def lines
    nil
  end

  def exceptions
    nil
  end

  def literals
    nil
  end

  def line_from_ip(i)
    0
  end
end
