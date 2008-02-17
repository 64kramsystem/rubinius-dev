require File.dirname(__FILE__) + '/../../spec_helper'

describe "Fixnum#|" do
  it "returns self bitwise OR other" do
    (1 | 0).should == 1
    (5 | 4).should == 5
    (5 | 6).should == 7
    (248 | 4096).should == 4344
    (0xffff | bignum_value + 0xf0f0).should == 0x8000_0000_0000_ffff
  end

  it "raises a RangeError if passed a Float out of Fixnum range" do
    lambda { 1 | bignum_value(10000).to_f }.should raise_error(RangeError)
    lambda { 1 | -bignum_value(10000).to_f }.should raise_error(RangeError)
  end
  
  it "tries to convert the given argument to an Integer using to_int" do
    (5 | 4.3).should == 5
    
    (obj = mock('4')).should_receive(:to_int).and_return(4)
    (3 | obj).should == 7
  end
  
  it "raises a TypeError when the given argument can't be converted to Integer" do
    obj = mock('asdf')
    lambda { 3 | obj }.should raise_error(TypeError)
    
    obj.should_receive(:to_int).and_return("asdf")
    lambda { 3 | obj }.should raise_error(TypeError)
  end

  it "raises a RangeError when the given argument is out of range of Integer" do
    (obj = mock('large value')).should_receive(:to_int).and_return(8000_0000_0000_0000_0000)
    lambda { 3 | obj }.should raise_error(RangeError)
  end
end
