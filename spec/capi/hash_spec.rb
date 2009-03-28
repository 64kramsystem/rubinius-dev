require File.dirname(__FILE__) + '/spec_helper'

load_extension("hash")

describe "C-API Hash function" do
  before :each do
    @s = CApiHashSpecs.new
  end

  describe "rb_hash_new" do
    it "returns a new hash" do
      @s.rb_hash_new.should == {}
    end
  end

  describe "rb_hash_aref" do
    it "returns the value associated with the key" do
      hsh = {:chunky => 'bacon'}
      @s.rb_hash_aref(hsh, :chunky).should == 'bacon'
    end

    it "returns nil if the key does not exist" do
      hsh = { }
      @s.rb_hash_aref(hsh, :chunky).should be_nil
      @s.rb_hash_aref_nil(hsh, :chunky).should be_true
    end
  end

  describe "rb_hash_aset" do
    it "adds the key/value pair and returns the value" do
      hsh = {}
      @s.rb_hash_aset(hsh, :chunky, 'bacon').should == 'bacon'
      hsh.should == {:chunky => 'bacon'}
    end
  end

  describe "rb_hash_delete" do
    it "removes the key and returns the value" do
      hsh = {:chunky => 'bacon'}
      @s.rb_hash_delete(hsh, :chunky).should == 'bacon'
      hsh.should == {}
    end
  end
end
