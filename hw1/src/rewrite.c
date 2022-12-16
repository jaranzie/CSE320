#include <stdlib.h>
#include <stdio.h>

#include "debug.h"
#include "reverki.h"
#include "global.h"

void printLevel(REVERKI_TERM * term);
void printRulesAndSubs(REVERKI_RULE * head, REVERKI_SUBST * subs);
int getRuleListLength(REVERKI_RULE *rule_list);
void recycleSubs(REVERKI_SUBST* subs);
void checkRewrite();
static int currentLevel = -1;
static int rewrites = 0;

/**
 * @brief  This function rewrites a term, using a specified list of rules.
 * @details  The specified term is rewritten, using the specified list of
 * rules, until no more rewriting is possible (i.e. the result is a
 * "normal form" with respect to the rules).
 * Each rewriting step involves choosing one of the rules, matching the
 * left-hand side of the rule against a subterm of the term being rewritten,
 * and, if the match is successful, returning a new term obtained by
 * replacing the matched subterm by the term obtained by applying the
 * matching substitution to the right-hand side of the rule.
 *
 * The result of rewriting will in general depend on the order in which
 * the rules are tried, as well as the order in which subterms are selected
 * for matching against the rules.  These orders can affect whether the
 * rewriting procedure terminates, as well as what final term results from
 * the rewriting.  We will use a so-called "leftmost-innermost" strategy
 * for choosing the order in which rewriting occurs.  In this strategy,
 * subterms are recursively rewritten, in left-to-right order, before their
 * parent term is rewritten.  Once no further rewriting is possible for
 * subterms, rewriting of the parent term is attempted.  Since rewriting
 * of the parent term affects what subterms there are, each time the parent
 * term is changed rewriting is restarted, beginning again with recursive
 * rewriting of the subterms.  Rules are always tried in the order in which
 * they occur in the rule list: a rule occurring later in the list is
 * never used to rewrite a term unless none of the rules occurring earlier
 * can be applied.
 *
 * @param rule_list  The list of rules to be used for rewriting.
 * @param term  The term to be rewritten.
 * @return  The rewritten term.  This term should have the property that
 * no further rewriting will be possible on it or any of its subterms,
 * using rules in the specified list.
 */
REVERKI_TERM *reverki_rewrite(REVERKI_RULE *rule_list, REVERKI_TERM *term) {
    currentLevel++;
    printLevel(term);
    if(term->type != REVERKI_PAIR_TYPE) {
        REVERKI_SUBST subs = NULL;
        REVERKI_RULE* tempRule = rule_list;
        int i;
        while(tempRule != NULL) {
            i = reverki_match(tempRule->lhs,term,&subs);
            if(i != 0) {
                printRulesAndSubs(tempRule,&subs);
                checkRewrite();
                REVERKI_TERM * newTerm = reverki_apply(subs,tempRule->rhs);
                printLevel(newTerm);
                recycleSubs(&subs);
                currentLevel--;
                return newTerm;
            }
            tempRule = tempRule->next;
        }
        tempRule = rule_list;
        while(tempRule != NULL) {
            if(!reverki_compare_term(tempRule->lhs,term)) {
                printLevel(term);
                printRulesAndSubs(tempRule,&subs);
                currentLevel--;
                checkRewrite();
                return tempRule->rhs;
            }
            tempRule = tempRule->next;
        }
        currentLevel--;
        return term;
    }

    REVERKI_TERM * lastTerm = NULL;
    while(lastTerm != term) {
        lastTerm = term;
        if(term->type == REVERKI_PAIR_TYPE) {
            REVERKI_TERM * left = reverki_rewrite(rule_list, term->value.pair.fst);
            REVERKI_TERM * right = reverki_rewrite(rule_list, term->value.pair.snd);
            if(left != term->value.pair.fst || right != term->value.pair.snd) {
                term = reverki_make_pair(left,right);
            }
        }
        int lengthRuleList = getRuleListLength(rule_list);
        REVERKI_RULE* rulePointer = rule_list;
        int numRulesWorking = lengthRuleList;
        while(numRulesWorking > 0) {
            if(rulePointer == rule_list) {
                numRulesWorking = lengthRuleList;
            }
            REVERKI_SUBST subs = NULL;
            int i = reverki_match(rulePointer->lhs,term,&subs);
            if (i != 0) {
                checkRewrite();
                printLevel(term);
                printRulesAndSubs(rulePointer,&subs);
                term = reverki_apply(subs,rulePointer->rhs);
                printLevel(term);
                recycleSubs(&subs);
            }
            else {
                numRulesWorking--;
            }
            if(rulePointer->next != NULL) {
                rulePointer = rulePointer->next;
            }
            else {
                rulePointer = rule_list;
            }
        }
    }
    currentLevel--;
    return term;
}

void printLevel(REVERKI_TERM * term) {
    if (global_options & TRACE_OPTION) {
        for (int i = 0; i < currentLevel; i++) {
            fputc('.', stderr);
        }
        reverki_unparse_term(term, stderr);
        fputc('\n', stderr);
    }
}

void printRulesAndSubs(REVERKI_RULE * head, REVERKI_SUBST * subs) {
    if (global_options & TRACE_OPTION) {
        /*Trace*/
        fprintf(stderr,"==> rule: ");
        reverki_unparse_rule(head,stderr);
        fprintf(stderr, ", subst: ");
        REVERKI_SUBST tempSubst = *subs;
        while(tempSubst != NULL) {
            reverki_unparse_rule(tempSubst,stderr);
            fputc(' ',stderr);
            tempSubst = tempSubst->next;
        }
        fprintf(stderr, " .");
        fputc('\n', stderr);
    }
}

int getRuleListLength(REVERKI_RULE *rule_list) {
    int lengthRuleList = 0;
    REVERKI_RULE* tempRule = rule_list;
    while(tempRule != NULL) {
        tempRule = tempRule->next;
        lengthRuleList++;
    }
    return lengthRuleList;
}

void checkRewrite() {
    rewrites++;
    if((global_options & LIMIT_OPTION) && rewrites > (global_options >> 32)) {
        fprintf(stderr,"LIMIT EXCEEDED: ABORTING\n");
        abort();
    }
}
