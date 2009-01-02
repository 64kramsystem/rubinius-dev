require File.dirname(__FILE__) + '/../../../spec_helper'

process_is_foreground do
  require 'readline'

  not_supported_on :ironruby do
    describe "Readline::HISTORY.empty?" do
      it "returns true when the history is empty" do
        Readline::HISTORY.should be_empty
        Readline::HISTORY.push("test")
        Readline::HISTORY.should_not be_empty
        Readline::HISTORY.pop
        Readline::HISTORY.should be_empty
      end
    end
  end
end
