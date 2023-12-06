#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20080827

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
#ifdef YYPARSE_PARAM_TYPE
#define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
#else
#define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
#endif
#else
#define YYPARSE_DECL() yyparse(void)
#endif /* YYPARSE_PARAM */

extern int YYPARSE_DECL();

static int yygrowstack(void);
#define YYPREFIX "yy"
#line 2 "iyacc.y"
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

/*static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/iyacc.y,v 1.1.1.1 1996/11/29 01:45:34 avinash Exp $";*/

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
#line 122 "y.tab.c"
#define DUMMY_TOK 257
#define A_TOK 258
#define AA_TOK 259
#define I_TOK 260
#define II_TOK 261
#define U_TOK 262
#define UU_TOK 263
#define RI_TOK 264
#define RII_TOK 265
#define LI_TOK 266
#define LII_TOK 267
#define AY_TOK 268
#define AAY_TOK 269
#define AI_TOK 270
#define O_TOK 271
#define OO_TOK 272
#define AU_TOK 273
#define AM_TOK 274
#define AHA_TOK 275
#define HALF_TOK 276
#define IMPLICIT_TOK 277
#define CONJUNCT_TOK 278
#define KA_TOK 279
#define KHA_TOK 280
#define GA_TOK 281
#define GHA_TOK 282
#define NGA_TOK 283
#define CHA_TOK 284
#define CHHA_TOK 285
#define JA_TOK 286
#define JHA_TOK 287
#define JNH_TOK 288
#define TTA_TOK 289
#define TTHA_TOK 290
#define DDA_TOK 291
#define DDHA_TOK 292
#define NNA_TOK 293
#define TA_TOK 294
#define THA_TOK 295
#define DA_TOK 296
#define DHA_TOK 297
#define NA_TOK 298
#define PA_TOK 299
#define PHA_TOK 300
#define BA_TOK 301
#define BHA_TOK 302
#define MA_TOK 303
#define YA_TOK 304
#define YYA_TOK 305
#define RA_TOK 306
#define LA_TOK 307
#define VA_TOK 308
#define SHA_TOK 309
#define SHHA_TOK 310
#define SA_TOK 311
#define HA_TOK 312
#define LDA_TOK 313
#define KSHA_TOK 314
#define GYA_TOK 315
#define NNX_TOK 316
#define NYA_TOK 317
#define RRA_TOK 318
#define KADOT_TOK 319
#define KHADOT_TOK 320
#define GADOT_TOK 321
#define DDADOT_TOK 322
#define DDHADOT_TOK 323
#define JADOT_TOK 324
#define PHADOT_TOK 325
#define RA_HALF_TOK 326
#define ANUSVARA_TOK 327
#define CHANDRA_TOK 328
#define CHANDRA_BN_TOK 329
#define VIRAAM_TOK 330
#define AVAGRAHA_TOK 331
#define SRI_TOK 332
#define AUM_TOK 333
#define BLANK_TOK 334
#define NEWLINE_TOK 335
#define NOLIG_TOK 336
#define COMMAND_TOK 337
#define ENDPREV_TOK 338
#define IMARKER_TOK 339
#define MARATHI_TOK 340
#define HINDI_TOK 341
#define TAMIL_TOK 342
#define BENGALI_TOK 343
#define TELUGU_TOK 344
#define MALAYALAM_TOK 345
#define GUJARATI_TOK 346
#define ORIYA_TOK 347
#define KANNADA_TOK 348
#define SANSKRIT_TOK 349
#define GURMUKHI_TOK 350
#define ROMAN_TOK 351
#define ENDMARKER_TOK 352
#define SETIFM_TOK 353
#define SETFONT_TOK 354
#define YYERRCODE 256
static const short yylhs[] = {                           -1,
    0,    0,    0,    3,    0,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    2,    5,
    2,    7,    2,    2,    2,    4,    4,    9,    9,    9,
   10,   10,   10,   10,   13,   13,   13,   12,   12,   12,
   12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
   12,   12,   12,   12,   12,   14,   14,   14,   14,   14,
   14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
   14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
   14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
   14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
   14,   14,   11,   11,   11,   11,   11,   11,   11,   11,
    6,    8,    8,    8,    8,
};
static const short yylen[] = {                            2,
    0,    2,    2,    0,    5,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    0,    0,
    3,    0,    3,    2,    1,    1,    2,    1,    2,    1,
    1,    2,    2,    1,    1,    2,    2,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,
};
static const short yydefred[] = {                         1,
    0,    8,    6,    7,    9,   11,   12,   13,   14,   15,
   16,   10,   17,   18,    2,    3,    4,    0,   25,    0,
  112,  113,  114,  115,    5,    0,    0,   24,   38,   39,
   40,   41,   42,   43,   44,   45,   46,   47,   48,   49,
   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,
   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,
   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
   80,   81,   82,   83,   84,   85,   86,   87,   88,   89,
   90,   91,   92,   93,   94,   95,   96,   97,   98,  101,
  102,   99,  100,  103,  104,  105,  106,  110,  107,  108,
  109,    0,   26,    0,   30,   31,    0,   35,  111,   23,
   27,   29,   33,   37,   32,   36,
};
static const short yydgoto[] = {                          1,
   17,   20,   18,  102,   26,  110,   27,   28,  103,  104,
  105,  106,  107,  108,
};
static const short yysindex[] = {                         0,
  118,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -238,    0,  104,
    0,    0,    0,    0,    0,   91, -318,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   91,    0, -245,    0,    0,   12,    0,    0,    0,
    0,    0,    0,    0,    0,    0,
};
static const short yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -258,    0,  -68,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -171,    0, -163,    0,    0,   99,    0,    0,    0,
    0,    0,    0,    0,    0,    0,
};
static const short yygindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,  -82,    0,
  -15,  -17,    0,  -16,
};
#define YYTABLESIZE 472
static const short yytable[] = {                         19,
   19,   19,   19,   19,   19,   19,   19,   19,   19,   19,
   19,   19,   19,   19,   19,   19,   19,   19,  109,  111,
   19,   19,   19,   19,   19,   19,   19,   19,   19,   19,
   19,   19,   19,   19,   19,   19,   19,   19,   19,   19,
   19,   19,   19,   19,   19,   19,   19,   19,   19,   19,
   19,   19,   19,   19,   19,   19,   19,   19,   19,   19,
   19,   19,   19,   19,   19,   19,   19,   19,   19,   19,
   19,   19,   19,   19,   19,   19,   19,   19,   19,   19,
   94,   95,   96,   97,   98,   99,  100,  101,  112,  115,
  116,    0,    0,   19,   28,   28,   28,   28,   28,   28,
   28,   28,   28,   28,   28,   28,   28,   28,   28,   28,
   28,   28,    0,    0,    0,   28,   28,   28,   28,   28,
   28,   28,   28,   28,   28,   28,   28,   28,   28,   28,
   28,   28,   28,   28,   28,   28,   28,   28,   28,   28,
   28,   28,   28,   28,   28,   28,   28,   28,   28,   28,
   28,   28,   28,   28,   28,   28,   28,   28,   28,   28,
   28,   28,   21,   21,   21,   21,   21,    0,    0,    0,
   28,   28,   28,   28,   28,    0,    0,    0,    0,    0,
   21,    0,    0,    0,    0,    0,    0,    0,   28,   20,
   20,   20,   20,   20,   20,   20,   20,   20,   20,   20,
   20,   20,   20,   20,   20,   20,   20,    0,    0,    0,
   20,   20,   20,   20,   20,   20,   20,   20,   20,   20,
   20,   20,   20,   20,   20,   20,   20,   20,   20,   20,
   20,   20,   20,   20,   20,   20,   20,   20,   20,   20,
   20,   20,   20,   20,   20,   20,   20,   20,   20,   20,
   20,   20,   20,   20,   20,   20,   20,   20,   20,   20,
   20,   20,   20,   20,   20,    0,    0,    0,   22,   29,
   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,
   40,   41,   42,   43,   44,   45,   46,    0,    0,    0,
   47,   48,   49,   50,   51,   52,   53,   54,   55,   56,
   57,   58,   59,   60,   61,   62,   63,   64,   65,   66,
   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,
   77,   78,   79,   80,   81,   82,   83,   84,   85,   86,
   87,   88,   89,   90,   91,   92,   93,    0,    0,    0,
    0,  113,    0,    0,    0,    0,    0,  114,   29,   30,
   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
   41,   42,   43,   44,   45,   46,    0,    0,    0,   47,
   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,
   58,   59,   60,   61,   62,   63,   64,   65,   66,   67,
   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,
   78,   79,   80,   81,   82,   83,   84,   85,   86,   87,
   88,   89,   90,   91,   92,   93,   94,   95,   96,   97,
   98,   99,  100,  101,   34,   34,   34,   34,    0,   34,
   34,   34,   34,   34,    0,   34,   34,   21,   22,   23,
    0,   24,    0,    0,    0,    0,    0,    0,    0,    0,
   34,    0,    0,    0,    0,   25,    2,    3,    4,    5,
    6,    7,    8,    9,   10,   11,   12,   13,   14,    0,
   15,   16,
};
static const short yycheck[] = {                        258,
  259,  260,  261,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  256,  337,  102,
  279,  280,  281,  282,  283,  284,  285,  286,  287,  288,
  289,  290,  291,  292,  293,  294,  295,  296,  297,  298,
  299,  300,  301,  302,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,  334,  335,  336,  337,  338,
  326,  327,  328,  329,  330,  331,  332,  333,  104,  107,
  107,   -1,   -1,  352,  258,  259,  260,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,   -1,   -1,   -1,  279,  280,  281,  282,  283,
  284,  285,  286,  287,  288,  289,  290,  291,  292,  293,
  294,  295,  296,  297,  298,  299,  300,  301,  302,  303,
  304,  305,  306,  307,  308,  309,  310,  311,  312,  313,
  314,  315,  316,  317,  318,  319,  320,  321,  322,  323,
  324,  325,  334,  335,  336,  337,  338,   -1,   -1,   -1,
  334,  335,  336,  337,  338,   -1,   -1,   -1,   -1,   -1,
  352,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  352,  258,
  259,  260,  261,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,   -1,   -1,   -1,
  279,  280,  281,  282,  283,  284,  285,  286,  287,  288,
  289,  290,  291,  292,  293,  294,  295,  296,  297,  298,
  299,  300,  301,  302,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  326,  327,  328,
  329,  330,  331,  332,  333,   -1,   -1,   -1,  337,  258,
  259,  260,  261,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,   -1,   -1,   -1,
  279,  280,  281,  282,  283,  284,  285,  286,  287,  288,
  289,  290,  291,  292,  293,  294,  295,  296,  297,  298,
  299,  300,  301,  302,  303,  304,  305,  306,  307,  308,
  309,  310,  311,  312,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  323,  324,  325,   -1,   -1,   -1,
   -1,  330,   -1,   -1,   -1,   -1,   -1,  336,  258,  259,
  260,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,   -1,   -1,   -1,  279,
  280,  281,  282,  283,  284,  285,  286,  287,  288,  289,
  290,  291,  292,  293,  294,  295,  296,  297,  298,  299,
  300,  301,  302,  303,  304,  305,  306,  307,  308,  309,
  310,  311,  312,  313,  314,  315,  316,  317,  318,  319,
  320,  321,  322,  323,  324,  325,  326,  327,  328,  329,
  330,  331,  332,  333,  326,  327,  328,  329,   -1,  331,
  332,  333,  334,  335,   -1,  337,  338,  334,  335,  336,
   -1,  338,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  352,   -1,   -1,   -1,   -1,  352,  339,  340,  341,  342,
  343,  344,  345,  346,  347,  348,  349,  350,  351,   -1,
  353,  354,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 354
#if YYDEBUG
static const char *yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"DUMMY_TOK","A_TOK","AA_TOK",
"I_TOK","II_TOK","U_TOK","UU_TOK","RI_TOK","RII_TOK","LI_TOK","LII_TOK",
"AY_TOK","AAY_TOK","AI_TOK","O_TOK","OO_TOK","AU_TOK","AM_TOK","AHA_TOK",
"HALF_TOK","IMPLICIT_TOK","CONJUNCT_TOK","KA_TOK","KHA_TOK","GA_TOK","GHA_TOK",
"NGA_TOK","CHA_TOK","CHHA_TOK","JA_TOK","JHA_TOK","JNH_TOK","TTA_TOK",
"TTHA_TOK","DDA_TOK","DDHA_TOK","NNA_TOK","TA_TOK","THA_TOK","DA_TOK","DHA_TOK",
"NA_TOK","PA_TOK","PHA_TOK","BA_TOK","BHA_TOK","MA_TOK","YA_TOK","YYA_TOK",
"RA_TOK","LA_TOK","VA_TOK","SHA_TOK","SHHA_TOK","SA_TOK","HA_TOK","LDA_TOK",
"KSHA_TOK","GYA_TOK","NNX_TOK","NYA_TOK","RRA_TOK","KADOT_TOK","KHADOT_TOK",
"GADOT_TOK","DDADOT_TOK","DDHADOT_TOK","JADOT_TOK","PHADOT_TOK","RA_HALF_TOK",
"ANUSVARA_TOK","CHANDRA_TOK","CHANDRA_BN_TOK","VIRAAM_TOK","AVAGRAHA_TOK",
"SRI_TOK","AUM_TOK","BLANK_TOK","NEWLINE_TOK","NOLIG_TOK","COMMAND_TOK",
"ENDPREV_TOK","IMARKER_TOK","MARATHI_TOK","HINDI_TOK","TAMIL_TOK","BENGALI_TOK",
"TELUGU_TOK","MALAYALAM_TOK","GUJARATI_TOK","ORIYA_TOK","KANNADA_TOK",
"SANSKRIT_TOK","GURMUKHI_TOK","ROMAN_TOK","ENDMARKER_TOK","SETIFM_TOK",
"SETFONT_TOK",
};
static const char *yyrule[] = {
"$accept : intext",
"intext :",
"intext : intext SETIFM_TOK",
"intext : intext SETFONT_TOK",
"$$1 :",
"intext : intext startmarker $$1 dinput ENDMARKER_TOK",
"startmarker : MARATHI_TOK",
"startmarker : HINDI_TOK",
"startmarker : IMARKER_TOK",
"startmarker : TAMIL_TOK",
"startmarker : SANSKRIT_TOK",
"startmarker : BENGALI_TOK",
"startmarker : TELUGU_TOK",
"startmarker : MALAYALAM_TOK",
"startmarker : GUJARATI_TOK",
"startmarker : ORIYA_TOK",
"startmarker : KANNADA_TOK",
"startmarker : GURMUKHI_TOK",
"startmarker : ROMAN_TOK",
"dinput :",
"$$2 :",
"dinput : dinput $$2 word",
"$$3 :",
"dinput : dinput $$3 command",
"dinput : dinput space",
"dinput : error",
"word : oletter",
"word : word oletter",
"oletter : letter",
"oletter : letter special",
"oletter : special",
"letter : vowel",
"letter : consonants vowel",
"letter : consonants VIRAAM_TOK",
"letter : consonants",
"consonants : consonant",
"consonants : consonants consonant",
"consonants : consonants NOLIG_TOK",
"vowel : A_TOK",
"vowel : AA_TOK",
"vowel : I_TOK",
"vowel : II_TOK",
"vowel : U_TOK",
"vowel : UU_TOK",
"vowel : RI_TOK",
"vowel : RII_TOK",
"vowel : LI_TOK",
"vowel : LII_TOK",
"vowel : AY_TOK",
"vowel : AAY_TOK",
"vowel : AI_TOK",
"vowel : O_TOK",
"vowel : OO_TOK",
"vowel : AU_TOK",
"vowel : AM_TOK",
"vowel : AHA_TOK",
"consonant : KA_TOK",
"consonant : KHA_TOK",
"consonant : GA_TOK",
"consonant : GHA_TOK",
"consonant : NGA_TOK",
"consonant : CHA_TOK",
"consonant : CHHA_TOK",
"consonant : JA_TOK",
"consonant : JHA_TOK",
"consonant : JNH_TOK",
"consonant : TTA_TOK",
"consonant : TTHA_TOK",
"consonant : DDA_TOK",
"consonant : DDHA_TOK",
"consonant : NNA_TOK",
"consonant : TA_TOK",
"consonant : THA_TOK",
"consonant : DA_TOK",
"consonant : DHA_TOK",
"consonant : NA_TOK",
"consonant : PA_TOK",
"consonant : PHA_TOK",
"consonant : BA_TOK",
"consonant : BHA_TOK",
"consonant : MA_TOK",
"consonant : YA_TOK",
"consonant : YYA_TOK",
"consonant : RA_TOK",
"consonant : LA_TOK",
"consonant : VA_TOK",
"consonant : SHA_TOK",
"consonant : SHHA_TOK",
"consonant : SA_TOK",
"consonant : HA_TOK",
"consonant : LDA_TOK",
"consonant : KSHA_TOK",
"consonant : GYA_TOK",
"consonant : NNX_TOK",
"consonant : NYA_TOK",
"consonant : RRA_TOK",
"consonant : KADOT_TOK",
"consonant : KHADOT_TOK",
"consonant : GADOT_TOK",
"consonant : JADOT_TOK",
"consonant : PHADOT_TOK",
"consonant : DDADOT_TOK",
"consonant : DDHADOT_TOK",
"special : RA_HALF_TOK",
"special : ANUSVARA_TOK",
"special : CHANDRA_TOK",
"special : CHANDRA_BN_TOK",
"special : AVAGRAHA_TOK",
"special : SRI_TOK",
"special : AUM_TOK",
"special : VIRAAM_TOK",
"command : COMMAND_TOK",
"space : BLANK_TOK",
"space : NEWLINE_TOK",
"space : NOLIG_TOK",
"space : ENDPREV_TOK",

};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;
int      yyerrflag;
int      yychar;
short   *yyssp;
YYSTYPE *yyvsp;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* variables for the parser stack */
static short   *yyss;
static short   *yysslim;
static YYSTYPE *yyvs;
static unsigned yystacksize;
#line 257 "iyacc.y"
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
#line 714 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void)
{
    int i;
    unsigned newsize;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = yyssp - yyss;
    newss = (yyss != 0)
          ? (short *)realloc(yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    yyss  = newss;
    yyssp = newss + i;
    newvs = (yyvs != 0)
          ? (YYSTYPE *)realloc(yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    yystate = 0;
    *yyssp = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yyssp = yytable[yyn];
        *++yyvsp = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

#ifdef lint
    goto yyerrlab;
#endif

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yyssp = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yyvsp[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 2:
#line 131 "iyacc.y"
	{setifm(yyvsp[0], yytext);}
break;
case 3:
#line 133 "iyacc.y"
	{setfontcmd(yyvsp[0], yytext);}
break;
case 4:
#line 136 "iyacc.y"
	{process_istart(yyvsp[0]);}
break;
case 5:
#line 138 "iyacc.y"
	{process_iend();}
break;
case 6:
#line 141 "iyacc.y"
	{yyval = MARATHI_TOK;}
break;
case 7:
#line 142 "iyacc.y"
	{yyval = HINDI_TOK;}
break;
case 8:
#line 143 "iyacc.y"
	{yyval = IMARKER_TOK;}
break;
case 9:
#line 144 "iyacc.y"
	{yyval = TAMIL_TOK;}
break;
case 10:
#line 145 "iyacc.y"
	{yyval = SANSKRIT_TOK;}
break;
case 11:
#line 146 "iyacc.y"
	{yyval = BENGALI_TOK;}
break;
case 12:
#line 147 "iyacc.y"
	{yyval = TELUGU_TOK;}
break;
case 13:
#line 148 "iyacc.y"
	{yyval = MALAYALAM_TOK;}
break;
case 14:
#line 149 "iyacc.y"
	{yyval = GUJARATI_TOK;}
break;
case 15:
#line 150 "iyacc.y"
	{yyval = ORIYA_TOK;}
break;
case 16:
#line 151 "iyacc.y"
	{yyval = KANNADA_TOK;}
break;
case 17:
#line 152 "iyacc.y"
	{yyval = GURMUKHI_TOK;}
break;
case 18:
#line 153 "iyacc.y"
	{yyval = ROMAN_TOK;}
break;
case 20:
#line 159 "iyacc.y"
	{ S_init_word(); }
break;
case 21:
#line 160 "iyacc.y"
	{ S_end_word(); }
break;
case 22:
#line 162 "iyacc.y"
	{ S_init_command(); }
break;
case 23:
#line 163 "iyacc.y"
	{ S_end_command(); }
break;
case 26:
#line 170 "iyacc.y"
	{ S_end_letter(); }
break;
case 27:
#line 171 "iyacc.y"
	{ S_end_letter(); }
break;
case 29:
#line 175 "iyacc.y"
	{ ADD_TOKEN_TYPE(yyvsp[0], SPECIAL_TYPE); }
break;
case 30:
#line 176 "iyacc.y"
	{ ADD_TOKEN_TYPE(yyvsp[0], SPECIAL_TYPE); }
break;
case 31:
#line 179 "iyacc.y"
	{ ADD_TOKEN_TYPE(yyvsp[0], VOWEL_TYPE); }
break;
case 32:
#line 181 "iyacc.y"
	{ ADD_TOKEN(yyvsp[0]);}
break;
case 33:
#line 183 "iyacc.y"
	{ ADD_TOKEN(HALF_FORM);}
break;
case 34:
#line 185 "iyacc.y"
	{ /*
					   * all consonants must end in
					   * a vowel.
					   * vowel may be A_TOK or HALF_TOK
					   */

					  ADD_TOKEN( hook_get_default_vowel() );
					}
break;
case 35:
#line 196 "iyacc.y"
	{ ADD_TOKEN_TYPE(yyvsp[0], CONSONANT_SINGLE_TYPE);}
break;
case 36:
#line 199 "iyacc.y"
	{ ADD_TOKEN_TYPE(yyvsp[0], CONSONANT_DOUBLE_TYPE);}
break;
case 37:
#line 202 "iyacc.y"
	{ S_word.nolig[S_word.curr_letter_n-1] = TRUE;}
break;
case 111:
#line 246 "iyacc.y"
	{ process_command(yytext); }
break;
case 112:
#line 249 "iyacc.y"
	{ process_space(yytext); }
break;
case 113:
#line 250 "iyacc.y"
	{ process_newline(); }
break;
case 114:
#line 251 "iyacc.y"
	{  S_handle_other(G_endprev_str); }
break;
case 115:
#line 252 "iyacc.y"
	{  S_handle_other(G_endprev_str); }
break;
#line 1056 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = (short) yystate;
    *++yyvsp = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    return (1);

yyaccept:
    return (0);
}
