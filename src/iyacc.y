%{
/*
 *========================================================================== 
 * Copyright 1991-1992 Avinash Chopde, All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Avinash Chopde not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * Avinash Chopde makes no representations about the suitability of this
 * software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * AVINASH CHOPDE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL AVINASH CHOPDE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Author:  Avinash Chopde
 *
 */

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/iyacc.y,v 1.1.1.1 1996/11/29 01:45:34 avinash Exp $";

#include <stdio.h>
#include "itrans.h"

int G_lineno = 1; /* GLOBAL var: used by lex to indicate current line number */

#define WORD_LEN	511

/* Define a struct to keep track of the components of the letter, and
 * the text representation of the word being parsed.
 */
static struct {
    char word[WORD_LEN+1];
    int n; /* index of current akshara in this word 0 .. n */
    int curr_letter[WORD_LEN+1]; /* collect tokens regarding this letter here */
    int nolig[WORD_LEN+1];/*collect nolig tokens here (see itrans.h, letter_t)*/
    int curr_letter_n; /* number of tokens in curr_letter */
    int curr_type; /* type of letter - special, vowel, cons, etc */
    int curr_spec; /* accent or special character for this letter */
} S_word;

char G_endprev_str[LINELEN]; /* used by ilex.l to  pass string to
			      * process in case of the ENDPREV_TOK
			      */

#define ADD_TOKEN(t) {	\
    if (S_word.curr_letter_n < WORD_LEN) { \
        S_word.nolig[S_word.curr_letter_n] = FALSE;	\
        S_word.curr_letter[S_word.curr_letter_n++] = t;	\
        strcat(S_word.word, yytext);			\
    } else {	\
        yyerror("Input error: too many letters in word");    \
        S_init_word(); \
    } \
}

#define ADD_TOKEN_TYPE(t, typ) {			\
    if (typ == SPECIAL_TYPE) {				\
        S_word.curr_spec = t;				\
        strcat(S_word.word, yytext);			\
        if (S_word.curr_type < 0) S_word.curr_type = typ;	\
    } else {						\
        ADD_TOKEN(t);					\
        S_word.curr_type = typ;				\
    }							\
}

/* forward declarations (for prototypes) */

static void S_init_command(void);
static void S_end_command(void);
static void S_init_word(void);
static void S_end_word(void);
static void S_end_letter(void);
static void S_init_letter(void);
static void S_handle_other(char*);

/* lex function */
extern int yylex (void);
%}

%token DUMMY_TOK
/* vowels - A_TOK must be first */
%token A_TOK AA_TOK I_TOK II_TOK U_TOK UU_TOK RI_TOK RII_TOK
%token LI_TOK LII_TOK AY_TOK AAY_TOK AI_TOK O_TOK OO_TOK AU_TOK AM_TOK AHA_TOK
/* dummy vowels - parser does not use them, but each consonant has these */
%token HALF_TOK IMPLICIT_TOK CONJUNCT_TOK
/* consonants */
%token KA_TOK KHA_TOK GA_TOK GHA_TOK NGA_TOK
%token CHA_TOK CHHA_TOK JA_TOK JHA_TOK JNH_TOK
%token TTA_TOK TTHA_TOK DDA_TOK DDHA_TOK NNA_TOK
%token TA_TOK THA_TOK DA_TOK DHA_TOK NA_TOK
%token PA_TOK PHA_TOK BA_TOK BHA_TOK MA_TOK
%token YA_TOK YYA_TOK RA_TOK LA_TOK VA_TOK
%token SHA_TOK SHHA_TOK SA_TOK
%token HA_TOK
%token LDA_TOK
%token KSHA_TOK GYA_TOK
/* tamil, bengali, etc */
%token NNX_TOK NYA_TOK RRA_TOK
/* additional consonants (PHADOT_TOK must be last - see imap.h) */
%token KADOT_TOK KHADOT_TOK GADOT_TOK DDADOT_TOK DDHADOT_TOK JADOT_TOK
%token PHADOT_TOK
/* specials */
%token RA_HALF_TOK ANUSVARA_TOK CHANDRA_TOK CHANDRA_BN_TOK VIRAAM_TOK
%token AVAGRAHA_TOK
/* aum must be the last token */
%token SRI_TOK AUM_TOK
/* others */
%token BLANK_TOK NEWLINE_TOK NOLIG_TOK COMMAND_TOK ENDPREV_TOK
/* markers: IMARKER and ENDMARKER must bracket all other marker tokens... */
%token IMARKER_TOK MARATHI_TOK HINDI_TOK TAMIL_TOK BENGALI_TOK TELUGU_TOK
%token MALAYALAM_TOK GUJARATI_TOK ORIYA_TOK KANNADA_TOK SANSKRIT_TOK
%token GURMUKHI_TOK ROMAN_TOK ENDMARKER_TOK SETIFM_TOK SETFONT_TOK

%start intext

%%

intext		: 

		| intext SETIFM_TOK	{setifm($2, yytext);}

		| intext SETFONT_TOK	{setfontcmd($2, yytext);}

		| intext
		  startmarker      {process_istart($2);}
		      dinput
		  ENDMARKER_TOK	{process_iend();}
		;

startmarker	: MARATHI_TOK		{$$ = MARATHI_TOK;}
		| HINDI_TOK		{$$ = HINDI_TOK;}
		| IMARKER_TOK		{$$ = IMARKER_TOK;}
		| TAMIL_TOK		{$$ = TAMIL_TOK;}
		| SANSKRIT_TOK		{$$ = SANSKRIT_TOK;}
		| BENGALI_TOK		{$$ = BENGALI_TOK;}
		| TELUGU_TOK		{$$ = TELUGU_TOK;}
		| MALAYALAM_TOK		{$$ = MALAYALAM_TOK;}
		| GUJARATI_TOK		{$$ = GUJARATI_TOK;}
		| ORIYA_TOK		{$$ = ORIYA_TOK;}
		| KANNADA_TOK		{$$ = KANNADA_TOK;}
		| GURMUKHI_TOK		{$$ = GURMUKHI_TOK;}
		| ROMAN_TOK		{$$ = ROMAN_TOK;}
		;


dinput		:

		| dinput 		{ S_init_word(); }
		  word			{ S_end_word(); }

		| dinput		{ S_init_command(); }
		  command		{ S_end_command(); }

		| dinput space

		| error
		;

word		: oletter		{ S_end_letter(); }
		| word oletter		{ S_end_letter(); }
		;

oletter		: letter
		| letter special	{ ADD_TOKEN_TYPE($2, SPECIAL_TYPE); }
		| special		{ ADD_TOKEN_TYPE($1, SPECIAL_TYPE); }
		;

letter		: vowel			{ ADD_TOKEN_TYPE($1, VOWEL_TYPE); }

		| consonants vowel	{ ADD_TOKEN($2);}

		| consonants VIRAAM_TOK	{ ADD_TOKEN(HALF_FORM);}

		| consonants		{ /*
					   * all consonants must end in
					   * a vowel.
					   * vowel may be A_TOK or HALF_TOK
					   */

					  ADD_TOKEN( hook_get_default_vowel() );
					}
		;

consonants	: consonant
				{ ADD_TOKEN_TYPE($1, CONSONANT_SINGLE_TYPE);}

		| consonants consonant
				{ ADD_TOKEN_TYPE($2, CONSONANT_DOUBLE_TYPE);}

		| consonants NOLIG_TOK
				{ S_word.nolig[S_word.curr_letter_n-1] = TRUE;}
		;

vowel		: A_TOK
		| AA_TOK
		| I_TOK
		| II_TOK
		| U_TOK
		| UU_TOK
		| RI_TOK
		| RII_TOK
		| LI_TOK
		| LII_TOK
		| AY_TOK
		| AAY_TOK
		| AI_TOK
		| O_TOK
		| OO_TOK
		| AU_TOK
		| AM_TOK
		| AHA_TOK
		;

consonant	: KA_TOK   |   KHA_TOK   |   GA_TOK   |   GHA_TOK   |   NGA_TOK
		| CHA_TOK  |   CHHA_TOK  |   JA_TOK   |   JHA_TOK   |   JNH_TOK
		| TTA_TOK  |   TTHA_TOK  |   DDA_TOK  |   DDHA_TOK  |   NNA_TOK
		| TA_TOK   |   THA_TOK   |   DA_TOK   |   DHA_TOK   |   NA_TOK
		| PA_TOK   |   PHA_TOK   |   BA_TOK   |   BHA_TOK   |   MA_TOK
		| YA_TOK   |   YYA_TOK   |   RA_TOK   |   LA_TOK    |   VA_TOK
		| SHA_TOK  |   SHHA_TOK  |   SA_TOK
		| HA_TOK
		| LDA_TOK
		| KSHA_TOK |   GYA_TOK
		| NNX_TOK  |   NYA_TOK   |   RRA_TOK
		| KADOT_TOK|   KHADOT_TOK|   GADOT_TOK|   JADOT_TOK|  PHADOT_TOK
		| DDADOT_TOK|  DDHADOT_TOK
		;

special		: RA_HALF_TOK | ANUSVARA_TOK
		| CHANDRA_TOK | CHANDRA_BN_TOK | AVAGRAHA_TOK
		| SRI_TOK     | AUM_TOK
		| VIRAAM_TOK
		;

command		: COMMAND_TOK		{ process_command(yytext); }
		;

space		: BLANK_TOK		{ process_space(yytext); }
		| NEWLINE_TOK		{ process_newline(); }
		| NOLIG_TOK		{  S_handle_other(G_endprev_str); }
		| ENDPREV_TOK		{  S_handle_other(G_endprev_str); }
					/* ENDPREV could be any puncuation */
		;

%%
/*========================================================================== */
/* redefining yyerror to print out yytext, lineno, etc */
void yyerror(char* s) {
    fprintf(stderr,"(%s) Error at/after line %d,  before word (%s) Seen (%s)\n",
		s, G_lineno, yytext, S_word.word);
}

/*========================================================================== */
/*
 * Internal functions
 */

static void S_init_word()
{
	/* send the previous word output to process_init_word() */
    process_init_word(S_word.word);
    S_word.word[0] = '\0';
    S_init_letter();

}

static void S_init_letter()
{
    /*
    fprintf(stderr, "INIT_LETTER\n");
    */
    S_word.curr_letter_n = 0;
    S_word.n = 0;
    S_word.curr_type = -1;
    S_word.curr_spec = -1;
}
static void S_end_word()
{
    process_end_word(S_word.word);
}

/* S_handle_other is called on a ENDPREV_TOK---ilex.l gives that
 * token when a punctuation, etc is seen.
 * This implies that the current word, if any must be output before
 * outputting the chars in yytext.
 */
static void S_handle_other(char* inp)
{
    int i, j;
    j = strlen(inp);
    for (i = 0; i < j; i ++) {
	process_otherchar(inp[i]);
    }
}

static void S_init_command()
{
    /* NOTHING */
}
static void S_end_command()
{
    /* NOTHING */
}

static void S_end_letter()
{
    int i;
    letter_t out;

    if (S_word.curr_letter_n <= 0 && S_word.curr_spec < 0) return;

    if (S_word.curr_letter_n >= CONS_MAX) {
        yyerror("Input Error: too many consonants in letter - can't process it.");
        S_init_word(); /* throw away this letter, error condition */
        return;
    }

    for (i = 0; i < S_word.curr_letter_n-1; i ++) {
	out.cons[i] = S_word.curr_letter[i];
	out.nolig[i] = S_word.nolig[i];
    }
    out.v = S_word.curr_letter[i];
    out.n = S_word.curr_letter_n - 1;
    out.type = S_word.curr_type;
	       /* is correct only for
		* VOWEL, CONSONANT_SINGLE and SPECIAL types 
		*/
    /* check type for correctness */
    out.s = S_word.curr_spec;
    if (out.type == CONSONANT_DOUBLE_TYPE && out.n > 2) {
	out.type = CONSONANT_MANY_TYPE;
    } else if (out.type == SPECIAL_TYPE) {
	out.s = S_word.curr_spec;
	out.v = IMPLICIT_FORM; /* all specials printed out in IMPLICIT form */
	out.n = 0;
    }

#ifdef DEBUG
    fprintf(stderr, "end_letter:out: type %d n %d v %d CONS: ",
	out.type, out.n, out.v);
    for (i = 0; i < out.n; i ++) {
	fprintf(stderr, " %d ", out.cons[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "                                     ");
    for (i = 0; i < out.n; i ++) {
	fprintf(stderr, " %d ", out.nolig[i]);
    }
    fprintf(stderr, "\n");
#endif /*DEBUG*/

    if (S_word.n > 0) {
        /* print a discretionary hyphen for all other akshars in the word */
        print_disc_hyphen();
    }
    
    process_letter(out);

    S_init_letter();
    
    S_word.n ++; /* increment akshar count */
}

/* in_letter() returns TRUE if the parser is in the middle of scanning
 * some letter --- used by ilex.l to return ENDPREV_TOK if needed.
 */
int in_letter()
{
    return (S_word.curr_letter_n > 0);
}
