#ifndef ITRANS_H
#define ITRANS_H

/*
 *========================================================================== 
 * Copyright 1991-2001 Avinash Chopde, All Rights Reserved.
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

/* One minor change by Shrisha Rao <shrao@dvaita.org> on 2002-10-04
   for FreeBSD/Darwin/MacOSX compliance. 
*/

/* static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/itrans.h,v 1.6 1998/09/13 02:13:29 avinash Exp $"; */

#include <stdio.h>
/* #include <malloc.h> -- Commented out by SR */
#include <ctype.h>

/* The following four lines added by SR */

#include <stdlib.h>
#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

/* In FreeBSD 2.0.5, malloc.h was made obsolete; stdlib.h should be
   used instead.  This of course also applies to successor versions of
   FreeBSD, and to Darwin 6.0, the open-source base of MacOS X based
   on FreeBSD 4.4.

   Using the if condition enables malloc.h to be used in cases where
   it is found, e.g., in Linux. -- SR
*/

#if defined(sgi) || defined(__STDC__) || defined(__GNUC__)
/* actually, should be included in more places, but many systems
 * are still missing this file...
 */
#include <unistd.h>
#endif

#if defined(SYSV) || defined(WIN32)
# include <string.h>
#else
# include <strings.h>
#endif
#if defined(__STDC__) || (PROTO_C)
#define P(s)	s
#else
#define P(s) ()
#endif

#define YYDEBUG 1

#include "imap.h"

#if defined(MSDOS) || defined(WIN32)

#define PATHSEP	';'
#define DIRSEP		'\\'
#define PATH_MAX	256
#define READBIN_MODE	"rb"

#else

#include <pwd.h>
#define PATHSEP	':'
#define DIRSEP		'/'
#ifndef PATH_MAX
#define PATH_MAX	1024
#endif
#define READBIN_MODE	"r"

#endif

/* CONVWORD_LEN is the number of characters that will be generated
 * (in TeX or in PostScript) when a single input word is converted
 * Should be large enough to handle the verbose TeX commands,
 * long words in indian languages....
 */
#define CONVWORD_LEN	8192

/* somewhere between Jan-July 1991, the first version of this
 * software came out.
 * Supported Devnagari only, under hindi/marathi switches.
 */

/* environment variable for all the paths to search for input file.. */
#define ITRANS_PATH	"ITRANSPATH"

/* default value for the  ITRANSPATH list of directories */
#if defined(MSDOS) || defined(WIN32)
# define ITRANS_PATH_DEF1	"ITRANSPATH=;..\\lib;..\\lib\\fonts"
#else
# define ITRANS_PATH_DEF1	"ITRANSPATH=:../lib:../lib/fonts"
#endif

#if defined(ITRANSLIB)
# if defined(MSDOS) || defined(WIN32)
#  define ITRANS_PATH_DEF	ITRANS_PATH_DEF1 ";" ITRANSLIB ";" ITRANSLIB "\\fonts"
# else
#  define ITRANS_PATH_DEF	ITRANS_PATH_DEF1 ":" ITRANSLIB ":" ITRANSLIB "/fonts"
# endif
#else
# define ITRANS_PATH_DEF	ITRANS_PATH_DEF1
#endif

#define FALSE	0
#define TRUE	1

/* namelen is used for ifm token name lens, for file name len use PATH_MAX */
#define NAMELEN		256

#define ENDMARKER_INDEX	(ENDMARKER_TOK - IMARKER_TOK)
#define MARKERS_MAX	(ENDMARKER_INDEX + 1)

#define FONTS_MAX	15
#define CONSMANY_ALLOC_SIZE	50 /* allocate these many font_t.consmany at a time */

#define LINELEN		1024
	/* max input line len - AFM, IFM files, mainly */

/* various output types that itrans can generate. */
#define OUTPUT_NONE	0
#define OUTPUT_TEX	1
#define OUTPUT_POSTSCRIPT 2
#define OUTPUT_HTML7	3
#define OUTPUT_HTML8	4
#define OUTPUT_HTML_UTF8	5

/* input word/letter components */
#define	VOWEL_TYPE	1
#define	CONSONANT_SINGLE_TYPE	2
#define	CONSONANT_DOUBLE_TYPE	3
#define	SPECIAL_TYPE	4

#define DIGIT_TYPE	5

#define	CONSONANT_MANY_TYPE	100
#define CONS_MAX	10
	/* maximum number of (half) consonants allowed.
	 * each indian language letter is either a vowel or a
	 * list of half consonants followed by a vowel.
	 */

#define	_I_(ANY_TOK)	((ANY_TOK)-OFFSET_TOK)
	/* _I_ converts a token from the parser to its index in
	 * the 'font_t.enc[_I_()]' array.
	 * assert: _I_(A_TOK) must be 0, i.e., (OFFSET_TOK == A_TOK)
	 */
#define _T_(ANY_FORM)	((ANY_FORM) + OFFSET_TOK)
	/* opposite of _I_(..) above */

#define	A_FORM		(_I_(A_TOK))
#define	AA_FORM		(_I_(AA_TOK))
#define	I_FORM		(_I_(I_TOK))
#define	II_FORM		(_I_(II_TOK))
#define	U_FORM		(_I_(U_TOK))
#define	UU_FORM		(_I_(UU_TOK))
#define	RI_FORM		(_I_(RI_TOK))
#define	RII_FORM	(_I_(RII_TOK))
#define	LI_FORM		(_I_(LI_TOK))
#define	LII_FORM	(_I_(LII_TOK))
#define	AY_FORM		(_I_(AY_TOK))
#define	AAY_FORM	(_I_(AAY_TOK))
#define	AI_FORM		(_I_(AI_TOK))
#define	O_FORM		(_I_(O_TOK))
#define	OO_FORM		(_I_(OO_TOK))
#define	AU_FORM		(_I_(AU_TOK))
#define	AM_FORM		(_I_(AM_TOK))
#define	AHA_FORM	(_I_(AHA_TOK))
#define	HALF_FORM	(_I_(HALF_TOK))
#define	IMPLICIT_FORM	(_I_(IMPLICIT_TOK))
#define	CONJUNCT_FORM	(_I_(CONJUNCT_TOK))

#define NUMFORMS	(CONJUNCT_FORM + 1)

/* =================================================================== */
/* PostScript Stuff */

/* EMSIZE will be the current font size
 * EM converts given em size to PostScript point size
 */
#define EMSIZE	"EMSIZE"
#define EMTOPS	"EM"

/* =================================================================== */
/* TeX Stuff */

/* nothing yet */

/* =================================================================== */
/* LETTER_T */
/* Used to pass info from the parser to the application routine */

typedef struct {
    int		type; /* type - vowel, consonant, or
		       * half consonant and another consonant
		       */
    int		cons[CONS_MAX];
    char	nolig[CONS_MAX];
			/* normally, cons[i] and cons[i+1] are checked
			 * for ligatures, and if one exists, it is used.
			 * But, if nolig[i] is TRUE, then no check
			 * is made for ligatures, cons[i] is printed
			 * using its half form.
			 * Naturally, this only applies if n >= 2
			 */
    int		n; /* number of consonants in cons */
    int		v;    /* vowel that goes with c1 or c2 - or is standalone */
    int		s;    /* accent or special token */
}   letter_t;

/* =================================================================== */
/* PSCHAR_T */
typedef struct {
	/* all units are for a 1 point size char, times 1000 */
    int w; /* char width */
    int llx, lly, urx, ury; /* bounding box */
} pschar_t;

#define UNDEF_CHAR_WIDTH (-999) /* undefined value for pschar_t.w */

/* =================================================================== */
/* COMP_UNIT_T */
/* Each Devnagari Character is a Composite Character.
 * Each composite character is a list of <pcode, deltas> that
 * specify printer the given postscript char pcode, at the given delta.
 * pcode may be IMPLICIT_PSCHAR, implying that this composite char
 * is a complex char (ex: half sa, half ta, yee), and need to get the
 * implicit form the the char.
 * Implicit forms of single chars are directly defined, for multiple
 * forms (as in the above given example) need to compose together
 * all half/implicit forms of the complex char.
 */

#define	IMPLICIT_PSCHAR	-1
#define	NO_PSCHAR	-2

#define D_ORG	1
#define D_CUR	2
	/* the deltas in comp_unit_t may be specied with
	 * respect to the current akshara origin (D_ORG), or wrt to the
	 * current location (D_CUR)
	 * For the X-axis coordinates, D_CUR is the default.
	 * For the Y-axis, D_ORG is the default.
	 * This is because it makes sense to have a single baseline
	 * for a line, thus, by default, the Y-baseline is kept constant
	 * for all characters. (D_CUR is used only for certain characters
	 * that appear under all previous characters - for ex in telugu,
	 * a consonant conjunct has to appear below all the vowel
	 * characters...)
	 * For X axis, as characters placed, the next char should be placed
	 * to the right of the previous character --- and so on, so D_CUR
	 * is the default for X axis coordinates.
	 * As of 25 April 1992, only Y axis can have either of these
	 * directions - X axis only uses D_CUR. Will implement it later...
	 */

typedef struct __comp_unit_t {
    int u_pschar; /* the postscript/TeX character code */
		  /* may be IMPLICIT_PSCHAR, implies that the entire
		   * implicit letter has to be printed out at this
		   * point.
		   */
    int deltax, deltay;   /* The offset that is to be used
			   * to print this character.
			   * The offset is wrt to the current position,
			   * depends on the value of the  variable dtype_?.
			   * (UNLIKE Adobe's Composite Character
			   * defn which is always from char origin)
			   * The delta's refer to a char 1 point
			   * in size, and the delta's are x1000.
			   * If u_pschar == NO_PSCHAR, then
			   * no char is printed, just this delta
			   * is applied.
			   */
    int	dtype_x, dtype_y; /* specifies whether the offset is wrt current pos
	       * or wrt the origin of the character.
	       * default is D_CUR, i.e current pos. for X axis
	       * default is D_ORG, i.e current baseline. for Y axis
	       */
    struct __comp_unit_t* next; /* pointer to the next comp_unit.. */
} comp_unit_t; 

int cus_to_ps P((comp_unit_t* cus, /* chain of PostScript Chars to output */
	      pschar_t fm[256], /* font metrics */
	      char pscomm[])); /* the postscript commands returned here */

int cus_to_tex P((comp_unit_t* cus, /* chain of PostScript Chars to output */
	      pschar_t fm[256], /* font metrics */
	      char pscomm[])); /* the TeX commands returned here */

int cus_to_html P((comp_unit_t* cus, /* chain of Direct Text Chars to output */
	      pschar_t fm[256], /* font metrics */
	      char pscomm[])); /* the text chars returned here */

/* =================================================================== */
/* DCHAR_T */

typedef struct __dchar_t {
    comp_unit_t**	cus; /* for every form, will allocate NUMFORMS */
    struct __dchar_t*	same_as; /* cus not defined for this, use some
				  * some other character's cus.
				  */
} dchar_t;

typedef struct __dcharcm_t {
    int		cons[CONS_MAX]; /* used to store list of CONSONANTS_MANY */
    int		count; /* number of cons */
    comp_unit_t* cus[NUMFORMS]; /* all CUS needed here, so use array,
    				 * just allocate the cus chains
				 * (PCC list) later
				 */
    dchar_t*	same_as; /* see same_as for dchar_t - NOTE: same_as is dchar_t not dcharcm_t */
    char	word[128];	/* for debugging - keep text rep of this conjunct */
} dcharcm_t;

/* =================================================================== */
/* following numbers are assigned to font_t.prop data variable */
#define	UNKNOWN_FONT	0
#define	PS_FONT		0x1 /* use this font for PostScript out only */
#define	TEX_FONT	0x2 /* use this font for TeX out only */
#define	TXT_FONT	0x4 /* use this font for Text (HTML) output only */
#define	UNICODE_FONT	0x8 /* use this font for Text (UTF8) output only */
#define	TEX_PS_FONT	(PS_FONT | TEX_FONT)
#define	TXT_TEX_PS_FONT	(PS_FONT | TEX_FONT | TXT_FONT)

/* FONT_T */
typedef struct {

    int			prop; /* font property: Postscript only, TeX
    			       * only, both Tex  and PostScript, etc */

    ifm_enc_t		enc[NUMCHARS];
	/* Every indian lang char token that is returned by the
	 * parser is associated with a name here:
	 * ex, A_TOK from the parser is named "a",
	 * GYA_TOK is named "gya", etc
	 * These names are used to get the khadi, ligatures, etc
	 * from the IFM file for every indian lang form.
	 */

    dchar_t		khadi[NUMKHADI];
	/* the ka, kaa, ki, etc forms, incl vowels.
	 * Vowels have only one form - the "a" form (implicit char form).
	 * Most chars here have khadi[][].cus non-NULL, while
	 * khadi[][].same_as is NULL.
	 */

    dchar_t		ligatures[NUMCHARS][NUMCHARS];
	/* if special chars exist for consonant-consonant pairs:
	 * da-da, dha-da, ta-ta, etc
	 * Most chars here have ligatures[][].cus NULL, while
	 * ligatures[][].same_as is non-NULL.
	 */

    dcharcm_t*		consmany;
    int			numconsmany;
    int			maxconsmany;
	/* if special chars exist for consonant-many sequences:
	 *  - three or above. Don't know how many will appear in an
	 * IFM file, so this is allocated on the fly.
	 */

    comp_unit_t*	digits[10]; /* the numbers (0..9) themselves */
	/* The CU list for the digits. (no vowel form, etc is required)
         * Normally, a single cu is sufficient to describe a digit.
         * But, there is no reason not to have multiple CUs for a single
         * digit.
         * If digit[i] is null, it implies that the digit "i" itself
         * should be output --- the digits are in the correct location
         * in the font.
	 */

    pschar_t		psfm[NUMPSCHARS]; /* PostScript Font Metrics */	

    int			hyphenchar;/* -1 for no hyphenation, else hyphenchar */
    char		name[NAMELEN]; /* the font name */
    char		fname[NAMELEN]; /* the ifm file name */
    char*		pname; /* the font program file name (PS font only) */
    int			lang_tok; /* lang number - read in from .ifm */
    int			default_vowel; /* if a consonant is present in the
					* input without a vowel, this
					* vowel will be assumed.
					* is either HALF_TOK (most langs)
					* or A_TOK (devanagari: hindi, marathi)
					*/
} font_t;

int fillup_font		P((font_t* font, char fname[]));
void init_font		P((font_t* font));
void dump_font		P((font_t* font, FILE* outfp));
int decode_name		P((char inword[], int* l0, int *l1, dcharcm_t* cm, int* form));

/* in ichar.c */
int make_letter		P((font_t* fptr, /* the font data structure to use */
		letter_t dlet, /* letter to convert  */
		comp_unit_t pcus[], /* the comp units that make this
				     * letter - all u_pschar are valid
				     * PostScript codes, or NO_PSCHAR
				     */
		int	size_pcus));/* array size of ecus- number of elements*/

int add_cus	P((pschar_t psfm[], comp_unit_t* pcu, comp_unit_t cu));
int add_cus_list	P((pschar_t fm[], comp_unit_t* outcus,
			   comp_unit_t* cus, comp_unit_t* impcus));
int get_cus_explicit	P((font_t* fptr, /* the font data structure to use */
		letter_t dlet, /* letter searched for */
		comp_unit_t** cus, /* will return a pointer to the cus here */
		comp_unit_t** imp));/*will return implicit char creation ptr */
int add_special	P((font_t* fptr, int spec, comp_unit_t pcus[], int  size_pcus));

/* in dflt_mkl.c */
int dflt_make_letter	P((font_t* fptr, letter_t dlet, comp_unit_t pcus[],
			  int	size_pcus));
/* in tmil_mkl.c */
int tamil_make_letter	P((font_t* fptr, letter_t dlet, comp_unit_t pcus[],
			  int	size_pcus));

/* in tlgu_mkl.c */
int telugu_make_letter	P((font_t* fptr, letter_t dlet, comp_unit_t pcus[],
			  int	size_pcus));

/* in beng_mkl.c */
int beng_make_letter	P((font_t* fptr, letter_t dlet, comp_unit_t pcus[],
			  int	size_pcus));

/* in iother.c */
int make_other		P((font_t* fptr, /* the font data structure to use */
		int other, /* character to convert  */
		comp_unit_t pcus[], /* the comp units that make this
				     * character - all u_pschar are valid
				     * PostScript codes, or NO_PSCHAR
				     */
		int	size_pcus));/*array size of ecus- number of elements*/

/* =================================================================== */
/* MARKER_T */
/* markers are used to delimit the indian language texts in documents---
 * a single document could contain english, marathi, bengali, etc
 * portions.
 * example markers are : #indian ... #endindian
 * example markers are : #bengali ... #endbengali
 * example markers are : ## ...... ##
 * etc
 * Two things are associated with markers: the IFM file to use, and
 * and font changing text string (command to print out).
 * Even though the markers are named bengali, tamil, etc, they could
 * point to any language --- it all depends on what IFM file is
 * actually being used.
 */
typedef struct {
    char*	curr_ifmname; /* the current IFM file name */
    char*	curr_fontcmd; /* the current font command */
    font_t*	curr_font; /* font metrics */
    char*	name; /* marker name */
} marker_t;

void init_markers	P((marker_t* l, int num));
void clear_marker	P((marker_t *l));
void null_marker	P((marker_t *l));
int get_lang_token	P((char name[]));

/* =================================================================== */
/* ALLFONTS_T */
typedef font_t*	allfonts_t[FONTS_MAX];

font_t*	find_font	P((allfonts_t af, char fname[]));
FILE*   search_fopen	P((char* path, char* fname, char *mode, char* fopened));
int chk_ext		P((char *path, char *right_ext));
void* my_realloc	P((void *ptr, int size));

int setifm		P((int marker_tok, char ifmtok[]));
int setfontcmd		P((int marker_tok, char fontcmd[]));
int find_load_ifm	P((marker_t* l, allfonts_t allf, char ifmtok[]));
void process_istart	P((int marker_tok));
void process_iend	P((void));
int find_cons_many	P((font_t* f, dcharcm_t cm));

int process_init_word	P((char* word));
int process_letter	P((letter_t in));
int process_newline	P((void));
int process_space	P((char* what));
int process_otherchar	P((int c));
int process_end_word	P((char* word));
int process_command	P((char in[]));
int outps_start		P((int fsize));
int outps_end		P((void));
int outtex_start	P((void));
int outtex_end		P((void));
int outhtml_mode	P((int output_html_mode));
int outhtml_start	P((int fsize));
int outhtml_end		P((void));

/* hooks for the lexer/parser */
int print_disc_hyphen	P((void));
int hook_get_lang_tok	P((void));
int hook_get_default_vowel	P((void));
int hook_set_default_vowel	P((int token));
int set_output_type	P((int html_TeX_PostScript));

int yyparse		P((void));

/* =================================================================== */
/* private stuff for use between the lexer/parser */
int in_letter	P((void)); /* returns TRUE if the parse (iyacc.y) is in the
			 * middle of scanning some letter.
			 * Used to decide whether to tell the parser to
			 * stop looking for more characters, and
			 * terminate the letter.
			 * That is done by sending in a ENDPREV_TOK.
			 */
/* ===================================================================== */
/* have to use flex, cannot use AT&T lex, and flex defines yytext as char* */
extern char* yytext; /* for flex */

extern int G_lineno;
extern int G_verbose;
extern font_t*	G_current_font;
extern ifm_enc_t G_ifm_map[NUMCHARS];
extern ifm_enc_t G_ifm_digit_map[10];
/* ===================================================================== */
#ifndef PROTO_C
/* 
   Shouldn't really need these prototype defns, but...
   don't know what to do with this, where does this go, ...... 
   I hope that if the user sets PROTO_C in the Makefile, then there
   is some other source of this defn, otherwise, try it here...
*/
extern char* getenv	P((char*));
   /* stdlib.h is missing on many systems, therefore, I don't
    * want to include stdlib.h.
    * since I use only these functions, added
    * it here specifically.
    */

#endif

/* ===========================^ itrans.h ^============================== */
#undef P
#endif /* ITRANS_H */
