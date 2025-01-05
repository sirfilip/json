#include "json.h"

int main(void) {
  char *input = "[1, 2, \"3\", {\"foo\": \"bar\", \"baz\": 42}]";
  arena a = arena_new(1024);
  lexer l = {
    .input = input,
    .input_len = strlen(input),
    .a = &a
  };
  parser p = {
    .l = &l
  };
  json_value j = parser_parse(&p);
  print_json(j);
  printf("\n");
  arena_free(&a);
  return 0;
}
