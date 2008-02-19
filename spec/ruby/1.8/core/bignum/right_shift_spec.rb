require File.dirname(__FILE__) + '/../../spec_helper'

describe "Bignum#>>" do
  before(:each) do 
    @bignum = bignum_value(90812)
  end

  it "returns self shifted the given amount of bits to the right" do
    (@bignum >> 1).should == 4611686018427433310
    (@bignum >> 3).should == 1152921504606858327
  end

  it "performs a left-shift if given a negative value" do
    (@bignum >> -1).should == (@bignum << 1)
    (@bignum >> -3).should == (@bignum << 3)
  end
  
  it "tries to convert it's argument to an Integer using to_int" do
    (@bignum >> 1.3).should == 4611686018427433310
    
    (obj = mock('1')).should_receive(:to_int).and_return(1)
    (@bignum >> obj).should == 4611686018427433310
  end
  
  it "raises a TypeError when the given argument can't be converted to Integer" do
    obj = mock('asdf')
    lambda { @bignum >> obj }.should raise_error(TypeError)
    
    obj.should_receive(:to_int).and_return("asdf")
    lambda { @bignum >> obj }.should raise_error(TypeError)
  end

  compliant_on :ruby do
    it "return the right shift alignment" do
      ((1 - 2**31) >> 31).should == -1
      ((1 - 2**32) >> 32).should == -1
      ((1 - 2**63) >> 63).should == -1 
      ((1 - 2**64) >> 64).should == -1 
    end
  end

  platform_is_not :darwin do
    it "return the right shift alignment" do
      ((1 - 2**31) >> 31).should == -1
      ((1 - 2**32) >> 32).should == -1
      ((1 - 2**63) >> 63).should == -1 
      ((1 - 2**64) >> 64).should == -1 
    end
  end
end
