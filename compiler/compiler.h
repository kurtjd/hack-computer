#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

// Recursively parses tokens by following language rules
void cp_compile(const Tokenizer *tk);

#endif
