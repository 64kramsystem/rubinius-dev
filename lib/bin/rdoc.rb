#!/usr/bin/env ruby
#
#  RDoc: Documentation tool for source code
#        (see lib/rdoc/rdoc.rb for more information)
#
#  Copyright (c) 2003 Dave Thomas
#  Released under the same terms as Ruby
#
#  $Revision: 15033 $

require 'rdoc/rdoc'

begin
  r = RDoc::RDoc.new
  r.document ARGV
rescue Interrupt
  $stderr.puts
  $stderr.puts "Interrupted"
rescue RDoc::Error => e
  $stderr.puts e.message
  exit 1
end
