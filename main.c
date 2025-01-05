#include "json.h"

int main(void) {
  char *input = "[1, 2, 3]";
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
  arena_free(&a);
}
