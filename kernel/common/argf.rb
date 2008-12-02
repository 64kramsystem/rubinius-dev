# depends on: class.rb

##
# The virtual concatenation file of the files given on command line (or from
# $stdin if no files were given).

class ARGFClass
  def getc
    stream = next_stream
    stream.getc if stream
  end

  def gets
    stream = next_stream
    if stream
      stream.lineno = $.
      value = stream.gets
      @lineno = stream.lineno
      value
    end
  end

  def each_line(&block)
    while stream = next_stream
      stream.each_line(&block)
    end
  end

  def each(&block)
    each_line &block
  end

  def each_byte(&block)
    while stream = next_stream
      stream.each_byte(&block)
    end
  end

  def read(bytes=nil, output=nil)
    result = ""
    have_enough = false
    while not have_enough and stream = next_stream
      to_read = bytes ? (bytes-result.size) : nil
      result << stream.read(to_read)
      have_enough = bytes ? (result.size == bytes) : false
    end
    output ? output << result : result
  end

  def readlines
    lines = []
    while stream = next_stream
      lines << stream.gets
    end
    lines.any? ? lines : nil
  end

  def readline
    stream = next_stream
    raise EOFError unless stream
    stream.readline
  end

  alias_method :to_a, :readlines

  def readchar
    stream = next_stream
    raise EOFError unless stream
    stream.readchar
  end

  def pos
    raise ArgumentError if closed?
    current_stream.pos
  end

  alias_method :tell, :pos

  def pos=(position)
    current_stream.pos = position unless current_stream.closed?
  end

  def seek(offset, whence=IO::SEEK_SET)
    current_stream.seek(offset, whence) unless closed? or current_stream.closed?
  end

  def rewind
    raise ArgumentError if closed?
    $. = @last_stream_lineno
    current_stream.rewind
  end

  def fileno
    raise ArgumentError if closed?
    @fileno
  end

  def lineno
    @lineno || 0
  end

  def lineno= value
    current_stream.lineno = value
    $. = @lineno = value
    @last_stream_lineno = 0
  end

  alias_method :to_i, :fileno

  def filename
    current_stream
    @last_filename
  end

  alias_method :path, :filename

  def file
    current_stream
    @last_file
  end

  def to_io
    current_stream.to_io
  end

  def skip
    current_stream.close unless closed? or current_stream.closed?
  end

  def eof?
    raise IOError if closed?
    current_stream.eof
  end

  alias_method :eof, :eof?

  def close
    close_stream if current_stream
  end

  def closed?
    ARGV.empty? and @stream == nil
  end

  def current_stream
    @stream or next_stream
  end
  private :current_stream

  def next_stream
    return @stream unless @stream == nil or @stream.closed? or @stream.eof?

    close_stream if @stream
    return nil if ARGV.empty? # nothing left
    $. = @last_stream_lineno = 0 unless @stream # reset values on first file

    $FILENAME = @last_filename = ARGV.shift
    @stream = @last_filename == '-' ? STDIN : File.open(@last_filename, 'r')
    @last_file = @stream
    @fileno = (@fileno || 0) + 1

    @stream
  end
  private :next_stream

  def close_stream
    unless @stream.closed? or @stream.fileno == 0 # STDIN
      @last_stream_lineno = @stream.lineno
      @stream.close
    end
    @stream = nil if ARGV.empty? # close ARGF
  end
  private :close_stream
end

ARGF = ARGFClass.new
