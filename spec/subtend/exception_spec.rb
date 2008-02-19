require File.dirname(__FILE__) + '/../spec_helper'
require File.dirname(__FILE__) + '/subtend_helper'

compile_extension('subtend_exception')
require File.dirname(__FILE__) + '/ext/subtend_exception'

describe "SubtendRaiser" do
  before :each do
    @s = SubtendRaiser.new
  end

  it "rb_raise should raise an exception" do
    lambda { @s.raise! }.should raise_error(TypeError)
  end

  it "rb_raise terminates the function early" do
    h = {}
    lambda { @s.raise_early(h) }.should raise_error(TypeError)
    h[:screwed].should == false
  end
end
