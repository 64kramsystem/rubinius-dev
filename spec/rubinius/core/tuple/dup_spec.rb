require File.dirname(__FILE__) + '/../../../spec_helper'

describe "Tuple#dup" do
  it "returns a copy of tuple" do
    t = Tuple[:a, 'three', 4].dup
    t[0].should == :a
    t[1].should == 'three'
    t[2].should == 4
    t.size.should == 3
  end
end
