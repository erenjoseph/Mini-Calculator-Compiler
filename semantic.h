/* semantic.h - Semantic analyzer interface (Phase 3: meaning checks on the AST) */
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/* Checks the AST, prints every semantic error it finds and finally prints the
   symbol table. Returns the number of semantic errors (0 means success). */
int semantic_analyze(const AstNode *program);

#endif
