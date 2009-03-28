desc "Build extensions from lib/ext"
task :extensions => %w[
  extension:readline
  extension:digest
]

#  lib/etc.rb
#  lib/fcntl.rb
#  lib/openssl/digest.rb
#  lib/syslog.rb
#  lib/zlib.rb

#
# Ask the VM to build an extension from source.
#
def compile_extension(path, flags = "-p -I#{Dir.pwd}/vm/capi")
  cflags = Object.const_get(:FLAGS).reject {|f| f =~ /-Wno-deprecated|-Weffc\+\+/ }

  cflags.each {|flag| flags << " -C,#{flag}" }

  verbose = $verbose ? "-d" : ""

  command = "./bin/rbx compile #{verbose} #{flags} #{path}"

  if $verbose
    sh command
  else
    puts "Building extension #{path}"
    sh command, :verbose => false
  end
end

namespace :extension do

  desc "Cleans all C extension libraries and build products."
  task :clean do
    Dir["lib/ext/**/*.{o,#{$dlext}}"].each do |f|
      rm_f f, :verbose => $verbose
    end
  end

  desc "Build the readline extension"
  task :readline => %W[kernel:build lib/ext/readline/readline.#{$dlext}]

  file "lib/ext/readline/readline.#{$dlext}" => FileList[
       "lib/ext/readline/build.rb",
       "lib/ext/readline/readline.c",
       "vm/capi/ruby.h"
  ] do
    compile_extension 'lib/ext/readline'
  end

  desc "Build the Digest extensions"
  task :digest => %w[extension:digest:digest
                     extension:digest:md5
                     extension:digest:rmd160
                     extension:digest:sha1
                     extension:digest:sha2
                     extension:digest:bubblebabble]

  namespace :digest do
    def digest_task name
      desc "Build Digest's #{name} extension."
      task name => %W[kernel:build lib/ext/digest/#{name}/#{name}.#{$dlext}]
      file "lib/ext/digest/#{name}/#{name}.#{$dlext}" =>
        FileList["lib/ext/digest/#{name}/build.rb",
                 "lib/ext/digest/#{name}/{#{name},#{name}init}.c",
                 "lib/ext/digest/#{name}/#{name}.h",
                 "lib/ext/digest/defs.h",
                 "vm/capi/ruby.h"
      ] do
        compile_extension "lib/ext/digest/#{name}"
      end
    end

    desc "Build Digest extension."
    task :digest => %W[kernel:build lib/ext/digest/digest.#{$dlext}]
    file "lib/ext/digest/digest.#{$dlext}" =>
      FileList["lib/ext/digest/build.rb",
               "lib/ext/digest/digest.c",
               "lib/ext/digest/digest.h",
               "lib/ext/digest/defs.h",
               "vm/capi/ruby.h"
    ] do
      compile_extension "lib/ext/digest"
    end

    digest_task "md5"
    digest_task "rmd160"
    digest_task "sha1"
    digest_task "sha2"
    digest_task "bubblebabble"
  end


  # Undocumented, used by spec/capi/capi_helper to build the spec exts.
  namespace :specs do

    FileList["spec/capi/ext/*.c"].each do |source|
      name = File.basename source, ".c"
      library = source.sub(/\.c/, ".#{$dlext}")

      task name => library

      file library => source do
        compile_extension source
      end
    end

  end
end
