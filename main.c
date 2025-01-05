#include "json.h"

int main(void) {
  char *input = "[1, 2, 3]";
  lexer l = {
    .input = input,
    .input_len = strlen(input)
  };
  parser p = {
    .l = &l
  };
  json_value j = parser_parse(&p);
  print_json(j);
}
