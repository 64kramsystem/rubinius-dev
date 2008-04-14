require File.dirname(__FILE__) + "/../spec_helper.rb"

describe "Help#execute" do
  before :each do
    @dbg = Debugger.instance
    @dbg.load_commands
  end

  it "returns an array of commands and descriptions" do
    hlp = Debugger::Help.new
    out = hlp.execute @dbg, nil
    out.class.should == Debugger::Output
  end
end