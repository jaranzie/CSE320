#include <stdlib.h>

#include "reverki.h"
#include "global.h"
#include "debug.h"


static int checkflag(char* flag, char character);
static int strToInt(char* str);
extern int getAtomArrayLength();
extern int getTermArrayLength();
extern int getRuleArrayLength();

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */

int validargs(int argc, char **argv) {

    //check if adequate args
    if (argc == 1) {
        return -1;
    }
    int currentArg = 1;
    //check help flag
    if (checkflag(*(argv+currentArg),'h')) {
        global_options = HELP_OPTION;
        return 0;
    }
    /* Validate flag */
    if (checkflag(*(argv+currentArg),'v')) {
        currentArg++;
        global_options += VALIDATE_OPTION;
        /*validate and stats */
        if (currentArg < argc && checkflag(*(argv+currentArg),'s')) {
        global_options += STATISTICS_OPTION;
        currentArg++;
    }
        if (currentArg == argc) {return 0;}
        else{return -1;}
    }
    /* rewrite */
    if (checkflag(*(argv+currentArg),'r')) {
        currentArg++;
        global_options += REWRITE_OPTION;
        int stats = 0, trace = 0, lmt = 0;
        while (currentArg < argc) {
            if (currentArg < argc && stats == 0 && checkflag(*(argv+currentArg),'s')) {
                global_options += STATISTICS_OPTION;
                stats = 1;
                currentArg++;
                continue;
            }
            if (currentArg < argc && trace == 0 && checkflag(*(argv+currentArg),'t')) {
                global_options += TRACE_OPTION;
                trace = 1;
                currentArg++;
                continue;
            }
            if (currentArg < argc && lmt == 0 && checkflag(*(argv+currentArg),'l')) {
                currentArg++;
                if(currentArg >= argc) {
                    return -1;
                }
                lmt = 1;
                global_options += LIMIT_OPTION;
                long int limit = (long)strToInt((*(argv+currentArg)));
                if (limit <= 0 || limit > 0x7FFFFFFF) {
                    return -1;
                }
                global_options += (limit << 32);
                currentArg++;
                continue;
            }
            return -1;
        }
        return 0;

    }
    
    
    return -1;
    abort();
}

static int checkflag(char* flag, char character) {
    if (*flag == '-' && *(flag+1) == character && *(flag+2) == '\0') {
        return 1;
    }
    return 0;
}

static int strToInt(char* str) {
    int num = 0;
    char add = *str;
    while (add >= 48 && add <= 57) {
        num = num * 10 + (add-48);
        str++;
        add = *str;
    }
    if (add != '\0') {
        return 0;
    }
    return num;
}
extern int currentAtom;
extern int currentTerm;
extern int currentRule;

void printStats() {
    if(global_options & STATISTICS_OPTION) {
        fprintf(stderr,"Atoms used: %d, free: %d\nTerms used: %d, free: %d\nRules used: %d, free: %d",
               getAtomArrayLength(),(REVERKI_NUM_ATOMS-getAtomArrayLength()),getTermArrayLength(),(REVERKI_NUM_TERMS-getTermArrayLength()),getRuleArrayLength(),(REVERKI_NUM_RULES-getRuleArrayLength()));
    }
}
