#include <stdlib.h>
#include <stdio.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"

static int currentTerm = 0;
extern int isWhiteSpace(int c);
extern int isNotAllowed(int c);
extern int isLowerCase(int c);
int getTermArrayLength();
static REVERKI_TERM* createTerm(REVERKI_ATOM * atom);

/*
 * @brief  Create a variable term from a specified atom.
 * @details  A term of type REVERKI_VARIABLE is created that contains the
 * specified atom.  The atom must also have type REVERKI_VARIABLE, otherwise
 * an error message is printed and the program aborts.
 * @param atom  The atom from which the term is to be constructed.
 * @return  A pointer to the newly created term.
 */
REVERKI_TERM *reverki_make_variable(REVERKI_ATOM *atom) {
    if ((*atom).type != REVERKI_VARIABLE_TYPE) {
        fprintf(stderr, "atom does not have type REVERKI VARIABLE");
        abort();
    }
    REVERKI_TERM newTerm;
    newTerm.type = REVERKI_VARIABLE_TYPE;
    newTerm.value.atom = atom;
    if(currentTerm < REVERKI_NUM_TERMS) {
        REVERKI_TERM* newAddress = reverki_term_storage + currentTerm;
        *newAddress = newTerm;
        currentTerm++;
        return newAddress;
    }
    fprintf(stderr, "Reverki Term Array Full");
    abort();
}

/* 
 * @brief  Create a constant term from a specified atom.
 * @details  A term of type REVERKI_CONSTANT is created that contains the
 * specified atom.  The atom must also have type REVERKI_CONSTANT, otherwise
 * an error message is printed and the program aborts.
 * @param atom  The atom from which the constant is to be constructed.
 * @return  A pointer to the newly created term.
 */
REVERKI_TERM *reverki_make_constant(REVERKI_ATOM *atom) {
    REVERKI_TERM newTerm;
    newTerm.type = REVERKI_CONSTANT_TYPE;
    if ((*atom).type != REVERKI_CONSTANT_TYPE) {
        fprintf(stderr, "atom does not have type REVERKI CONSTANT");
        abort();
    }
    newTerm.value.atom = atom;
    if(currentTerm < REVERKI_NUM_TERMS) {
        REVERKI_TERM* newAddress = reverki_term_storage + currentTerm; /* *sizeof(REVERKI_TERM);*/
        *newAddress = newTerm;
        currentTerm++;
        return newAddress;
    }
    fprintf(stderr, "Reverki Term Array Full");
    abort();
}

/*
 * @brief  Create a pair term from specified subterms.
 * @details  A term of type REVERKI_PAIR is created that contains specified
 * terms as its first and second subterms.
 * @param fst  The first (or "left-hand") subterm of the pair to be constructed.
 * @param snd  The second (or "right-hand") subterm of the pair to be constructed.
 * @return  A pointer to the newly created term.
 */
REVERKI_TERM *reverki_make_pair(REVERKI_TERM *fst, REVERKI_TERM *snd) {
    REVERKI_TERM newTerm;
    newTerm.type = REVERKI_PAIR_TYPE;
    newTerm.value.pair.fst = fst;
    newTerm.value.pair.snd = snd;
    if(currentTerm < REVERKI_NUM_TERMS) {
        REVERKI_TERM* newAddress = reverki_term_storage + currentTerm;  /*  *sizeof(REVERKI_TERM);*/
        *newAddress = newTerm;
        currentTerm++;
        return newAddress;
    }
    fprintf(stderr, "Reverki Term Array Full");
    abort();
}

/*
 * @brief  Compare two specified terms for equality.
 * @details  The two specified terms are compared for equality.  Equality of terms
 * means that they have the same type and that corresponding atoms or subterms they
 * contain are recursively equal.
 * @param term1  The first of the two terms to be compared.
 * @param term2  The second of the two terms to be compared.
 * @return  Zero if the specified terms are equal, otherwise nonzero.
 */
int reverki_compare_term(REVERKI_TERM *term1, REVERKI_TERM *term2) {
    if(term1 == NULL) {
        return -1;
    }
    if(term2 == NULL) {
        return -1;
    }
    REVERKI_TERM termOne = *term1;
    REVERKI_TERM termTwo = *term2;
    if (termOne.type == termTwo.type) {
        if (termOne.type == REVERKI_CONSTANT_TYPE || termOne.type == REVERKI_VARIABLE_TYPE) {
            if (termOne.value.atom == termTwo.value.atom) {
                return 0;
            }
        }
        if (termOne.type == REVERKI_PAIR_TYPE) {
            return 0 + reverki_compare_term(termOne.value.pair.fst,termOne.value.pair.snd);
        }
    }
    return -1;
}

/*
 * @brief  Parse a term from a specified input stream and return the resulting object.
 * @details  Read characters from the specified input stream and attempt to interpret
 * them as a term.  A term may either have the form of a single atom, or of a
 * sequence of terms enclosed by left parenthesis '(' and right parenthesis ')'.
 * Arbitrary whitespace may appear before a term or in between the atoms and punctuation
 * that make up the term.  Any such whitespace is not part of the term, and is ignored.
 * When a whitespace or punctuation character is encountered that signals the end of
 * the term, this character is pushed back into the input stream.  If the term is
 * terminated due to EOF, no character is pushed back.  A term of the form
 * ( a b c d ... ) is regarded as an abbreviation for ( ... ( ( a b ) c ) d ) ...);
 * that is, parentheses in a term may be omitted under the convention that the subterms
 * associate to the left.  If, while reading a term, a syntactically incorrect atom
 * or improperly matched parentheses are encountered, an error message is issued
 * (to stderr) and NULL is returned.
 * @param in  The stream from which characters are to be read.
 * @return  A pointer to the newly created term, if parsing was successful,
 * otherwise NULL.
 */
REVERKI_TERM *reverki_parse_term(FILE *in) {
    if(currentTerm >= REVERKI_NUM_TERMS) {
        fprintf(stderr, "term storage full\n");
        return NULL;
    }
    REVERKI_TERM * left = NULL;
    REVERKI_TERM * right = NULL;
    int c;
    do {
        c = fgetc(in);
    } while (isWhiteSpace(c));
    if (c != '(') {
        ungetc(c,stdin);
        /*In case sole atom term*/
        REVERKI_ATOM* atom = reverki_parse_atom(in);
        if (atom == NULL) {
            fprintf(stderr, "syntactically incorrect atom\n");
            return NULL;
        }
        do {
            c = fgetc(in);
        } while (isWhiteSpace(c));
        if(c != ']' && c != ',') {
            ungetc(c,in);
            fprintf(stderr, "syntactically incorrect atom\n");
            return NULL;
        }
        else {
            ungetc(c,in);
            return createTerm(atom);
        }
    }
    int numPs = 1;
    while (numPs > 0) {
        if (c == EOF) {
            fprintf(stderr, "mismatched parethesis\n");
            return NULL;
        }
        c = fgetc(in);
        if (isWhiteSpace(c)) {
            continue;
        }
        else if (c == '(') {
            ungetc(c,in);
            REVERKI_TERM * newParsed = reverki_parse_term(in);
            if (newParsed == NULL) {
                return NULL;
            }
            if (left == NULL) {
                left = newParsed;
            }
            else if (right == NULL) {
                right = newParsed;
            }
            else {
                left = reverki_make_pair(left,right);
                right = newParsed;
            }
        }
        else if (c == ')') {
            numPs--;
        }
        /* If atom */
        else if (c > 0) {
            ungetc(c,in);
            REVERKI_ATOM * newAtom = reverki_parse_atom(in);
            if (newAtom == NULL) {
                fprintf(stderr, "syntactically incorrect atom\n");
                return NULL;
            }
            REVERKI_TERM * atomTerm = createTerm(newAtom);
            if (left == NULL) {
                left = atomTerm;
            }
            else if (right == NULL) {
                right = atomTerm;
            }
            else {
                /*right = reverki_make_pair(right, atomTerm);*/
                left = reverki_make_pair(left, right);
                right = atomTerm;
            }
        }
    }
    if (left == NULL && right == NULL) {
        fprintf(stderr,"not a term\n");
        return NULL;
    }
    else if(right == NULL) {
        return left;
    }
    else {
        return reverki_make_pair(left, right);
    }
    abort();
}

/*
 * @brief  Output a textual representation of a specified term to a specified output stream.
 * @details  A textual representation of the specified term is output to the specified
 * output stream.  The textual representation is of a form from which the original term
 * can be reconstructed using reverki_parse_term.  If the output is successful, then 0
 * is returned.  If any error occurs then the value EOF is returned.
 * @param term  The term that is to be printed.
 * @param out  Stream to which the term is to be printed.
 * @return  0 if output was successful, EOF if not.
 */
int reverki_unparse_term(REVERKI_TERM *term, FILE *out) {
    REVERKI_TYPE type = (*term).type;
    if (type == REVERKI_NO_TYPE) {
        return EOF;
    }
    else if (type == REVERKI_PAIR_TYPE) {
        int i = fputc('(',out);
        if(i == EOF) {
            return EOF;
        }
        REVERKI_TERM * left = (*term).value.pair.fst;
        REVERKI_TERM * right = (*term).value.pair.snd;
        i = reverki_unparse_term(left,out);
        if(i == EOF) {
            return EOF;
        }
        i = fputc(' ',out);
        if(i == EOF) {
            return EOF;
        }
        i = reverki_unparse_term(right,out);
        if(i == EOF) {
            return EOF;
        }
        i = fputc(')',out);
        if(i == EOF) {
            return EOF;
        }
        return 0;
    }
    else {
        int i = reverki_unparse_atom((*term).value.atom,out);
        if(i == EOF) {
            return EOF;
        }
        return 0;
    }
    abort();
}


static REVERKI_TERM* createTerm(REVERKI_ATOM * atom) {
    if ((*atom).type == REVERKI_CONSTANT_TYPE) {
        return reverki_make_constant(atom);
    }
    else {
        return reverki_make_variable(atom);
    }
}

int getTermArrayLength() {
    return currentTerm;
}