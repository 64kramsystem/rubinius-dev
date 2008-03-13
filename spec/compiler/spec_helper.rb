require File.dirname(__FILE__) + '/../spec_helper'

require File.dirname(__FILE__) + '/../../lib/compiler/compiler'
require File.dirname(__FILE__) + '/../../lib/compiler/generator'
require File.dirname(__FILE__) + '/../../lib/compiler/bytecode'
require File.dirname(__FILE__) + '/../../lib/compiler/text'

class TestGenerator
  def initialize
    @stream = []
    @ip = 0
  end
  
  attr_reader :stream, :ip
  
  def run(node)
    node.bytecode(self)
  end

  def inspect
    [:test_generator, @stream].inspect
  end

# TODO:
#   def pretty_print(pp)
#     pp.group(1, '[:test_generator, ', ']') {
#       pp.seplist(@stream) {|v|
#         pp.pp v
#       }
#     }
#   end

#   def pretty_print_cycle(pp)
#     pp.text(empty? ? '[xxx]' : '[xxx...]')
#   end

  def add(*args)
    @stream << args
    @ip += 1
  end

  def set_line(line, file)
    @file, @line = file, line
  end
  
  def set_label(lbl)
    @stream << [:set_label, lbl]
  end
  
  attr_accessor :redo, :break, :next, :retry, :ensure_return
  attr_reader :file, :line
    
  def close
  end
  
  def advanced_since?(ip)
    ip < @ip
  end
    
  def ==(tg)
    tg.stream == @stream
  end

  opcodes = InstructionSet::OpCodes.map { |desc| desc.opcode }
  stupids = [:add_literal, :gif, :git, :pop_modifiers, :push,
             :push_literal_at, :push_modifiers, :push_unique_literal, :send,
             :send_super, :send_with_block, :send_with_register, :swap,]

  (opcodes + stupids - [:class]).each do |name|
    class_eval <<-CODE
      def #{name}(*args)
        add :#{name}, *args
      end
    CODE
  end

  class Label
    def initialize(gen)
      @generator = gen
      @ip = nil
    end
    
    def inspect
      ":label_#{@ip}"
    end
    
    attr_reader :ip
    
    def set!
      @ip = @generator.ip
      @generator.set_label self
    end
    
    def ==(lbl)
      raise "Unset label!" unless @ip
      @ip == lbl.ip
    end
  end
  
  def new_label
    lbl = Label.new(self)
    return lbl
  end
  
  class ExceptionBlock
    def initialize(start, handler)
      @start = start
      @handler = handler
    end
    
    attr_accessor :start, :fin, :handler
    
    def handle!
      @handler.set!
    end
  end
  
  def exceptions
    eb = ExceptionBlock.new(self.new_label, self.new_label)
    eb.start.set!
    yield eb
  end
  
  def lvar_at slot
    unshift_tuple
    set_local_depth 0, slot
    pop
  end
  
  def passed_block(local=0, in_block=false)
    g = self
    ok = g.new_label
    g.exceptions do |ex|
      g.push_cpath_top
      g.find_const :LongReturnException
      g.send :allocate, 0
      g.set_local local
      g.pop

      yield
      g.goto ok

      ex.handle!

      g.push_exception
      g.dup
      g.push_local local
      g.equal

      after = g.new_label
      g.gif after
      
      g.clear_exception
      leave = g.new_label
      g.dup
      g.send :is_return, 0
      
      g.gif leave
    
      unless in_block
        g.send :value, 0
        g.sret
      end

      after.set!

      g.raise_exc

      leave.set!
      g.send :value, 0
    end

    ok.set!
  end

  # Emits userland style code only.
  def add_method(name)
    self.push_literal name
    self.push :self
    self.send :__add_method__, 2
  end

  def push_self_or_class
    lbl = self.new_label
    self.push_cpath_top
    self.find_const :Module
    self.push :self
    self.send :kind_of?, 1
    self.git lbl
    self.send :class, 0
    lbl.set!
  end
end

def gen(sexp, plugins=[])
  comp = Compiler.new TestGenerator
  plugins.each { |n| comp.activate n }
  tg = TestGenerator.new
  yield tg
  node = comp.convert_sexp [:snippit, sexp]
  act = TestGenerator.new
  node.bytecode act
  act.should == tg
end

def gen_iter x
  gen x do |g|
    desc = description do |d|

      yield d

      d.pop
      d.push_modifiers
      d.new_label.set!
      d.push :nil
      d.pop_modifiers
      d.soft_return
    end

    g.push_literal desc
    g.create_block2
    g.push :self
    g.send :ary, 0, true

    g.passed_block do
      g.send_with_block :each, 0, false
    end
  end
end

def description
  desc = Compiler::MethodDescription.new TestGenerator, 0
  yield desc.generator
  return desc
end
