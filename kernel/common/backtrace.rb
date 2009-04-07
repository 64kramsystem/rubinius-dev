##
# Contains all logic for gathering and displaying backtraces.

class Backtrace
  include Enumerable

  MAX_WIDTH = 40

  attr_accessor :first_color
  attr_accessor :kernel_color
  attr_accessor :eval_color

  def initialize(locations)
    color_config = Rubinius::RUBY_CONFIG["rbx.colorize_backtraces"]
    if ENV['RBX_NOCOLOR'] or color_config == "no" or color_config == "NO"
      @colorize = false
    else
      @colorize = true
    end

    @locations = locations
    @first_color = "\033[0;31m"
    @kernel_color = "\033[0;34m"
    @eval_color = "\033[0;33m"
  end

  def [](index)
    @locations[index]
  end

  def show(sep="\n")
    first = true
    if @colorize
      clear = "\033[0m"
    else
      clear = ""
    end

    max = 0
    lines = @locations.map do |loc|
      str = loc.describe
      max = str.size if str.size > max
      [str, loc]
    end
    max = MAX_WIDTH if max > MAX_WIDTH

    formatted = lines.map do |recv, location|
      pos  = location.position
      color = color_from_loc(pos, first) if @colorize
      first = false # special handling for first line
      times = max - recv.size
      times = 0 if times < 0
      "#{color}    #{' ' * times}#{recv} at #{pos}#{clear}"
    end
    return formatted.join(sep)
  end

  def join(sep)
    show
  end

  alias_method :to_s, :show

  def color_from_loc(loc, first)
    return @first_color if first
    if loc =~ /^kernel/
      @kernel_color
    elsif loc =~ /\(eval\)/
      @eval_color
    else
      ""
    end
  end

  def self.backtrace(locations)
    return new(locations)
  end

  def each
    @locations.each { |f| yield f }
    self
  end

  # HACK: This should be MRI compliant-ish. --rue
  #
  def to_mri()
    @locations.map do |loc|
      meth =  if loc.is_block
                "#{loc.name} {}"
              elsif loc.name == loc.method.name
                "#{loc.name}"
              else
                "#{loc.name} (#{loc.method.name})"
              end

      "#{loc.position}:in `#{meth}'"
    end
  end
end
