class Regexp

  def self.allocate
    Ruby.primitive :regexp_allocate
    raise PrimitiveFailure, "Regexp.allocate primitive failed"
  end

  ##
  # See Regexp.new. This may be overridden by subclasses.

  def initialize(pattern, opts, lang)
    Ruby.primitive :regexp_initialize
    raise PrimitiveFailure,
          "regexp_new(#{str.inspect}, #{opts}, #{lang.inspect}) primitive failed"
  end

  def search_region(str, start, finish, forward) # equiv to MRI's re_search
    Ruby.primitive :regexp_search_region
    raise PrimitiveFailure, "Regexp#search_region primitive failed"
  end

  def match_start(str, offset) # equiv to MRI's re_match
    Ruby.primitive :regexp_match_start
    raise PrimitiveFailure, "Regexp#match_start primitive failed"
  end

  def options
    Ruby.primitive :regexp_options
    raise PrimitiveFailure, "Regexp#options primitive failed"
  end
end
