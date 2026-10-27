/* semantic.c - Semantic analysis: symbol table, use-before-assignment, division by zero */
#include "semantic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Symbol table: a linked list of variables that have been assigned so far. */
typedef struct Symbol {
    char name[MAX_LEXEME];
    int line;                 /* line of the first assignment */
    struct Symbol *next;
} Symbol;

typedef struct {
    Symbol *head;
    Symbol *tail;
    int errors;
} Analyzer;

static Symbol *find_symbol(const Analyzer *a, const char *name) {
    Symbol *s;
    for (s = a->head; s != NULL; s = s->next) {
        if (strcmp(s->name, name) == 0) {
            return s;
        }
    }
    return NULL;
}

/* Add a variable to the table if it is not there yet. Returns 0 if memory ran out. */
static int declare_symbol(Analyzer *a, const char *name, int line) {
    Symbol *s;
    if (find_symbol(a, name) != NULL) {
        return 1;
    }
    s = malloc(sizeof(Symbol));
    if (s == NULL) {
        return 0;
    }
    snprintf(s->name, sizeof s->name, "%s", name);
    s->line = line;
    s->next = NULL;
    if (a->tail == NULL) {
        a->head = s;
    } else {
        a->tail->next = s;
    }
    a->tail = s;
    return 1;
}

static void free_symbols(Analyzer *a) {
    Symbol *s = a->head;
    while (s != NULL) {
        Symbol *next = s->next;
        free(s);
        s = next;
    }
    a->head = NULL;
    a->tail = NULL;
}

/* Try to compute the value of an expression that contains only numbers.
   Returns 1 and stores the value in *value if that is possible; returns 0 if the
   expression uses a variable or contains a division by zero. */
static int eval_constant(const AstNode *node, double *value) {
    double a, b;
    switch (node->type) {
        case AST_NUMBER:
            *value = strtod(node->name, NULL);
            return 1;
        case AST_UNARY:
            if (!eval_constant(node->left, &a)) {
                return 0;
            }
            *value = (node->op == '-') ? -a : a;
            return 1;
        case AST_BINARY:
            if (!eval_constant(node->left, &a) || !eval_constant(node->right, &b)) {
                return 0;
            }
            switch (node->op) {
                case '+': *value = a + b; return 1;
                case '-': *value = a - b; return 1;
                case '*': *value = a * b; return 1;
                case '/':
                    if (b == 0.0) {
                        return 0;
                    }
                    *value = a / b;
                    return 1;
                default:
                    return 0;
            }
        default:
            return 0;
    }
}

/* Walk an expression tree and apply the semantic rules. */
static void check_expression(Analyzer *a, const AstNode *node) {
    double divisor;

    switch (node->type) {
        case AST_NUMBER:
            break;
        case AST_VARIABLE:
            /* Rule 1: a variable must be assigned before it is used. */
            if (find_symbol(a, node->name) == NULL) {
                printf("Semantic error (line %d): variable '%s' is used before it is assigned "
                       "(undeclared or uninitialized)\n", node->line, node->name);
                a->errors++;
            }
            break;
        case AST_UNARY:
            check_expression(a, node->left);
            break;
        case AST_BINARY:
            check_expression(a, node->left);
            check_expression(a, node->right);
            /* Rule 2: the divisor must not be a constant that equals zero. */
            if (node->op == '/' && eval_constant(node->right, &divisor) && divisor == 0.0) {
                printf("Semantic error (line %d): division by zero\n", node->line);
                a->errors++;
            }
            break;
        default:
            break;
    }
}

int semantic_analyze(const AstNode *program) {
    Analyzer analyzer;
    const AstNode *statement;
    const Symbol *s;

    analyzer.head = NULL;
    analyzer.tail = NULL;
    analyzer.errors = 0;

    for (statement = program->left; statement != NULL; statement = statement->next) {
        if (statement->type == AST_ASSIGN) {
            /* Check the right-hand side FIRST, so that "a = a + 1;" is an error
               when 'a' has not been assigned before. Then record the variable. */
            check_expression(&analyzer, statement->left);
            if (!declare_symbol(&analyzer, statement->name, statement->line)) {
                printf("Semantic error: out of memory\n");
                analyzer.errors++;
                break;
            }
        } else if (statement->type == AST_PRINT) {
            check_expression(&analyzer, statement->left);
        }
    }

    printf("Symbol table (variables assigned in the program):\n");
    if (analyzer.head == NULL) {
        printf("  (empty)\n");
    }
    for (s = analyzer.head; s != NULL; s = s->next) {
        printf("  %-20s first assigned on line %d\n", s->name, s->line);
    }

    free_symbols(&analyzer);
    return analyzer.errors;
}
