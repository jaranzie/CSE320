#include <stdio.h>
#include <stdlib.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"



#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

void printStats();

int isWhiteSpace(int c);

int main(int argc, char **argv)
{
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    int c;
    if (global_options & VALIDATE_OPTION) {
        while (1) {
            c = fgetc(stdin);
            if (c == '[') {
                ungetc(c,stdin);
                REVERKI_RULE* rule = reverki_parse_rule(stdin);
                if(rule == NULL) {
                    fprintf(stderr, "Invalid Rule\n");
                    return EXIT_FAILURE;
                }
            }
            else if (c == '(') {
                ungetc(c,stdin);
                REVERKI_TERM * term = reverki_parse_term(stdin);
                if(term == NULL) {
                    fprintf(stderr, "Invalid Term\n");
                    return EXIT_FAILURE;
                }
            }
            else if (c == EOF) {
                printStats();
                return EXIT_SUCCESS;
            }
            else if (isWhiteSpace(c)) {
                continue;
            }
            else {
                return EXIT_FAILURE;
            }
        }
    }
    REVERKI_RULE * head = NULL;
    REVERKI_RULE * last_rule = NULL;
    if (global_options & REWRITE_OPTION) {
        while (1) {
            c = fgetc(stdin);
            if (c == '[') {
                ungetc(c,stdin);
                REVERKI_RULE* rule = reverki_parse_rule(stdin);
                if(rule == NULL) {
                    fprintf(stderr, "Invalid Rule\n");
                    return EXIT_FAILURE;
                }
                if (global_options & TRACE_OPTION) {
                    fputc('#', stderr);
                    fputc(' ', stderr);
                    reverki_unparse_rule(rule, stderr);
                    fputc('\n', stderr);
                }

                if(head == NULL) {
                    head = rule;
                    last_rule = head;
                }
                else {
                    last_rule->next = rule;
                    last_rule = rule;
                }

            }
            else if (c == '(') {
                break;
            }
            else if (c == EOF) {
                fprintf(stderr,"No term found to parse");
                return EXIT_FAILURE;
            }
            else if (isWhiteSpace(c)) {
                continue;
            }
            else {
                return EXIT_FAILURE;
            }
        }
    }
    while (c != EOF) {
        if (c == '(') {
            ungetc(c,stdin);
            REVERKI_TERM * term = reverki_parse_term(stdin);
            if(term == NULL) {
                fprintf(stderr, "Invalid Term\n");
                return EXIT_FAILURE;
            }
            if (global_options & TRACE_OPTION) {
                fputc('#', stderr);
                fputc(' ', stderr);
                reverki_unparse_term(term, stderr);
                fputc('\n', stderr);
            }
            REVERKI_TERM * rewrittenTerm = reverki_rewrite(head,term);
            reverki_unparse_term(rewrittenTerm,stdout);
            fputc('\n',stdout);
            c = fgetc(stdin);
        }
        else if(isWhiteSpace(c)) {
            c = fgetc(stdin);
            continue;
        }
        else {
            fprintf(stderr, "Bad char found in stdin\n");
            return EXIT_FAILURE;
        }
    }
    printStats();




    return 0;
    // TO BE IMPLEMENTED
    return EXIT_FAILURE; 
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
