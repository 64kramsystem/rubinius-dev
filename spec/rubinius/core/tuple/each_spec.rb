require File.dirname(__FILE__) + '/../../../spec_helper'

describe "Tuple#each" do
  it "passes each element to the block" do
    t = Tuple[:a, :c, :e, :g]
    ary = []
    t.each { |e| ary << e }
    ary.should == [:a, :c, :e, :g]
  end
  
  it "does nothing if tuple has zero length" do
    t = Tuple.new(0)
    ary = []
    t.each { |e| ary << e }
    ary.should == []
  end
end
