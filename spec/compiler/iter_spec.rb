require File.dirname(__FILE__) + '/../spec_helper'

describe "An Iter node" do
  empty_block = lambda do |g|
    g.passed_block do
      g.push :self

      g.in_block_send :m, :none do |d|
        d.push :nil
      end
    end
  end

  relates "m { }" do
    parse do
      [:iter, [:call, nil, :m, [:arglist]], nil]
    end

    compile(&empty_block)
  end

  relates "m do end" do
    parse do
      [:iter, [:call, nil, :m, [:arglist]], nil]
    end

    compile(&empty_block)
  end

  relates "m { x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:call, nil, :x, [:arglist]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push :self
          d.send :x, 0, true
        end
      end
    end
  end

  relates "m { || x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       0,
       [:call, nil, :x, [:arglist]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :empty do |d|
          d.push :self
          d.send :x, 0, true
        end
      end
    end
  end

  single_arg_block = lambda do |g|
    g.passed_block do
      g.push :self

      g.in_block_send :m, :single do |d|
        d.push_local_depth 0, 0
        d.push :self
        d.send :x, 0, true
        d.send :+, 1, false
      end
    end
  end

  relates "m { |a| a + x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:lasgn, :a],
       [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]]]
    end

    compile(&single_arg_block)
  end

  relates "m { |*| x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:splat]]],
       [:call, nil, :x, [:arglist]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :blank do |d|
          d.push :self
          d.send :x, 0, true
        end
      end
    end
  end

  relates "m { |*c| x; c }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:splat, [:lasgn, :c]]]],
       [:block, [:call, nil, :x, [:arglist]], [:lvar, :c]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :splat do |d|
          d.push :self
          d.send :x, 0, true
          d.pop
          d.push_local_depth 0, 0
        end
      end
    end
  end

  masgn_single_arg_block = lambda do |g|
    g.passed_block do
      g.push :self

      g.in_block_send :m, :multi, 1 do |d|
        d.push_local_depth 0, 0
        d.push :self
        d.send :x, 0, true
        d.send :+, 1, false
      end
    end
  end

  relates "m { |a, | a + x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:lasgn, :a]]],
       [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]]]
    end

    compile(&masgn_single_arg_block)
  end

  relates "m { |a, *| a + x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:lasgn, :a], [:splat]]],
       [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]]]
    end

    compile(&masgn_single_arg_block)
  end

  relates "m { |a, *c| a + x; c }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:lasgn, :a], [:splat, [:lasgn, :c]]]],
       [:block,
        [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]],
        [:lvar, :c]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :rest, -2 do |d|
          d.push_local_depth 0, 0
          d.push :self
          d.send :x, 0, true
          d.send :+, 1, false
          d.pop
          d.push_local_depth 0, 1
        end
      end
    end
  end

  relates "m { |a, b| a + x; b }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:lasgn, :a], [:lasgn, :b]]],
       [:block,
        [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]],
        [:lvar, :b]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :multi, 2 do |d|
          d.push_local_depth 0, 0
          d.push :self
          d.send :x, 0, true
          d.send :+, 1, false
          d.pop
          d.push_local_depth 0, 1
        end
      end
    end
  end

  masgn_multi_arg_block = lambda do |g|
    g.passed_block do
      g.push :self

      g.in_block_send :m, :multi, -2 do |d|
        d.push_local_depth 0, 0
        d.push :self
        d.send :x, 0, true
        d.send :+, 1, false
        d.pop
        d.push_local_depth 0, 1
      end
    end
  end

  relates "m { |a, b, | a + x; b }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:lasgn, :a], [:lasgn, :b]]],
       [:block,
        [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]],
        [:lvar, :b]]]
    end

    compile(&masgn_multi_arg_block)
  end

  relates "m { |a, b, *| a + x; b }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:lasgn, :a], [:lasgn, :b], [:splat]]],
       [:block,
        [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]],
        [:lvar, :b]]]
    end

    compile(&masgn_multi_arg_block)
  end

  masgn_rest_arg_block = lambda do |g|
    g.passed_block do
      g.push :self

      g.in_block_send :m, :rest, -3 do |d|
        d.push_local_depth 0, 0
        d.push :self
        d.send :x, 0, true
        d.send :+, 1, false
        d.pop
        d.push_local_depth 0, 1
        d.pop
        d.push_local_depth 0, 2
      end
    end
  end

  relates "m { |a, b, *c| a + x; b; c }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:lasgn, :a], [:lasgn, :b], [:splat, [:lasgn, :c]]]],
       [:block,
        [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]],
        [:lvar, :b],
        [:lvar, :c]]]
    end

    compile(&masgn_rest_arg_block)
  end

  relates "m do |a, b, *c| a + x; b; c end" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:masgn, [:array, [:lasgn, :a], [:lasgn, :b], [:splat, [:lasgn, :c]]]],
       [:block,
        [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]],
        [:lvar, :b],
        [:lvar, :c]]]
    end

    compile(&masgn_rest_arg_block)
  end

  relates "m { n = 1; n }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:block, [:lasgn, :n, [:lit, 1]], [:lvar, :n]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push 1
          d.set_local_depth 0, 0
          d.pop
          d.push_local_depth 0, 0
        end
      end
    end
  end

  relates "m { n = 1; m { n } }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:block,
        [:lasgn, :n, [:lit, 1]],
        [:iter, [:call, nil, :m, [:arglist]], nil, [:lvar, :n]]]]
    end

    compile do |g|
      iter = description do |d|
        d.pop
        d.push_modifiers
        d.new_label.set! # redo
        d.push 1
        d.set_local_depth 0, 0
        d.pop

        i2 = description do |j|
          j.pop
          j.push_modifiers
          j.new_label.set! # redo
          j.push_local_depth 1, 0
          j.pop_modifiers
          j.ret
        end

        d.break_rescue do
          d.push :self
          d.create_block i2
          d.send_with_block :m, 0, true
        end

        d.pop_modifiers
        d.ret
      end

      g.passed_block do
        g.push :self
        g.create_block iter
        g.send_with_block :m, 0, true
      end
    end
  end

  relates "n = 1; m { n = 2 }; n" do
    parse do
      [:block,
        [:lasgn, :n, [:lit, 1]],
        [:iter, [:call, nil, :m, [:arglist]], nil, [:lasgn, :n, [:lit, 2]]],
        [:lvar, :n]]
    end

    compile do |g|
      g.push 1
      g.set_local 0
      g.pop

      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push 2
          d.set_local 0
        end
      end

      g.pop
      g.push_local 0
    end
  end

  relates "m(a) { |b| a + x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist, [:call, nil, :a, [:arglist]]]],
       [:lasgn, :b],
       [:call,
        [:call, nil, :a, [:arglist]],
        :+,
        [:arglist, [:call, nil, :x, [:arglist]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self
        g.push :self
        g.send :a, 0, true

        g.in_block_send :m, :single, nil, 1 do |d|
          d.push :self
          d.send :a, 0, true
          d.push :self
          d.send :x, 0, true
          d.send :+, 1, false
        end
      end
    end
  end

  relates <<-ruby do
      m { |a|
        a + x
      }
    ruby

    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:lasgn, :a],
       [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]]]
    end

    compile(&single_arg_block)
  end

  relates <<-ruby do
      m do |a|
        a + x
      end
    ruby

    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       [:lasgn, :a],
       [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]]]
    end

    compile(&single_arg_block)
  end

  relates "obj.m { |a| a + x }" do
    parse do
      [:iter,
       [:call, [:call, nil, :obj, [:arglist]], :m, [:arglist]],
       [:lasgn, :a],
       [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self
        g.send :obj, 0, true

        g.in_block_send :m, :single, nil, 0, false do |d|
          d.push_local_depth 0, 0
          d.push :self
          d.send :x, 0, true
          d.send :+, 1, false
        end
      end
    end
  end

  relates "obj.m(x) { |a| a + x }" do
    parse do
      [:iter,
       [:call,
        [:call, nil, :obj, [:arglist]],
        :m,
        [:arglist, [:call, nil, :x, [:arglist]]]],
       [:lasgn, :a],
       [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self
        g.send :obj, 0, true
        g.push :self
        g.send :x, 0, true

        g.in_block_send :m, :single, nil, 1, false do |d|
          d.push_local_depth 0, 0
          d.push :self
          d.send :x, 0, true
          d.send :+, 1, false
        end
      end
    end
  end

  relates "obj.m(a) { |a| a + x }" do
    parse do
      [:iter,
       [:call,
        [:call, nil, :obj, [:arglist]],
        :m,
        [:arglist, [:call, nil, :a, [:arglist]]]],
       [:lasgn, :a],
       [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self
        g.send :obj, 0, true
        g.push :self
        g.send :a, 0, true

        g.in_block_send :m, :single, nil, 1, false do |d|
          d.push_local_depth 0, 0
          d.push :self
          d.send :x, 0, true
          d.send :+, 1, false
        end
      end
    end
  end

  relates "a = 1; m { |a| a + x }" do
    parse do
      [:block,
       [:lasgn, :a, [:lit, 1]],
       [:iter,
        [:call, nil, :m, [:arglist]],
        [:lasgn, :a],
        [:call, [:lvar, :a], :+, [:arglist, [:call, nil, :x, [:arglist]]]]]]
    end

    compile do |g|
      g.push 1
      g.set_local 0
      g.pop

      g.passed_block do
        g.push :self

        iter = g.block_description do |d|
          d.cast_for_single_block_arg
          d.set_local 0

          d.pop
          d.push_modifiers
          d.new_label.set!

          d.push_local 0
          d.push :self
          d.send :x, 0, true
          d.send :+, 1, false

          d.pop_modifiers
          d.ret
        end
        iter.required = 1

        g.send_with_block :m, 0, true
      end
    end
  end

  relates <<-ruby do
      x = nil
      m do |a|
        begin
          x
        rescue Exception => x
          break
        ensure
          x = a
        end
      end
    ruby

    parse do
      [:block,
       [:lasgn, :x, [:nil]],
       [:iter,
        [:call, nil, :m, [:arglist]],
        [:lasgn, :a],
        [:ensure,
         [:rescue,
          [:lvar, :x],
          [:resbody,
           [:array, [:const, :Exception], [:lasgn, :x, [:gvar, :$!]]],
           [:break]]],
         [:lasgn, :x, [:lvar, :a]]]]]
    end

    # TODO
  end

  relates "m { next }" do
    parse do
      [:iter, [:call, nil, :m, [:arglist]], nil, [:next]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push :nil
          d.ret
        end
      end
    end
  end

  relates "m { next if x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:if, [:call, nil, :x, [:arglist]], [:next], nil]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          f = d.new_label
          done = d.new_label

          d.push :self
          d.send :x, 0, true
          d.gif f

          d.push :nil
          d.ret
          d.goto done

          f.set!
          d.push :nil

          done.set!
        end
      end
    end
  end

  relates "m { next x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:next, [:call, nil, :x, [:arglist]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push :self
          d.send :x, 0, true
          d.ret
        end
      end
    end
  end

  relates "m { next [1] }" do
    parse do
      [:iter, [:call, nil, :m, [:arglist]], nil, [:next, [:array, [:lit, 1]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push 1
          d.make_array 1
          d.ret
        end
      end
    end
  end

  relates "m { next *[1] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:next, [:svalue, [:splat, [:array, [:lit, 1]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.splatted_array
          d.ret
        end
      end
    end
  end

  relates "m { next [*[1]] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:next, [:array, [:splat, [:array, [:lit, 1]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.array_of_splatted_array
          d.ret
        end
      end
    end
  end

  relates "m { next *[1, 2] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:next, [:svalue, [:splat, [:array, [:lit, 1], [:lit, 2]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.splatted_array 2 do
            d.push 1
            d.push 2
          end
          d.ret
        end
      end
    end
  end

  relates "m { next [*[1, 2]] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:next, [:array, [:splat, [:array, [:lit, 1], [:lit, 2]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.array_of_splatted_array 2 do
            d.push 1
            d.push 2
          end
          d.ret
        end
      end
    end
  end

  relates "m { break }" do
    parse do
      [:iter, [:call, nil, :m, [:arglist]], nil, [:break]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push :nil
          d.break_raise
        end
      end
    end
  end

  relates "m { break if x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:if, [:call, nil, :x, [:arglist]], [:break], nil]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          f = d.new_label
          done = d.new_label

          d.push :self
          d.send :x, 0, true
          d.gif f

          d.push :nil
          d.break_raise
          d.goto done

          f.set!
          d.push :nil

          done.set!
        end
      end
    end
  end

  relates "m { break x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:break, [:call, nil, :x, [:arglist]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push :self
          d.send :x, 0, true
          d.break_raise
        end
      end
    end
  end

  relates "m { break [1] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:break, [:array, [:lit, 1]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push 1
          d.make_array 1
          d.break_raise
        end
      end
    end
  end

  relates "m { break *[1] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:break, [:svalue, [:splat, [:array, [:lit, 1]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.splatted_array
          d.break_raise
        end
      end
    end
  end

  relates "m { break [*[1]] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:break, [:array, [:splat, [:array, [:lit, 1]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.array_of_splatted_array
          d.break_raise
        end
      end
    end
  end

  relates "m { break *[1, 2] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:break, [:svalue, [:splat, [:array, [:lit, 1], [:lit, 2]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.splatted_array 2 do
            d.push 1
            d.push 2
          end
          d.break_raise
        end
      end
    end
  end

  relates "m { break [*[1, 2]] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:break, [:array, [:splat, [:array, [:lit, 1], [:lit, 2]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.array_of_splatted_array 2 do
            d.push 1
            d.push 2
          end
          d.break_raise
        end
      end
    end
  end

  relates "m { return }" do
    parse do
      [:iter, [:call, nil, :m, [:arglist]], nil, [:return]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push :nil
          d.return_raise
        end
      end
    end
  end

  relates "m { return if x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:if, [:call, nil, :x, [:arglist]], [:return], nil]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          f = d.new_label
          done = d.new_label

          d.push :self
          d.send :x, 0, true
          d.gif f

          d.push :nil
          d.return_raise
          d.goto done

          f.set!
          d.push :nil

          done.set!
        end
      end
    end
  end

  relates "m { return x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:return, [:call, nil, :x, [:arglist]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push :self
          d.send :x, 0, true
          d.return_raise
        end
      end
    end
  end

  relates "m { return [1] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:return, [:array, [:lit, 1]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.push 1
          d.make_array 1
          d.return_raise
        end
      end
    end
  end

  relates "m { return *[1] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:return, [:svalue, [:splat, [:array, [:lit, 1]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.splatted_array
          d.return_raise
        end
      end
    end
  end

  relates "m { return [*[1]] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:return, [:array, [:splat, [:array, [:lit, 1]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.array_of_splatted_array
          d.return_raise
        end
      end
    end
  end

  relates "m { return *[1, 2] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:return, [:svalue, [:splat, [:array, [:lit, 1], [:lit, 2]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.splatted_array 2 do
            d.push 1
            d.push 2
          end
          d.return_raise
        end
      end
    end
  end

  relates "m { return [*[1, 2]] }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:return, [:array, [:splat, [:array, [:lit, 1], [:lit, 2]]]]]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.in_block_send :m, :none do |d|
          d.array_of_splatted_array 2 do
            d.push 1
            d.push 2
          end
          d.return_raise
        end
      end
    end
  end

  relates "m { redo }" do
    parse do
      [:iter, [:call, nil, :m, [:arglist]], nil, [:redo]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.block_description do |d|
          d.pop
          redo_lbl = d.new_label
          d.push_modifiers
          redo_lbl.set!
          d.goto redo_lbl
          d.pop_modifiers
          d.ret
        end

        g.send_with_block :m, 0, true
      end
    end
  end

  relates "m { redo if x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist]],
       nil,
       [:if, [:call, nil, :x, [:arglist]], [:redo], nil]]
    end

    compile do |g|
      g.passed_block do
        g.push :self

        g.block_description do |d|
          redo_lbl = d.new_label
          f = d.new_label
          done = d.new_label

          d.pop
          d.push_modifiers
          redo_lbl.set!

          d.push :self
          d.send :x, 0, true
          d.gif f

          d.goto redo_lbl
          d.goto done

          f.set!
          d.push :nil

          done.set!
          d.pop_modifiers
          d.ret
        end

        g.send_with_block :m, 0, true
      end
    end
  end

  relates "m(a) { retry }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist, [:call, nil, :a, [:arglist]]]],
       nil,
       [:retry]]
    end

    # TODO
  end

  relates "m(a) { retry if x }" do
    parse do
      [:iter,
       [:call, nil, :m, [:arglist, [:call, nil, :a, [:arglist]]]],
       nil,
       [:if, [:call, nil, :x, [:arglist]], [:retry], nil]]
    end

    # TODO
  end

  relates "break" do
    parse do
      [:break]
    end

    compile do |g|
      g.push :nil
      g.pop
      g.push_const :Compiler
      g.find_const :Utils
      g.send :__unexpected_break__, 0
    end
  end

  relates "redo" do
    parse do
      [:redo]
    end

    compile do |g|
      g.invalid_context :redo
    end
  end

  relates "retry" do
    parse do
      [:retry]
    end

    compile do |g|
      g.invalid_context :retry
    end
  end

  relates "next" do
    parse do
      [:next]
    end

    compile do |g|
      g.invalid_context :next
    end
  end
end
