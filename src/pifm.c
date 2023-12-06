/*
 *========================================================================== 
 * Copyright 1991, 1992   Avinash Chopde, All Rights Reserved.
 */

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/pifm.c,v 1.2 1996/12/08 03:07:59 avinash Exp $";

#include "itrans.h"
#include "ifm.h"

#define IFMLINELEN_MAX		1024
/* OLD #define IFMLINE_STR		"Comment -I-" */
#define IFMLINE_STR		"itrans:"
#define IFMCOMMENT_STR		"Comment"
#define IFMCOMMENT_CHR		'%'
#define CCS_STR			"CCS"
#define CC_STR			"CC"
#define CCADD_STR		"CCADD"
#define PCC_STR			"PCC"
#define PSF_STR			"POSTSCRIPT"	
#define TEXF_STR		"TEX"
#define TXTF_STR		"TEXT_TTY"	
#define UNICODEF_STR		"TEXT_UNICODE"	
#define HCHAR_STR		"HYPHENCHAR"
#define DEFVOWEL_STR		"DEFAULTVOWEL"
#define NONE_STR		"none"
#define IMPLICIT_STR		"implicit"
#define STARTI_STR		"StartINDIAN"
#define ENDI_STR		"EndINDIAN"

int G_ifm_lineno = 0;

/* ==================================================================== */
/* extract a word from the given char pointer, and increment the pointer.
 * The char pointer is assumed to point to a string as returned
 * by fgets() - terminated by "\n\0"
 */

static int xtract_word(char** cptr, char word[])
{
    int ch, i;

    if (!*cptr)	return FALSE; /* error, NULL pointer */

    if (**cptr == '\n' || **cptr == '\0') return FALSE; /* eoln */

    /* skip spaces */
    ch = *(*cptr)++;
    while (ch == ' ' || ch == '	' || ch == ';') { /* space, tab, semicolon */
	ch = **cptr;
        if (ch == '\n' || ch == '\0')	return FALSE; /* eoln */
	(*cptr)++;
    }

    /* extract word */
    i = 0;
    while (ch != '\n' && ch != ' ' && ch != '	' && ch != ';') {
			/* nl, space, tab, semicolon */
	word[i++] = ch;
	ch = *(*cptr)++;
    }

    word[i] = '\0';
    return i;
}

/* ==================================================================== */
static int get_next_ifm_line(char* iline, int n, FILE* fp)
{
     G_ifm_lineno++;
     if (!fgets(iline, n, fp)) return FALSE; /* EOF */

     /* check for valid line - starts with "Comment -I-" */
     while (strncmp(iline, IFMLINE_STR, strlen(IFMLINE_STR))) {
         G_ifm_lineno++;
         if (!fgets(iline, n, fp)) return FALSE; /* EOF */
     }

     return TRUE;
}

/* ==================================================================== */
/* externally visible function, returns the next IFM token in word, and
 * does some pattern matching
 * Returns the TAG depending on the word (CC, CCS, etc) -see ifm.h
 */
static char S_line[IFMLINELEN_MAX] = "\0";
static char* S_cptr = &S_line[0];

int get_ifm_token(FILE* fp, char* word)
{
    int s;

    s = xtract_word(&S_cptr, word);


    if (!s) {
	do {
	    if (!get_next_ifm_line(S_line, IFMLINELEN_MAX, fp))
		return FALSE; /* EOF */
	    S_cptr = &S_line[strlen(IFMLINE_STR) + 1];
	    word[0] = '\0';
	    s = xtract_word(&S_cptr, word);
	} while (!s);
    }


    /* decipher word */
    if (	   !strcmp(word, STARTI_STR)
		|| !strcmp(word, ENDI_STR)
        	|| !strcmp(word, IFMCOMMENT_STR)
		|| (word[0] == IFMCOMMENT_CHR)) {
		/* dump the rest of the line, is a comment */

	S_line[0] = '\0';
	S_cptr = &S_line[0];
	return COMMENT_IFMTAG;
    }

    if (!strcmp(word, CCS_STR))		return CCS_IFMTAG;
    if (!strcmp(word, CC_STR))		return CC_IFMTAG;
    if (!strcmp(word, CCADD_STR))	return CCADD_IFMTAG;
    if (!strcmp(word, PCC_STR))		return PCC_IFMTAG;
    if (!strcmp(word, PSF_STR))		return PSF_IFMTAG;
    if (!strcmp(word, TEXF_STR))	return TEXF_IFMTAG;
    if (!strcmp(word, TXTF_STR))	return TXTF_IFMTAG;
    if (!strcmp(word, UNICODEF_STR))	return UNICODEF_IFMTAG;
    if (!strcmp(word, NONE_STR))	return NONE_IFMTAG;
    if (!strcmp(word, IMPLICIT_STR))	return IMPLICIT_IFMTAG;
    if (!strcmp(word, HCHAR_STR))	return HCHAR_IFMTAG;
    if (!strcmp(word, DEFVOWEL_STR))	return DEFVOWEL_IFMTAG;

    /* now, check for it being a number:
     * "digits" or "+digits" or "-digits" or "ox+-digits" or "cy+-digits"
     */
    if (isdigit(word[0]))		return NUMBER_IFMTAG;
    if (isdigit(word[1]) && (word[0] == '+' || word[0] == '-'))
	return NUMBER_IFMTAG;
    
    /* if nothing, then default is DORG for Y axis, DCUR for X axis */
    if (isdigit(word[3])) {
	if (!strncmp(word, "ox", 2)) {
	    word[0] = word[1] = ' ';
	    return DORG_IFMTAG;
	}
	if (!strncmp(word, "cy", 2)) {
	    word[0] = word[1] = ' ';
	    return DCUR_IFMTAG;
	}
    }

    /* finally, must be a DNAME_IFMTAG [*-.a-zA-Z0-9]* */

    return DNAME_IFMTAG;
}
/* ==================================================================== */
void reset_pifm()
{
    G_ifm_lineno = 0;
    S_line[0] = '\0';
    S_cptr = &S_line[0];
}
/* ===========================^ pifm.c ^ ============================== */
