module TaskSpecs
  class A
    def simple_method(i, do_yield=false)
      a = i * 10
      Task.current.debug_context_change = true if do_yield
      a
    end
 
    def call_mthd(do_yield=false)
      Task.current.debug_context_change = true if do_yield
      simple_method(5) / 3
    end

    def call_block(do_yield=false)
      ary = [1,2,3]
      Task.current.debug_context_change = true if do_yield
      yield
    end

    def mthd_return(do_yield=false)
      simple_method(2, do_yield)
    end

    def block_return(do_yield=false)
      yielded = false
      ary = [1,2,3]
      ary.each do |i|
        if do_yield and !yielded
          Task.current.debug_context_change = true
          yielded = true
        end
        i * 7
      end
    end

    def raise_exc(do_yield=false)
      ex = RuntimeError.new("Test exception")
      Task.current.debug_context_change = true if do_yield
      Rubinius.asm(ex) { |e| e.bytecode(self); raise_exc }
    end

    def call_raise_exc(do_yield=false)
      raise_exc do_yield
    end

    def yield_debugger
      Rubinius.asm { push :nil; yield_debugger }
    end
  end

  # Simple listener class to receive call-backs when yield_debugger is hit
  class Listener

    def initialize
      @debug_channel = Channel.new
      @msg_channel = Channel.new
      Rubinius::VM.debug_channel = @debug_channel
    end

    def wait_for_breakpoint(&prc)
      @thread = Thread.new do
        @msg_channel.send true
        id = Scheduler.send_in_microseconds @debug_channel, 1_000_000, nil
        thr = @debug_channel.receive
        if thr
          Scheduler.cancel(id)
          @msg_channel.send thr.task.current_context.method.name
          prc.call(thr.task) if block_given?
          thr.control_channel.send nil
        else
          Thread.main.raise "No breakpoint triggered"
        end
      end
      @msg_channel.receive
      Thread.pass until @thread.status == "sleep"
    end
    attr_reader :thread

    def get_breakpoint_method
      id = Scheduler.send_in_microseconds @debug_channel, 1_000_000, nil
      mthd = @msg_channel.receive
      Scheduler.cancel(id) if mthd
      mthd
    end
  end

end
