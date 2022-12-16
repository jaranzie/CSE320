#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "reverki.h"
#include "global.h"

static int currentRule = 0;
extern int isWhiteSpace(int c);
int getRuleArrayLength();
/*
 * @brief  Create a rule with a specified left-hand side and right-hand side terms.
 * @details  A rule is created that contains specified terms as its left-hand side
 * and right-hand side.
 * @param lhs  Term to use as the left-hand side of the rule.
 * @param rhs  Term to use as the right-hand side of the rule.
 * @return  A pointer to the newly created rule.
 */
REVERKI_RULE *reverki_make_rule(REVERKI_TERM *lhs, REVERKI_TERM *rhs) {
    if(currentRule >= REVERKI_NUM_RULES) {
        fprintf(stderr, "rule storage full\n");
        return NULL;
    }
    REVERKI_RULE rule;
    rule.lhs = lhs;
    rule.rhs = rhs;
    rule.next = NULL;
    REVERKI_RULE * address = (reverki_rule_storage + currentRule);
    *(address) = rule;
    currentRule++;
    return address;
}

/*
 * @brief  Parse a rule from a specified input stream and return the resulting object.
 * @details  Read characters from the specified input stream and attempt to interpret
 * them as a rule.  A rule starts with a left square bracket '[', followed by a term,
 * then a comma ',' and a second term, and finally terminated by a right square bracket ']'.
 * Arbitrary whitespace may appear before a term or in between the atoms and punctuation
 * that make up the rule.  If, while parsing a rule, the first non-whitespace character
 * seen is not the required initial left square bracket, then the character read is
 * pushed back into the input stream and NULL is returned.  If, while parsing a rule,
 * the required commma ',' or right square bracket ']' is not seen, or parsing one of
 * the two subterms fails, then the unexpected character read is pushed back to the
 * input stream, an error message is issued (to stderr) and NULL is returned.
 * @param in  The stream from which characters are to be read.
 * @return  A pointer to the newly created rule, if parsing was successful,
 * otherwise NULL.
 */
REVERKI_RULE *reverki_parse_rule(FILE *in) {
    /* add white space remover */
    if(currentRule >= REVERKI_NUM_RULES) {
        fprintf(stderr, "rule storage full\n");
        return NULL;
    }
    int c;
    do {
        c = fgetc(in);

    } while (isWhiteSpace(c));
    if (c != '[') {
        fprintf(stderr,"didnt start with bracket\n");
        return NULL;
    }
    REVERKI_TERM * left = reverki_parse_term(in);
    if (left == NULL) {
        ungetc(c,in);
        return NULL;
    }
    do {
        c = fgetc(in);

    } while (isWhiteSpace(c));
    /* maybe remove whitespace */
    if (c != ',') {
        ungetc(c,in);
        fprintf(stderr,"comma not properly placed\n");
        return NULL;
    }
    REVERKI_TERM * right = reverki_parse_term(in);
    if (right == NULL) {
        ungetc(c,in);
        return NULL;
    }
    c = fgetc(in);
    /* maybe remove whitespace */
    if (c != ']') {
        fprintf(stderr,"closing bracket missing\n");
        return NULL;
    }
    return reverki_make_rule(left,right);
}

/*
 * @brief  Output a textual representation of a specified rule to a specified output stream.
 * @details  A textual representation of the specified rule is output to the specified
 * output stream.  The textual representation is of a form from which the original rule
 * can be reconstructed using reverki_parse_rule.  If the output is successful, then 0
 * is returned.  If any error occurs then the value EOF is returned.
 * @param rule  The rule to be printed.
 * @param out  Stream to which the rule is to be printed.
 * @return  0 if output was successful, EOF if not.
 */
int reverki_unparse_rule(REVERKI_RULE *rule, FILE *out) {
    fputc('[', out);
    int i = reverki_unparse_term((*rule).lhs,out);
    if (i == EOF) {
        return EOF;
    }
    fputc(',',out);
    i = reverki_unparse_term((*rule).rhs,out);
    if (i == EOF) {
        return EOF;
    }
    fputc(']',out);
    return i;
    /*abort();*/
}

int getRuleArrayLength() {
    return currentRule;
}

void recycleSubs(REVERKI_SUBST* subs) {
    int subsLength = 0;
    REVERKI_SUBST subst = *subs;
    while (subst != NULL) {
        subsLength++;
        subst = subst->next;
    }
    *subs = NULL;
    currentRule = currentRule - subsLength;
}
