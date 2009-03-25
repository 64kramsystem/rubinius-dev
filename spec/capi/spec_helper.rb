require File.dirname(__FILE__) + '/../spec_helper'

require 'rbconfig'

def compile_extension(path, name)
  ext       = "#{path}#{name}_spec"
  source    = "#{ext}.c"
  obj       = "#{ext}.o"
  lib       = "#{ext}.#{Config::CONFIG['DLEXT']}"
  signature = "#{ext}.sig"

  return lib if File.exists?(signature) and
                IO.read(signature).chomp == RUBY_NAME and
                File.exists?(lib) and File.mtime(lib) > File.mtime(source)

  # TODO use Tap; write a common ext build task
  if RUBY_NAME == 'rbx'
    `./bin/rbx compile -I#{Rubinius::HDR_PATH} #{source}`
  elsif RUBY_NAME == 'ruby'
    cc        = Config::CONFIG["CC"]
    hdrdir    = Config::CONFIG["archdir"]
    cflags    = Config::CONFIG["CFLAGS"]
    incflags  = "-I#{path} -I#{hdrdir}"

    `#{cc} #{incflags} #{cflags} -c #{source} -o #{obj}`

    ldshared  = Config::CONFIG["LDSHARED"]
    libpath   = "-L#{path}"
    libs      = Config::CONFIG["LIBS"]
    dldflags  = Config::CONFIG["DLDFLAGS"]

    `#{ldshared} -o #{lib} #{libpath} #{dldflags} #{libs} #{obj}`
  else
    raise "Don't know how to build C extensions with #{RUBY_NAME}"
  end

  File.open(signature, "w") { |f| f.puts RUBY_NAME }

  lib
end

def load_extension(name)
  path = File.dirname(__FILE__) + '/ext/'

  ext = compile_extension path, name
  require ext
end
