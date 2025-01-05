#ifndef _JSON_H
#define _JSON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

typedef enum {
  TokenComma = 1,
  TokenColon,
  TokenString,
  TokenNumber,
  TokenOpenSquareBracket,
  TokenClosingSquareBracket,
  TokenOpenCurlyBracket,
  TokenClosingCurlyBracket,
  TokenNull,
  TokenEOF
} TokenType;

char *token_type_str(TokenType type) {
  switch(type) {
  case TokenComma:
    return "TokenComma";
  case TokenColon:
    return "TokenColon";
  case TokenString:
    return "TokenString";
  case TokenNumber:
    return "TokenNumber";
  case TokenOpenSquareBracket:
    return "TokenOpenSquareBracket";
  case TokenClosingSquareBracket:
    return "TokenClosingSquareBracket";
  case TokenOpenCurlyBracket:
    return "TokenOpenCurlyBracket";
  case TokenClosingCurlyBracket:
    return "TokenClosingCurlyBracket";
  case TokenNull:
    return "TokenNull";
  case TokenEOF:
    return "TokenEOF";
  default:
    fprintf(stderr, "unexpected token type: %d\n", type);
  }
}

typedef struct {
  TokenType type;
  char *value;
  size_t value_len;
  size_t line;
  size_t col;
} Token;

typedef struct {
  char *input;
  size_t input_len;
  size_t position;
  size_t col;
  size_t line;
} lexer;

typedef struct {
  lexer *l;
  Token t;
} parser;

typedef enum {
  JsonString = 1,
  JsonNumber,
  JsonObject,
  JsonArray,
  JsonNull
} JsonType;

typedef struct {
  JsonType type;
  void *value;
} json_value;

typedef struct {
  char *data;
  size_t data_len;
} string;

typedef struct {
  size_t elements_len;
  json_value *elements;
} json_array;

typedef struct {
  size_t field_len;
  char *field_name;
  json_value field_value;
} obj_field;

typedef struct {
  size_t fields_len;
  obj_field *fields;
} json_obj;

void print_token(Token t) {
  printf("%s: %s\n", token_type_str(t.type), t.value);
}

json_value parser_parse(parser *p);

void print_json(json_value json);

void print_json_array(json_value json) {
  printf(" [");
  json_array *arr = (json_array *) json.value;
  size_t i;
  for (i = 0; i < arr->elements_len; i++) {
    print_json(arr->elements[i]); 
  }
  printf("] ");
}

void print_json_object(json_value json) {
  printf(" { ");
  json_obj *obj = (json_obj *) json.value;
  size_t i;
  for (i = 0; i < obj->fields_len; i++) {
    printf("\"%s\":", obj->fields[i].field_name); 
    print_json(obj->fields[i].field_value); 
  }
  printf(" } ");
}

void print_json(json_value json) {
  switch (json.type) {
    case JsonNull:
      printf(" null ");
      break;
    case JsonNumber:
      printf(" %d ", *(int *)json.value);
      break;
    case JsonString:
      printf(" \"%s\" ", (char *)json.value);
      break;
    case JsonArray:
      print_json_array(json);
      break;
    case JsonObject:
      print_json_object(json);
      break;
    default:
      fprintf(stderr, "unknown json type: %d\n", json.type);
      exit(1);
  }
}


Token eat_digit(lexer *lex) {
  size_t line = lex->line;
  size_t col = lex->col;
  char buf[20] = {0};
  size_t i = 0;
  while (isdigit(lex->input[lex->position])) {
    assert(i < 20 && "Buffer overflow in eat digit");  
    buf[i++] = lex->input[lex->position++];
  }
  char *result = (char *)malloc(i * sizeof(char) + 1);
  size_t j = 0;
  for (j=0; j < i; j++) {
    result[j] = buf[j];
  }
  result[j+1] = '\0';
  return (Token) {
    .type = TokenNumber,
    .value = result,
    .value_len = i,
    .line = line,
    .col = col
  };
}

Token eat_string(lexer *lex) {
  // eat the opening quote
  lex->position++;
  size_t line = lex->line;
  size_t col = lex->col;
  char buf[20] = {0};
  size_t i = 0;
  while (lex->input[lex->position] != '"') {
    if (lex->position >= lex->input_len) {
      fprintf(stderr, "unterminated string line: %zd col: %zd\n", line, col);
      exit(1);
    }
    assert(i < 20 && "Buffer overflow in eat string");  
    buf[i++] = lex->input[lex->position++];
  }
  // eat the closing quote
  lex->position++;
  char *result = (char *)malloc(i * sizeof(char) + 1);
  size_t j = 0;
  for (j=0; j < i; j++) {
    result[j] = buf[j];
  }
  result[j+1] = '\0';
  return (Token) {
    .type = TokenString,
    .value = result,
    .value_len = i,
    .line = line,
    .col = col
  };
}

Token lexer_next(lexer *lex) {
  if (lex->position >= lex->input_len) {
    return (Token) {
      .type = TokenEOF,
      .value = NULL,
      .value_len = 0,
      .line = lex->line,
      .col = lex->col
    };
  }

  // ignore all white space
  if (isspace(lex->input[lex->position])) {
    while (isspace(lex->input[lex->position])) {
      if (lex->position >= lex->input_len) {
        return (Token) {
          .type = TokenEOF,
          .value = NULL,
          .value_len = 0,
          .line = lex->line,
          .col = lex->col
        };
      }

      if (lex->input[lex->position] == '\n') {
        lex->line++;
        lex->col = 0;
        lex->position++;
        continue;
      }
      lex->col++;
      lex->position++;
    }
  }
  
  if (lex->position >= lex->input_len) {
    return (Token) {
      .type = TokenEOF,
      .value = NULL,
      .value_len = 0,
      .line = lex->line,
      .col = lex->col
    };
  }

  char c = lex->input[lex->position];
  size_t line = lex->line;
  size_t col = lex->col;
  switch (c) {
    case ',':
      lex->position++;
      return (Token) {
        .type = TokenComma,
        .value = ",",
        .line = line,
        .col = col
      };
    case ':':
      lex->position++;
      return (Token) {
        .type = TokenColon,
        .value = ":",
        .line = line,
        .col = col
      };
    case '[':
      lex->position++;
      return (Token) {
        .type = TokenOpenSquareBracket,
        .value = "[",
        .line = line,
        .col = col
      };
    case ']':
      lex->position++;
      return (Token) {
        .type = TokenClosingSquareBracket,
        .value = "]",
        .line = line,
        .col = col
      };
    case '{':
      lex->position++;
      return (Token) {
        .type = TokenOpenCurlyBracket,
        .value = "{",
        .line = line,
        .col = col
      };
    case '}':
      lex->position++;
      return (Token) {
        .type = TokenClosingCurlyBracket,
        .value = "}",
        .line = line,
        .col = col
      };
  }

  if (isdigit(c)) {
    return eat_digit(lex);
  }
  
  if (c == '"') {
    return eat_string(lex);
  }

  fprintf(stderr, "unexpected char: %c line: %zd col: %zd\n", c, line, col);
  exit(1);
}

void next_token(parser *p) {
  p->t = lexer_next(p->l);
}

void eat_token(TokenType type, parser *p) {
  if (p->t.type != type) {
    fprintf(
        stderr,
        "unexpected token want: %s got %s line: %zd col: %zd\n",
        token_type_str(type),
        token_type_str(p->t.type),
        p->t.line,
        p->t.col
    );
    exit(1);
  }
  next_token(p);
};

json_value parse_token(parser *p);

json_value parse_array(parser *p) {
  // eat the open square bracket
  next_token(p);
  json_array *ja = (json_array *)malloc(sizeof(json_array));
  json_value elements[200];
  size_t elements_count = 0;
  while (p->t.type != TokenClosingSquareBracket) {
    if (p->t.type == TokenEOF) {
      fprintf(stderr, "unterminated array");
      exit(1);
    }
    assert(elements_count < 200 && "array with more then 200 elements");
    if (elements_count > 0) {
      eat_token(TokenComma, p);
    }
    elements[elements_count] = parse_token(p);
    elements_count++;
    next_token(p);
  }

  json_value *elements_result = (json_value *) malloc(elements_count * sizeof(json_value));
  size_t i = 0;
  for (i = 0; i < elements_count; i++) {
    elements_result[i] = elements[i];
  }

  *ja = (json_array) {
    .elements = elements_result,
    .elements_len = elements_count
  };
  return (json_value) {
    .type = JsonArray,
    .value = (void *)ja
  };
}

char *token_string(char *token_string, size_t token_string_len) {
  char *field = (char *)malloc(sizeof(char) * token_string_len + 1); 
  memcpy(field, token_string, token_string_len);
  field[token_string_len + 1] = '\0';
  return field;
}

json_value parse_obj(parser *p) {
  // eat the open curly bracket
  next_token(p);
  size_t fields_count = 0;
  json_obj *obj = NULL;
  obj_field fields[200];
  while (p->t.type != TokenClosingCurlyBracket) {
    if (p->t.type == TokenEOF) {
      fprintf(stderr, "unterminated object\n");
      exit(1);
    }
    assert(fields_count < 200 && "object with more then 200 fields");

    if (fields_count > 0) {
      eat_token(TokenComma, p);
    }

    if (p->t.type != TokenString) {
      fprintf(stderr, "expected string got: %s line: %zd col: %zd\n", token_type_str(p->t.type), p->t.line, p->t.col); 
      exit(1);
    }
    char *field = token_string(p->t.value, p->t.value_len);
    size_t field_len = p->t.value_len;

    next_token(p);
    eat_token(TokenColon, p);

    json_value field_value = parse_token(p);
    fields[fields_count] = (obj_field) {
      .field_value = field_value,
      .field_name = field,
      .field_len = field_len
    };

    fields_count++;
    next_token(p);
  }

  obj_field *fields_result = (obj_field *)malloc(sizeof(obj_field) * fields_count);
  size_t i = 0;
  for (i = 0; i < fields_count; i++) {
    fields_result[i] = fields[i];
  }

  obj = (json_obj *)malloc(sizeof(json_obj));
  *obj = (json_obj) {
    .fields_len = fields_count,
    .fields = fields_result
  };
  return (json_value) {
    .type = JsonObject,
    .value = (void *)obj
  };
}

json_value parse_token(parser *p) {
  json_value val;
  switch (p->t.type) {
    case TokenEOF:
      val.type = JsonNull;
      val.value = NULL;
      return val;

    case TokenNull:
      val.type = JsonNull;
      val.value = NULL;
      return val;

    case TokenString:
      val.type = JsonString;
      val.value = (void *)p->t.value;
      return val;

    case TokenNumber:
      val.type = JsonNumber;
      int *num = malloc(sizeof(int));
      *num = atoi(p->t.value);
      val.value = (void *)num;
      return val;
    
    case TokenOpenSquareBracket:
      return parse_array(p);

    case TokenOpenCurlyBracket:
      return parse_obj(p);

    default:
      fprintf(stderr, "unexpected token: %s line: %zd col: %zd\n", token_type_str(p->t.type), p->t.line, p->t.col);
      exit(1);
  }
}

json_value parser_parse(parser *p) {
  next_token(p); 
  return parse_token(p);
}

#endif
