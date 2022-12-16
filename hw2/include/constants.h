//
// Created by jaranzie on 9/18/22.
//

#ifndef HW2_CONSTANTS_H
#define HW2_CONSTANTS_H


/*
 * Constants
 */
#define NSYM	400
#define LSIZE	129
#define HSHSIZ	211
#define DOT	(*dot)
#define SINST	01
#define SREG	02
#define SIDENT	04
#define SDIRECT	010
#define SDEF	020
#define SDOT	040
#define EOFF	1
#define GARBG	2
#define NEWLN	3
#define SPACE	4
#define EXCLA	5
#define DQUOTE	6
#define SHARP	7
#define DOL	8
#define PCNT	9
#define AND	10
#define SQUOTE	11
#define LPARN	12
#define RPARN	13
#define STAR	14
#define PLUS	15
#define COMMA	16
#define MINUS	17
#define LETTER	18
#define SLASH	19
#define	DIGIT	20
#define COLON	21
#define SEMI	22
#define LESS	23
#define EQUAL	24
#define GREAT	25
#define QUES	26
#define AT	27
#define LBRACK	28
#define BSLASH	29
#define RBRACK	30
#define UPARR	31
#define UNDER	32
#define GRAVE	33
#define LCURL	34
#define VBAR	35
#define RCURL	36
#define TILDE	37

#define TNEWLN	1
#define TEOFF	2
#define TEXCLA	3
#define TDQUO	4
#define TSHARP	5
#define TDOL	6
#define TPCNT	7
#define TAND	8
#define TCONST	9
#define TLPARN	10
#define TRPARN	11
#define TSTAR	12
#define TPLUS	13
#define TCOMMA	14
#define TMINUS	15
#define TSLASH	16
#define TCOLON	17
#define TLESS	18
#define TEQUAL	19
#define TGREAT	20
#define TQUES	21
#define TAT	22
#define TIDENT	23
#define TLBRAK	24
#define TBSLA	25
#define TRBRAK	26
#define TUPARR	27
#define TUNDER	28
#define TGRAVE	29
#define TLCURL	30
#define TVBAR	31
#define TRCURL	32
#define TTILDE	33

#define REGMOD	01
#define IMMMOD	02
#define DEFIMM	04
#define DEFREG	010

#define IHALT	0 /**/
#define IEIS	2 /**/
#define IDIS	3 /**/
#define ITCI	5 /**/
#define ICLRC	6 /**/
#define ISETC	7 /**/
#define ISWAP	8 /**/
#define ISLL	9 /**/
#define IRLC	10 /**/
#define ISLLC	11 /**/
#define ISLR	12 /**/
#define ISAR	13 /**/
#define IRRC	14 /**/
#define ISARC	15 /**/
#define IJ	    16 /**/
#define IJE	    17 /**/
#define IJD	    18 /* */
#define IJSR	20 /**/
#define IJSRE	21 /**/
#define IJSRD	22 /**/
#define IB	    32 /**/
#define INOP2	40 /**/
#define IBC	    33 /**/
#define IBNC	41 /**/
#define IBOV	34 /**/
#define IBNOV	42 /**/
#define IBZE	36 /**/
#define IBNZE	44 /**/
#define IBLGE	33 /**/
#define IBLLT	41 /**/
#define IBPL	35 /**/
#define IBMI	43 /**/
#define IBEQ	36 /**/
#define IBNEQ	44 /**/
#define IBLT	37 /**/
#define IBGE	45 /**/
#define IBLE	38 /**/
#define IBGT	46 /**/
#define IBUSC	39 /**/
#define IBESC	47 /**/
#define IBEXT	64 /**/
#define IMOV	66 /**/
#define IADD	67 /**/
#define ISUB	68 /**/
#define ICMP	69 /* */
#define IAND	70 /**/
#define IXOR	71 /**/
#define IINC	73 /**/
#define IDEC	74 /**/
#define ICOM	75 /**/
#define INEG	76 /**/
#define IADC	77 /* */
#define IGSWD	78 /**/
#define IRSWD	79
#define INOP	80
#define ISIN	81
#define IJMP	82
#define ITST	83 /**/
#define ICLR	84 /**/

union hw {
    short val;
    struct {
        char lobyte;
        char hibyte;
    } bytes;
};

struct symtab {
    char s_name[9];
    int s_flags;
    int s_hash;
    struct symtab *s_link;
    long s_val;
};
void dbyte();
int rtoken();
void dhex();
void doctal(), dascii(),dasciz(),dwordb(),dword();
int line(), undefined();
void lflush(FILE * f);
void lookup(char * st);
int eqs(char * s1,char * s2);
int peekch();
void label();
void assign();
void cexpr();
union hw expr(int t, int lev);
void num(char ch);
int error(char * s);
void putwd(int w);
void iflush(int a);
void puthex(int n);
void prtwd(int n);
void putoct(int n);
void cinstr();
void getops();
void ins0(struct symtab *sp);
void ins1(struct symtab * sp);
void single(int v);
int less(char * s1, char * s2);
void exchg(int i, int j);
char rgetc();
int getch();
void bflush();
void prsym();
void newline(int a);
void bopen(char * s);
void rwordb();
void sortsym();
void jump1(int v);
void branch(int v);
void ins2(struct symtab *sp);
void shift(int v);
void bext();
void dbl(int v);
void jsr(int v);
void untoke();
void dincld();
void syminit();
void dblk();

FILE* openFileWithEnv(char* filename, const char* mode);


#define USAGE(program_name, retcode) do { \
fprintf(stderr, "USAGE: %s %s\n", program_name, \
"[-h] [-O DIR/] [-2] \n" \
"   -h or --help                        Help: displays this help menu.\n" \
"   -O or --output-directory DIR/       Specify the /-terminated pathname of a directory into which the output binary is to be placed.\n" \
"   -2 or --no-second-pass              Perform only first-pass processing and skip the second pass.\n" \
); \
exit(retcode); \
} while(0)




#endif //HW2_CONSTANTS_H
