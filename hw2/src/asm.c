
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "constants.h"


/*typedef signed short int int16_t;
typedef unsigned char uint8_t;*/

struct symtab symtab[NSYM];

struct symtab *sorteds[NSYM];

struct symtab *hshtab[HSHSIZ];

struct insops {
	int i_mode;
	int i_val;
} insops[2];

int ninsops;
int nodata;
int odata;
int isline;
int lineno;
char *fname;
int lastaddr = -1;
int insaddr;
int nsym;
struct symtab *csym;
int cval = 0;
int ctoke;
char lbuf[LSIZE];
char *lp = lbuf;
FILE *ibuf0;
FILE *ibuf1;
int wchbuf = 0;
int pass1;
char peekc;
int ispc; /* Is there a char is peak buffer */
int peekt;
int ispt; /* Is there a token in peak buffer */
long *dot;
int nerr = 0;
int oradix = 8;
int nowds = 2;
short obuf[19];
int oaddr;
FILE * ofile;
int anyhalf;
jmp_buf env;
union hw halfw;


struct psym {
	char *p_name;
	int p_flags;
	long p_val;
} psym[] = {
	{"tst",	SINST,	ITST},
	{"clr",	SINST,	ICLR},
	{"com",	SINST,	ICOM},
	{"mov",	SINST,	IMOV},
	{"add",	SINST,	IADD},
	{"sub",	SINST,	ISUB},
	{"cmp",	SINST,	ICMP},
	{"and",	SINST,	IAND},
	{"xor",	SINST,	IXOR},
	{"inc",	SINST,	IINC},
	{"dec",	SINST,	IDEC},
	{"neg",	SINST,	INEG},
	{"adc",	SINST,	IADC},
	{"gswd",	SINST,	IGSWD},
	{"nop",	SINST,	INOP},
	{"sin",	SINST,	ISIN},
	{"rswd",	SINST,	IRSWD},
	{"swap",	SINST,	ISWAP},
	{"sll",	SINST,	ISLL},
	{"rlc",	SINST,	IRLC},
	{"sllc",	SINST,	ISLLC},
	{"slr",	SINST,	ISLR},
	{"sar",	SINST,	ISAR},
	{"rrc",	SINST,	IRRC},
	{"sarc",	SINST,	ISARC},
	{"halt",	SINST,	IHALT},
	{"eis",	SINST,	IEIS},
	{"dis",	SINST,	IDIS},
	{"tci",	SINST,	ITCI},
	{"clrc",	SINST,	ICLRC},
	{"setc",	SINST,	ISETC},
	{"jmp",	SINST,	IJMP},
	{"j",	SINST,	IJ},
	{"je",	SINST,	IJE},
	{"jd",	SINST,	IJD},
	{"jsr",	SINST,	IJSR},
	{"jsre",	SINST,	IJSRE},
	{"jsrd",	SINST,	IJSRD},
	{"b",	SINST,	IB},
	{"nop2",	SINST,	INOP2},
	{"bc",	SINST,	IBC},
	{"bnc",	SINST,	IBNC},
	{"bov",	SINST,	IBOV},
	{"bnov",	SINST,	IBNOV},
	{"bze",	SINST,	IBZE},
	{"bnze",	SINST,	IBNZE},
	{"blge",	SINST,	IBLGE},
	{"bllt",	SINST,	IBLLT},
	{"bpl",	SINST,	IBPL},
	{"bmi",	SINST,	IBMI},
	{"beq",	SINST,	IBEQ},
	{"bneq",	SINST,	IBNEQ},
	{"blt",	SINST,	IBLT},
	{"bge",	SINST,	IBGE},
	{"ble",	SINST,	IBLE},
	{"bgt",	SINST,	IBGT},
	{"busc",	SINST,	IBUSC},
	{"besc",	SINST,	IBESC},
	{"bext",	SINST,	IBEXT},
	{"r0",	SREG,	0},
	{"r1",	SREG,	1},
	{"r2",	SREG,	2},
	{"r3",	SREG,	3},
	{"r4",	SREG,	4},
	{"r5",	SREG,	5},
	{"r6",	SREG,	6},
	{"r7",	SREG,	7},
	{"sp",	SREG,	6},
	{"pc",	SREG,	7},
	{".hex",	SDIRECT,	(long)&dhex},
	{".octal",	SDIRECT,	(long)&doctal},
	{".ascii",	SDIRECT,	(long)&dascii},
	{".asciz",	SDIRECT,	(long)&dasciz},
	{".wordb",	SDIRECT,	(long)&dwordb},
	{".word",	SDIRECT,	(long)&dword},
	{".byte",	SDIRECT,	(long)&dbyte},
	{".blk",	SDIRECT,	(long)&dblk},
	{".incld",	SDIRECT,	(long)&dincld},
	{0,0,0}
};

char mapchr[] = {
EOFF,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,
GARBG,	SPACE,	NEWLN,	GARBG,	NEWLN,	GARBG,	GARBG,	GARBG,
GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,
GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,
SPACE,	EXCLA,	DQUOTE,	SHARP,	DOL,	PCNT,	AND,	SQUOTE,
LPARN,	RPARN,	STAR,	PLUS,	COMMA,	MINUS,	LETTER,	SLASH,
DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,
DIGIT,	DIGIT,	COLON,	SEMI,	LESS,	EQUAL,	GREAT,	QUES,
AT,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LBRACK,	BSLASH,	RBRACK,	UPARR,	UNDER,
GRAVE,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LCURL,	VBAR,	RCURL,	TILDE,	GARBG
};

static struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {"output-directory", 1, 0, 'O'},
        {"no-second-pass", 0, 0, '2'},
        {0,0,0,0}
};

static char* TARGET_DIRECTORY = NULL;
static int SECOND_PASS_FLAG = 0;
static int programArgIndex = 0; /* Might need to be 0*/

void orig_main(ac,av)
int ac;
char **av;
{
    if(ac == 1){
        USAGE(av[0],1);
    }
    int c = 0;
    int option_index = 0;
    while ((c= getopt_long(ac,av, "hO:2", long_options, &option_index)) != EOF) {
        switch (c) {
            case 'h':
            {
                USAGE(*av, 0);
                break;
            }
            case '2':
            {
                programArgIndex++;
                SECOND_PASS_FLAG = 1;
                break;
            }
            case 'O':
            {
                if(TARGET_DIRECTORY != NULL) {
                    fprintf(stderr, "ERROR: MUTIPLE OUTPUT DIRECTORIES SPECIFIED\n");
                    USAGE(av[0],1);
                }
                programArgIndex += 2;
                char lastChar;
                char * temp = optarg;
                for(;*temp;temp++) {
                    lastChar = *temp;
                }
                if(lastChar != '/') {
                    fprintf(stderr, "ERROR: OUTPUT DIRECTORY NOT /-TERMINATED\n");
                    USAGE(av[0],1);
                }
                size_t size = temp-optarg;
                char* tempDir = malloc(size+1); /*mb add one*/
                TARGET_DIRECTORY = tempDir;
                temp = optarg;
                for(;*temp;temp++) {
                    *tempDir = *temp;
                    tempDir++;
                }
                *temp++ = '\0';
                break;
            }
            case '?':
            {
                fprintf(stderr, "ERROR: UNKNOWN FLAGE SPECIFIED\n");
                USAGE(*av, 1);
            }
            default:
                break;
        }
    }
	int i;
	char **a;
	i = ac;
	av = av + programArgIndex;
	a = av+1;
	syminit();
	lp = lbuf;
	*lp = -1;
	while(--i != programArgIndex) {
		lineno = 0;
		fname = a[0];
		if ((ibuf0 = openFileWithEnv(a[0], "r")) == NULL) {
			nerr++;
			fprintf(stderr,"Can't open %s\n",a[0]);
			a++;
			continue;
		}
		setjmp(env);
		while(line() == 0){
            iflush(0);
        }
		iflush(0);
		fclose(ibuf0);
		a++;
	}
	if(undefined() || nerr) exit(1);
    if(SECOND_PASS_FLAG) {
        exit(0);
    }
	pass1++;
	DOT = 0;
	nowds = 2;
	oaddr = 0;
	i = ac;
	a = av+1;
	lp = lbuf;
	*lp = -1;
	anyhalf = 0;
	bopen(a[0]);
	while(--i != programArgIndex) {
		lineno = 0;
		fname = a[0];
		ibuf0 = openFileWithEnv(a[0], "r");
		a++;
		setjmp(env);
		while(line() == 0) {
            iflush(0);
        }
		iflush(0);
		fclose(ibuf0);
	}
	bflush();
	prsym();
	fflush(stdout);
	fflush(stderr);
    fclose(ofile);
    free(TARGET_DIRECTORY);
}

/* Returns a peaked token, or reads a new token*/
int token()
{
	if(ispt) {
		ispt = 0;
		ctoke = peekt;
		return(peekt);
	}
	ctoke = rtoken();
	return(ctoke);
}


/* Peeks token*/
int peektoke()
{
	if(ispt) return(peekt);
	ispt = 1;
	peekt = rtoken();
	return(peekt);
}


/* Sends current token to peekt */
void untoke()
{
	ispt = 1;
	peekt = ctoke;
}


/* Reads a token*/
int rtoken()
{
	int c;
	char tbuf[9];
    char *tp = tbuf;
loop:
	switch(mapchr[c = getch()]) {

	case SPACE:
		goto loop;
	case NEWLN:
		return(TNEWLN);
	case EOFF:
		return(TEOFF);
	case EXCLA:
		return(TEXCLA);
	case DQUOTE:
		return(TDQUO);
	case SHARP:
		return(TSHARP);
	case DOL:
		return(TDOL);
	case PCNT:
		return(TPCNT);
	case AND:
		return(TAND);
	case SQUOTE:
		if(mapchr[c = getch()] == EOFF) error("x");
		cval = c;
		return(TCONST);
	case LPARN:
		return(TLPARN);
	case RPARN:
		return(TRPARN);
	case STAR:
		return(TSTAR);
	case PLUS:
		return(TPLUS);
	case COMMA:
		return(TCOMMA);
	case MINUS:
		return(TMINUS);
	case SLASH:
		return(TSLASH);
	case DIGIT:
		num(c);
		return(TCONST);
	case COLON:
		return(TCOLON);
	case SEMI:
		while(mapchr[c = getch()] != NEWLN) {
			if(mapchr[c] == EOFF) error("x");
		}
		return(TNEWLN);
	case LESS:
		return(TLESS);
	case EQUAL:
		return(TEQUAL);
	case GREAT:
		return(TGREAT);
	case QUES:
		return(TQUES);
	case AT:
		return(TAT);
	case LETTER:
		*tp++ = c;
		while(((c = mapchr[peekch()]) == LETTER) || (c == DIGIT)) {
			c = getch();
			if(tp == &tbuf[8]) *tp = 0;
			else *tp++ = c;
		}
		*tp = 0;
		lookup(tbuf);
		return(TIDENT);
	case LBRACK:
		return(TLBRAK);
	case BSLASH:
		return(TBSLA);
	case RBRACK:
		return(TRBRAK);
	case UPARR:
		return(TUPARR);
	case UNDER:
		return(TUNDER);
	case GRAVE:
		return(TGRAVE);
	case LCURL:
		return(TLCURL);
	case VBAR:
		return(TVBAR);
	case RCURL:
		return(TRCURL);
	case TILDE:
		return(TTILDE);
	case GARBG:
		error("g");
	}
    return -1;
}


/* Returns a peeked char or reads a new one*/
int getch()
{
	if(ispc) {
		ispc = 0;
		return(peekc);
	}
	else return(rgetc());
}


/*peeks char*/
int peekch()
{
	if(ispc) {
		return(peekc);
	}
	else {
		ispc = 1;
		return(peekc = rgetc());
	}
}


/*Gets a character*/
char rgetc()
{
	char c;
	if(*lp != -1) return(*lp++);
	else {
		lineno++;
		lp = lbuf;
		isline++;
		do {
            if (lp == &lbuf[LSIZE - 1]) {
                lp = lbuf;
                error("l");
            }
            c = (char) getc(wchbuf ? ibuf1 : ibuf0);
            if (c < 0) {
                if (wchbuf) {
                    fclose(ibuf1);
                    wchbuf = 0;
                    c = (char) getc(ibuf0);
                    if (c < 0){ c = 0;}
                } else {
                    c = 0;
                }
            }
			*lp++ = c;
		} while((c != 0) && (c != '\n'));
		*lp = -1;
		lp = lbuf;
		return(*lp++);
	}
}

/*prints current line buffer to stream f*/
void lflush(FILE *f)
{
	char *rlp;
	isline = 0;
	rlp = lbuf;
	while(*rlp != -1) { /*changed from -1*/
        if(*rlp == '\0') {
            rlp++;
            continue;
        }
        fprintf(f, "%c", *rlp++);
        //fflush(stdout);
    }
}


/* Looks in hashtable for symbol,*/
void lookup(char *st)
{
	struct symtab *sp,*ssp;
	int hash,hsh;
	char *cp;
	hash = 0;
	cp = st;
	while(*cp) hash += *cp++;
	hsh = hash % HSHSIZ;
	if(hshtab[hsh] == 0) {
		if(++nsym >= NSYM) error("o");
		sp = &symtab[nsym-1];
		hshtab[hsh] = sp;
		goto gotone;
	}
	sp = hshtab[hsh];
	do {
		ssp = sp;
		if(hash != sp->s_hash) {
			continue;
		}
		if(eqs(st,sp->s_name)) {

			csym = sp;
			return;
		}
	} while((sp = sp->s_link));
	if(++nsym >= NSYM) error("o");
	sp = &symtab[nsym-1];
	ssp->s_link = sp;
gotone:	sp->s_link = 0;
	cp = sp->s_name;
	while(*st) *cp++ = *st++;
	csym = sp;
	sp->s_flags |= SIDENT;
	sp->s_hash = hash;
	/*return;*/
}


/* Compares two strings */
int eqs(char *s1, char *s2)
{
	while(*s1 == *s2++) if(*s1++ == 0) return(1);
	return(0);
}

int line()
{
	struct symtab *sp;
	long (*dirp)();
	insaddr = DOT;
loop:	switch(token()) {

	case TIDENT:
		sp = csym;
		if(sp->s_flags & SIDENT) {
			switch(peektoke()) {
			case TCOLON:
				token();
				label();
				goto loop;
			case TEQUAL:
				token();
				assign();
				return(0);
			default:
				cexpr();
				return(0);
			}
		} else if(sp->s_flags & SDIRECT) {
			dirp = (long (*)()) (long)sp->s_val;
			(*dirp)();
			return(0);
		} else if(sp->s_flags & SINST) {
			cinstr();
			return(0);
		}
	case TCONST:
	case TMINUS:
		cexpr();
		return(0);
	case TEOFF:
		return(1);
	case TNEWLN:
		return(0);
	default:
		untoke();
		error("g");
	}
    return -1;
}


/* Used for assmebly labels: */
void label()
{
	struct symtab *sp;
	sp = csym;
	if((!pass1) && (sp->s_flags & SDEF)) error("m");
	else {
		sp->s_flags |= SDEF;
		sp->s_val = DOT;
	}
}

void assign()
{
	struct symtab *sp;
	sp = csym;
	sp->s_val = expr(0,0).val;
	sp->s_flags |= SDEF;
	if(sp->s_flags & SDOT) bflush();
	newline(0);
}

void cexpr()
{
	putwd(expr(1,0).val);
	newline(0);
}
/* lev stores level of recursive call*/
union hw expr(int t, int lev)
{
	int toke;
	struct symtab *sp;
	int arg, nargs, nops, op;
    arg = 0;
    union hw val;
    val.val = 0;
	arg = nargs = nops = op = 0;
	if(t == 0) toke = token();
	else toke = ctoke;
loop:
	switch(toke) {

	case TLPARN:
		val = expr(0,1);
		if(token() != TRPARN) error("e");
		goto caseident;
	case TCONST:
		val.val = (short) cval;
        cval = 0;
		goto caseident;
	case TIDENT:
		sp = csym;
		if((sp->s_flags & SIDENT) == 0)
			error("e");
		val.val = (short) sp->s_val;
caseident:	if(nops == 0) {
			if(nargs != 0)
				error("e");
			arg = val.val;
			nargs++;
			toke = token();
			goto loop;
		}
		if(nargs == 0) {
			switch(op) {

			case TMINUS:
				arg = -val.val;
				break;
			case TTILDE:
				arg = ~val.val;
				break;
			default:
				error("e");
			}
		} else {
			switch(op) {

			case TPLUS:
				arg = arg+val.val;
				break;

			case TMINUS:
				arg = arg-val.val;
				break;

			case TSTAR:
				arg = arg*val.val;
				break;

			case TSLASH:
				arg = arg/val.val;
				break;

			case TPCNT:
				arg = arg%val.val;
				break;

			case TAND:
				arg = arg&val.val;
				break;

			case TVBAR:
				arg = arg|val.val;
				break;

			case TGREAT:
				arg = arg>>val.val;
				break;

			case TLESS:
				arg = arg<<val.val;
				break;

			default:
				error("e");
			}
		}
		nops = 0;
		nargs = 1;
		toke = token();
		goto loop;
	case TMINUS:
	case TTILDE:
	case TPLUS:
	case TSTAR:
	case TSLASH:
	case TPCNT:
	case TAND:
	case TVBAR:
	case TGREAT:
	case TLESS:
		if(nops != 0) error("e");
		if(nargs == 0) {
			if((toke != TTILDE) && (toke != TMINUS)) error("e");
		}
		op = toke;
		nops++;
		toke = token();
		goto loop;
	default:
		if((nops != 0) || (nargs == 0))
			error("e");
		untoke();
        union hw arg1;
        arg1.val = (short)arg;
		return(arg1);
	}
}

void num(char ch)
{
	int val;
	char buf[16];
	char *bp;
	int radix;
	radix = 8;
	bp = buf;
	*bp++ = ch;
	while(mapchr[peekch()] == DIGIT) {
		if(bp == &buf[15]) error("c");
		*bp++ = (char)getch();
	}
	if(peekch() == '.') {
		getch();
		radix = 10;
	}
    *bp++ = '\0';
	bp = buf;
	val = 0;
	while(*bp) {
		val *= radix;
		val += *bp++ - '0';
	}
	cval = val;
}

int error(char *s)
{
	nerr++;
	fprintf(stderr,"%s:%d: *** %s\n",fname,lineno,s);
	newline(1);
	iflush(1);
	longjmp(env,1);
}

void prsym()
{
	struct symtab *sp;
	int i,j;
	sortsym();
	printf("\n\nSymbol Table\n\n");
    i = 0;
	j = 0;
	while(j < nsym) {
		sp = sorteds[j];
		if(((sp->s_flags & SIDENT) == 0) || (sp->s_flags & SDOT)) {
			j++;
			continue;
		}
		printf("%-8s =\t", sp->s_name);
		prtwd(sp->s_val);
		if(!(sp->s_flags & SDEF)) putchar('U');
		putchar('\t');
		if((++i % 3) == 0) putchar('\n');
		j++;
	}
	putchar('\n');
}

void syminit()
{
	struct symtab *sp;
	struct psym *pp;
	lookup(".");
	sp = csym;
	sp->s_flags |= SDOT;
	dot = &(sp->s_val);
	pp = &psym[0];
	while(pp->p_name) {
		lookup(pp->p_name);
		sp = csym;
		sp->s_flags = pp->p_flags;
		sp->s_val = pp->p_val;
		pp++;
	}
}

void putwd(int w)

{
	if(anyhalf) error("h");
	obuf[nowds++] = w;
	odata = w;
	nodata++;
	DOT++;
	if(nowds == 18){
        bflush();
    }
	iflush(0);
}

void iflush(int a)
{
	if(pass1 == 0) {
		if(a) {
			lflush(stderr);
		}
		nodata = 0;
		return;
	}
	if((nodata == 0) && (isline == 0))
		return;
	if(insaddr != lastaddr) {
		lastaddr = insaddr;
		prtwd(insaddr);
	}
	putchar('\t');
	if(nodata == 0) {
		putchar('\t');
		lflush(stdout);
	} else {
		prtwd(odata);
		putchar('\t');
		if(isline) lflush(stdout);
		else putchar('\n');
		nodata = 0;
	}
}

void bflush()
{
	int i,chksum;
	if((nowds == 2) || (pass1 == 0)) {
		nowds = 2;
		oaddr = DOT;
		return;
	}
	obuf[0] = oaddr;
	obuf[1] = nowds-1;
	chksum = 0;
	for(i = 0; i < nowds; i++) chksum += obuf[i];
	obuf[nowds] = -chksum;
    fwrite(obuf, 2, nowds+1, ofile);
    fflush(ofile);
	/*write(ofile,obuf,(nowds+1)*2);*/
	oaddr = DOT;
	nowds = 2;
}

void puthex(int n)
{
	int count;
	int i;
	count = 4;
	while(count--) {
		i = ((n & 0170000) >> 12);
		i &= 017;
		if(i > 9) putchar(i - 10 + 'A');
		else putchar(i + '0');
		n <<= 4;
	}
}

void prtwd(int n)
{
	if(oradix == 16) puthex(n);
	else putoct(n);
    //fflush(stdout);
}

void putoct(int n)
{
    short m = (short)n;
	int count;
	count = 5;
	putchar(m < 0 ? '1' : '0');
	while(count--) {
		putchar(((m & 070000) >> 12) + '0');
		m <<= 3;
	}
}

void cinstr()
{
	struct symtab *sp;
	sp = csym;
	getops();
	if(ninsops == 0) ins0(sp);
	else if(ninsops == 1) ins1(sp);
	else if(ninsops == 2) ins2(sp);
	else error("i");
	newline(0);
}

void getops()
{
	struct insops *ip;
	struct symtab *sp;
	ninsops = 0;
    insops[0].i_val = 0;
    insops[1].i_val = 0;
    insops[0].i_mode = 0;
    insops[1].i_mode = 0;


loop:
	ip = &insops[ninsops > 1 ? 1 : ninsops];
	switch(token()) {

	case TIDENT:
		sp = csym;
		if(sp->s_flags & SREG) {
			ip->i_mode = REGMOD;
			ip->i_val = sp->s_val;
			break;
		}
		ip->i_mode = IMMMOD;
		ip->i_val = expr(1,0).val;
		break;
	case TAT:
		switch(token()) {

		case TIDENT:
			sp = csym;
			if(sp->s_flags & SREG) {
				ip->i_mode = DEFREG;
				ip->i_val = sp->s_val;
				if(peektoke() == TPLUS) {
					token();
					if((ip->i_val < 4) || (ip->i_val > 6))
						error("r");
				}
				break;
			}
			ip->i_mode = DEFIMM;
			ip->i_val = expr(1,0).val;
			break;
		case TMINUS:
			token();
			sp = csym;
			if((ctoke == TCONST) || ((ctoke == TIDENT) && (sp->s_flags != SREG)))
				goto caseconst;
			ip->i_mode = DEFREG;
			ip->i_val = sp->s_val;
			if(ip->i_val != 6) error("r");
			break;
		case TCONST:
		case TTILDE:
caseconst:		ip->i_mode = DEFIMM;
			ip->i_val = expr(1,0).val;
			break;
		default:
			untoke();
			error("i");
		}
		break;
	case TCONST:
	case TMINUS:
	case TTILDE:
		ip->i_mode = IMMMOD;
		ip->i_val = expr(1,0).val;
		break;
	case TNEWLN:
		untoke();
		return;
	default:
		error("i");
	}
	ninsops++;
	switch(peektoke()) {

	case TCOMMA:
		token();
		goto loop;
	case TNEWLN:
		return;
	default:
		error("i");
	}
}

void ins0(struct symtab *sp)
{
	if((sp->s_val == INOP) || (sp->s_val == ISIN)) {
		putwd((short)(064 | (sp->s_val == INOP ? 0 : 02)));
		return;
	} else
	if(sp->s_val > 7) {error("i");}
    putwd((short)(sp->s_val&07));
}


void ins1(struct symtab *sp)
{
	int v;
	int addr;
	v = sp->s_val;
	if(v < 8) error("i");
	else if((v >= 16) && (v <= 19)) jump1(v);
	else if((v >= 32) && (v <= 47)) branch(v);
	else if((v >= 73) && (v <= 79)) single(v);
	else if((v == ITST) || (v == ICLR)) {
		if((insops[0].i_mode & REGMOD) == 0)
			error("i");
		addr = insops[0].i_val;
		addr = (addr << 3) | addr;
		if(v == ITST) {
            putwd((short)(0200 | addr));
        }
		else {
            putwd((short)(0700 | addr));
        }
		return;
	}
	else if(v == IJMP) {
		if((insops[0].i_mode & REGMOD) == 0) {
			branch(IB);
			return;
		}
		addr = insops[0].i_val;
		putwd((short)(0200 | (addr << 3) | 07));
	}
	else error("i");
}
void ins2(struct symtab *sp)
{
	int v;
	v = sp->s_val;
	if((v >= 8) && (v <= 15)) shift(v);
	else if((v >= 66) && (v <= 71)) dbl(v);
	else if(v == IBEXT) bext();
	else if((v >= 20) && (v <= 22)) jsr(v);
	else error("i");
}

void jump1(int v)
{
	int addr;
	if(insops[0].i_mode != IMMMOD) error("i");
	addr = insops[0].i_val;
	putwd(04);
	putwd(01400 | (((addr & 0176000) >> 8) & 0377) | (v & 03));
	putwd(addr & 01777);
}

void branch(int v)
{
	int pc,addr,sign;
	pc = DOT+2;
	if(insops[0].i_mode != IMMMOD) error("i");
	addr = insops[0].i_val;
	sign = addr > pc ? 0 : 040;
	putwd(01000 | sign | (v & 017));
	if(sign)
		putwd(~(addr - pc));
	else
		putwd(addr - pc);
}
void single(int v)
{
	int mode, reg;
	reg = insops[0].i_val;
	mode = insops[0].i_mode;
	if(mode == REGMOD) {
		switch(v) {
		case IINC:
		case IDEC:
		case ICOM:
		case INEG:
		case IADC:
			putwd(((v & 07) << 3) | reg);
			return;
		case IGSWD:
			if(reg > 3) error("r");
			putwd(060 | reg);
			return;
		case IRSWD:
			putwd(070 | reg);
			return;
		default:
			error("i");
		}
	} else error("i");
}


void newline(int a)
{
	int c;
	if((!a) && (peektoke() != TNEWLN))
		error("n");
	if(ispt && (peekt == TNEWLN)) {
		token();
		return;
	}
	while((mapchr[c = getch()] != NEWLN) && (mapchr[c] != EOFF));
	/*return;*/
}

void shift(int v)
{
	int mode1, val1, mode2, val2;
	mode1 = insops[0].i_mode;
	val1 = insops[0].i_val;
	mode2 = insops[1].i_mode;
	val2 = insops[1].i_val;
	if((mode1 != IMMMOD) || (val1 <= 0) || (val1 >= 3))
		error("i");
	if((mode2 != REGMOD) || (val2 > 3))
		error("i");
	putwd(0100 | ((v & 07) << 3) | ((val1 - 1) << 2) | val2);
}

void dbl(int v)
{
	int mode1, val1, mode2, val2;
	mode1 = insops[0].i_mode;
	val1 = insops[0].i_val;
	mode2 = insops[1].i_mode;
	val2 = insops[1].i_val;
	if((mode1 == REGMOD) && (mode2 == REGMOD)) {
		putwd(((v & 07) << 6) | (val1 << 3) | val2);
		return;
	}
	if(mode1 == REGMOD) {
		if(v != IMOV) error("i");
		if(mode2 == IMMMOD) mode1 = 7;
		else if(mode2 == DEFIMM) mode1 = 0;
		else {
			if((val2 == 7) || (val2 == 0))
				error("r");
			mode1 = val2;
		}
		putwd(01100 | (mode1 << 3) | val1);
		if((mode1 == 0) || (mode1 == 7))
			putwd(val2);
		return;
	}
	if(mode2 != REGMOD) error("i");
	if(mode1 == IMMMOD) mode2 = 7;
	else if(mode1 == DEFIMM) mode2 = 0;
	else {
		if((val1 == 0) || (val1 == 7))
			error("r");
		mode2 = val1;
	}
	putwd(01000 | ((v & 07) << 6) | (mode2 << 3) | val2);
	if((mode2 == 0) || (mode2 == 7))
		putwd(val1);
}

void bext()
{
	int mode1, val1, mode2, val2;
	mode1 = insops[0].i_mode;
	val1 = insops[0].i_val;
	mode2 = insops[1].i_mode;
	val2 = insops[1].i_val;
	if((mode1 != IMMMOD) || (val1 < 0) || (val1 > 15) || (mode2 != IMMMOD))
		error("i");
	mode1 = DOT+2;
	mode2 = val2 > mode1 ? 0 : 040;
	putwd(01020 | mode2 | val1);
	if(mode2)
		putwd(~(val2 - mode1));
	else
		putwd(val2 - mode1);
}

void jsr(int v)
{
	int mode1, val1, mode2, val2;
	mode1 = insops[0].i_mode;
	val1 = insops[0].i_val;
	mode2 = insops[1].i_mode;
	val2 = insops[1].i_val;
	if((mode1 != REGMOD) || (mode2 != IMMMOD) || (val1 < 4) || (val1 == 7))
		error("i");
	putwd(04);
	putwd((((val1 - 4) << 8) | (((val2 & 0176000) >> 8) & 0377) | (v & 03)));
	putwd(val2 & 01777);
}

int undefined()
{
	struct symtab *sp;
	int anyund;
	sp = &symtab[0];
	while(sp < &symtab[nsym]) {
		if((sp->s_flags & SIDENT) &&
		  !(sp->s_flags & SDOT) &&
		  !(sp->s_flags & SDEF)) {
			if(!anyund) fprintf(stderr,"Undefined Symbols\n");
			fprintf(stderr,"%-8s\n",sp->s_name);
			anyund++;
		}
		sp++;
	}
	return(anyund);
}

void dhex()
{
	oradix = 16;
	newline(0);
}

void doctal()
{
	oradix = 8;
	newline(0);
}



void dascii()
{
	char c;
	char sep;
	do {
		c = mapchr[(int)(sep = (char)peekch())];
		if((c == NEWLN) || (c == EOFF))
			error("d");
		getch();
	} while(c == SPACE);
	while((c = (char)getch()) != sep) {
		if((mapchr[(int)c] == NEWLN) || (mapchr[(int)c] == EOFF))
			error("d");
		if(anyhalf) {
			halfw.bytes.lobyte = c;
			anyhalf = 0;
			putwd(halfw.val);
		} else {
			halfw.bytes.hibyte = c;
			anyhalf++;
		}
	}
	newline(0);
}

void dasciz()
{
	dascii();
	if(anyhalf != 0) {
		halfw.bytes.lobyte = 0;
		anyhalf = 0;
		putwd(halfw.val);
	} else {
		halfw.bytes.hibyte = 0;
		anyhalf++;
	}
}

/* rwordb + newline */
void dwordb()
{
	rwordb();
	newline(0);
}

void dword()
{
	int toke;
	do {
		putwd(expr(0,0).val);
		toke = token();
	} while(toke == TCOMMA);
	untoke();
	newline(0);
}

void dbyte()
{
	int toke;
	do {
		toke = expr(0,0).val & 0377;
		if(anyhalf) {
			halfw.bytes.lobyte = (char) toke;
			anyhalf = 0;
			putwd(halfw.val);
		} else {
			halfw.bytes.hibyte = (char) toke;
			anyhalf++;
		}
		toke = token();
	} while(toke == TCOMMA);
	untoke();
	newline(0);
}

void dblk()
{
	int n;
	n = expr(0,0).val;
	newline(0);
	while(n--) putwd(0);
}

void rwordb()
{
	if(anyhalf) {
		halfw.bytes.lobyte = 0;
		anyhalf = 0;
		putwd(halfw.val);
	}
}

void sortsym()
{
	register int i,j;
	for(i = 0; i < nsym; i++)
		sorteds[i] = &symtab[i];
	for(i = 0; i < nsym-1; i++)
		for(j = i+1; j < nsym; j++) {
			if(less(sorteds[j]->s_name,
				sorteds[i]->s_name))
				exchg(i,j);
			}
}


/*Return int type*/
int less(char * s1, char * s2)
{
	register char *rs1, *rs2;
	rs1 = s1;
	rs2 = s2;
	while(*rs1 == *rs2) {
		if(*rs1++ == 0) return(0);
		rs2++;
	}
	return(*rs1 < *rs2 ? 1 : 0);
}

void exchg(int i, int j)
{
	struct symtab *sp;
    struct symtab **si, **sj;
    /*int *si,*sj;*/
	si = &sorteds[i];
	sj = &sorteds[j];
	sp = *si;
	*si = *sj;
	*sj = sp;
}

void dincld()
{
	char fnbuf[32], *fn, c;
	do {
		c = (char)getch();
	} while(mapchr[(int)c] == SPACE);
	fn = fnbuf;
	while(mapchr[(int)c] != EOFF && mapchr[(int)c] != NEWLN) {
		*fn++ = c;
		c = (char)getch();
	}
	*fn = 0;
	if((ibuf1 = openFileWithEnv(fnbuf, "r")) == NULL) {
		nerr++;
		error("Can't include");
		return;
	}
	wchbuf = 1;
}

FILE* openFileWithEnv(char* filename, const char* mode) {
    char* path = getenv("ASM_INPUT_PATH");
    if(path == NULL) {
        return fopen(filename, mode);
    }
    else {
        int fileNameLength = 0;
        char* fileNamePointer = filename;
        for(;*fileNamePointer;fileNamePointer++) {
            fileNameLength++;
        }
        while(*path != ' ' && *path != '\0') { /*Possily change comparison */
            int length = fileNameLength;
            char* pathPointer = path;
            for(;*pathPointer != ':' && *pathPointer !='\0';pathPointer++) {
                length++;
            }
            char filePath[length + 5];
            int index = 0;
            for(char*c = path; c < pathPointer; c++) {
                filePath[index] = *c;
                index++;
            }
            for(char*c = filename; *c; c++) {
                filePath[index] = *c;
                index++;
            }
            filePath[index] = '\0';
            FILE* fp = fopen(filePath, mode);
            if(fp != NULL) {
                return fp;
            }
            else {
                path = pathPointer + 1;
            }
        }
        return fopen(filename, mode);
    }
}

void bopen(char *s)
{
    if(TARGET_DIRECTORY == NULL) {
        char st[64];
        char *sp;
        sp = st;
        while(((*sp++ = *s) != '.') && *s) s++; /* maybe * before s idk why though*/
        if(!*s) *sp++ = '.';
        *sp++ = 'b';
        *sp++ = 'i';
        *sp++ = 'n';
        *sp++ = '\0';
        ofile = fopen(st,"w");
    }
    else {
        char* path = TARGET_DIRECTORY;
        int fileNameLength = 0;
        char* filename = s;
        char* fileNamePointer = filename;
        for(;*fileNamePointer;fileNamePointer++) {
            fileNameLength++;
        }
            int length = fileNameLength;
            char* pathPointer = path;
            for(;*pathPointer !='\0';pathPointer++) {
                length++;
            }
            char filePath[length + 5];
            int index = 0;
            for(char*c = path; c < pathPointer; c++) {
                filePath[index] = *c;
                index++;
            }
            for(char*c = filename; *c !='.'; c++) {
                filePath[index] = *c;
                index++;
            }
            filePath[index] = '.';
            filePath[index+1] = 'b';
            filePath[index+2] = 'i';
            filePath[index+3] = 'n';
            filePath[index+4] = '\0';
        FILE* fp = fopen(filePath, "w");
        if(fp != NULL) {
            ofile = fp;
        }
    }

}
