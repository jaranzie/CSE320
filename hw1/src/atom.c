#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "reverki.h"
#include "global.h"


/* Index for current atom */
static int currentAtom = 0;
int isWhiteSpace(int c);
int isNotAllowed(int c);
int isLowerCase(int c);
int comparePnames(REVERKI_ATOM atom1, REVERKI_ATOM atom2);
int getAtomArrayLength();
int skipWhiteSpace(FILE * in);


/*
 * @brief  Parse an atom  from a specified input stream and return the resulting object.
 * @details  Read characters from the specified input stream and attempt to interpret
 * them as an atom.  An atom may start with any non-whitespace character that is not
 * a left '(' or right '(' parenthesis, a left '[' or right ']' square bracket,
 * or a comma ','.  If the first character read is one of '(', ')', '[', ']', then it
 * is not an error; instead, the character read is pushed back into the input stream
 * and NULL is returned.  Besides the first character, an atom may consist
 * of any number of additional characters (other than whitespace and the punctuation
 * mentioned previously), up to a maximum length of REVERKI_PNAME_BUFFER_SIZE-1.
 * If this maximum length is exceeded, then an error message is printed (on stderr)
 * and NULL is returned.  When a whitespace or punctuation character is encountered
 * that signals the end of the atom, this character is pushed back into the input
 * stream.  If the atom is terminated due to EOF, no character is pushed back.
 * An atom that starts with a lower-case letter has type REVERKI_VARIABLE_TYPE,
 * otherwise it has type REVERKI_CONSTANT_TYPE.  An atom is returned having the
 * sequence of characters read as its pname, and having the type determined by this
 * pname.  There can be at most one atom having a given pname, so if the pname read
 * is already the pname of an existing atom, then a pointer to the existing atom is
 * returned; otherwise a pointer to a new atom is returned.
 * @param in  The stream from which characters are to be read.
 * @return  A pointer to the atom, if the parse was successful, otherwise NULL if
 * an error occurred.
 */
REVERKI_ATOM *reverki_parse_atom(FILE *in) {
    /*Check if room for atom*/
    if(currentAtom >= REVERKI_NUM_ATOMS) {
        fprintf(stderr, "atom storage full\n");
        return NULL;
    }
    REVERKI_ATOM atom;
    int c = skipWhiteSpace(in);
    if (isNotAllowed(c)) {
        /* Push character back */
        ungetc(c,in);
        return NULL;
    }
    /*Determine Atom Type*/
    if (isLowerCase(c)) {
        atom.type = REVERKI_VARIABLE_TYPE;
    }
    else {
        atom.type = REVERKI_CONSTANT_TYPE;
    }
    int pnameIndex = 0;
    while (c != EOF && !isNotAllowed(c) && !isWhiteSpace(c)) {
        if (pnameIndex >= 63) /* possibly change to 63/65 */ {
            fprintf(stderr, "Maximum pname Length Exceeded\n");
            return NULL;
        }
        *(atom.pname + pnameIndex) = c;
        pnameIndex++;
        c = fgetc(in);
    }
    *(atom.pname + pnameIndex) = '\0';
    if (c != EOF) {
        ungetc(c,in);
    }
    for(int i = 0; i <= currentAtom; i++) {
        REVERKI_ATOM atom2 = *(reverki_atom_storage + i);
        if(comparePnames(atom, atom2)) {
            return (reverki_atom_storage + i);
        }
    }
    REVERKI_ATOM* newAddress = (reverki_atom_storage + currentAtom);
    *(newAddress) = atom;
    currentAtom++;
    return newAddress;
}

/*
 * @brief  Output the pname of a specified atom to the specified output stream.
 * @details  The pname of the specified atom is output to the specified output stream.
 * If the output is successful, then 0 is returned.  If any error occurs then the
 * value EOF is returned.
 * @param atom  The atom whose pname is to be printed.
 * @param out  Stream to which the pname is to be printed.
 * @return  0 if output was successful, EOF if not.
 */
int reverki_unparse_atom(REVERKI_ATOM *atom, FILE *out) {
    int index = 0;
    REVERKI_ATOM at = (*atom);
    char c = *(at.pname);
    while (c != '\0' && index < 63) {
        *(reverki_pname_buffer + index) = c;
        index++;
        c = *(at.pname+index);
    }
    *(reverki_pname_buffer + index) = '\0';
    if(c != '\0') {
        return EOF;
    }
    char* ch;
    for(ch = reverki_pname_buffer; *ch; ch++) {
        int val = fputc(*ch, out);
        if(val == EOF) {
            return EOF;
        }
    }
    return 0;
}

/* Returns 1 if is a white space character, 0 otherwise */
int isWhiteSpace(int c) {
    if (c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f') {
        return 1;
    }
    return 0;
}
int isNotAllowed(int c) {
    if (c == '(' || c == ')' || c == '[' || c == ']' || c == ',') {
        return 1;
    }
    return 0;
}
int isLowerCase(int c) {
    if (c >= 'a' && c <= 'z') {
        return 1;
    }
    return 0;
}
/* Return 1 if equal, 0 if not equal */
int comparePnames(REVERKI_ATOM atom1, REVERKI_ATOM atom2) {
    int index = 0;
    int a = (unsigned char) *atom1.pname;
    int b = (unsigned char) *atom2.pname;
    while (a == b && a != '\0' && b != '\0' && index < 64) {
        index++;
        a = (unsigned char) *(atom1.pname + index);
        b = (unsigned char) *(atom2.pname + index);
    }
    if (a == b) {
        return 1;
    }
    return 0;
}
/*Skips white space characters*/
int skipWhiteSpace(FILE * in) {
    int c;
    do {
        c = fgetc(in);
    } while (isWhiteSpace(c));
    return c;
}


int getAtomArrayLength() {
    return currentAtom;
}
