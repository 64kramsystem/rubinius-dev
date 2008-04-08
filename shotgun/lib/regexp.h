void regexp_init(STATE);
char *regexp_version(STATE);
OBJECT regexp_new(STATE, OBJECT pattern, OBJECT options, char *err_buf);
void regexp_cleanup(STATE, OBJECT regexp);
OBJECT regexp_match(STATE, OBJECT regexp, OBJECT string);
OBJECT regexp_match_start(STATE, OBJECT regexp, OBJECT string, OBJECT start);
OBJECT regexp_scan(STATE, OBJECT regexp, OBJECT string);
OBJECT regexp_search_region(STATE, OBJECT regexp, OBJECT string, OBJECT start, OBJECT end, OBJECT forward);
OBJECT regexp_options(STATE, OBJECT regexp);
