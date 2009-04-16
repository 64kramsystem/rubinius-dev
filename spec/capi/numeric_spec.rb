require File.dirname(__FILE__) + '/spec_helper'

load_extension("numeric")

describe "CApiNumericSpecs" do
  before :each do
    @s = CApiNumericSpecs.new
  end

  describe "rb_num2long" do
    it "raises an TypeError if passed nil" do
      lambda { @s.rb_num2long(nil) }.should raise_error(TypeError)
    end

    it "converts a Float" do
      @s.rb_num2long(4.2).should == 4
    end

    it "converts a Bignum" do
      @s.rb_num2long(0x7fff_ffff).should == 0x7fff_ffff
    end

    it "converts a Fixnum" do
      @s.rb_num2long(5).should == 5
    end

    it "calls #to_int to coerce the value" do
      obj = mock("number")
      obj.should_receive(:to_int).and_return(2)
      @s.rb_num2long(obj).should == 2
    end
  end

  describe "rb_num2ulong" do
    it "raises an TypeError if passed nil" do
      lambda { @s.rb_num2ulong(nil) }.should raise_error(TypeError)
    end

    it "converts a Float" do
      @s.rb_num2ulong(4.2).should == 4
    end

    it "converts a Bignum" do
      @s.rb_num2ulong(0xffff_ffff).should == 0xffff_ffff
    end

    it "converts a Fixnum" do
      @s.rb_num2ulong(5).should == 5
    end

    it "calls #to_int to coerce the value" do
      obj = mock("number")
      obj.should_receive(:to_int).and_return(2)
      @s.rb_num2ulong(obj).should == 2
    end
  end
end
