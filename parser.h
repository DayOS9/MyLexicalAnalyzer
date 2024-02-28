/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
/* 
Name: Daois Sanchez Mora
ID  : 1221797369
Sun Feb 11 23:49:52 MST 2024
*/
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"

class Parser {
  public:
    void parse_input();
    void readAndPrintAllInput();
  private:
    LexicalAnalyzer lexer;
    void syntax_error(std::string lexeme);
	void snytax_error();
	void semantic_error(std::string lexeme, int lineNum);
    Token expect(TokenType expected_type, std::string lexeme);
    void parse_tokens_section();
    void parse_token_list();
    void parse_token();
    struct REG* parse_expr(std::string lexeme);
};

#endif

