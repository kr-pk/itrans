/*
 *========================================================================== 
 * Copyright 1991-2001 Avinash Chopde, All Rights Reserved.
 */

///*NOTUSED*/static char S_RCSID[] = "$Header$";

#include "itrans.h"
#include "ifm.h"
#include "version.h"

static char S_copyright[] = "\
%%\n\
%% ---------------------------------------------------------\n\
%% %s %s\n\
%% Copyright 1991--2011 Avinash Chopde, All Rights Reserved.\n\
%% ---------------------------------------------------------\n\
%%\n\
";

/* =================================================================== */
#ifdef TCC
/* Under Turbo-C++, declare the stacklen to be larger */
extern unsigned _stklen = 7000U; /* was 7000 */
#endif
/* =================================================================== */

int G_verbose;

static int S_currfontsize = 0;
static int S_output_type = OUTPUT_NONE; /* is OUTPUT_TEX, HTML, or POSTSCRIPT*/
static int S_endwordvowel = -1; /* A_TOK or HALF_TOK or -1 for unassigned */

static allfonts_t	S_allfonts;
static marker_t		S_markers[MARKERS_MAX];

font_t*			G_current_font = NULL; /*GLOBAL*/
			/* when yyparse() is active, 
			 * this always holds: G_current_font  != NULL
			 */

static void S_usage(FILE*);
static void S_start_processing(void);

#define CUS_MAX	100
static comp_unit_t	S_cus[CUS_MAX];

	/* input words are converted to TeX or PostScript commands here */
static char S_comm[CONVWORD_LEN];
    /* must be large enough to store one word -
     * which in TeX can get very large..what with raise boxes, \char defs etc
     * made it static since it is currently 8K bytes, and on the PC stack
     * space is at a premium...
     */

/* =================================================================== */
int process_letter(letter_t in)
{
    int i;

#ifdef DEBUG
    int c1, c2, v;
#endif

    /*
    fprintf(stderr, ":offset is %d in.v %d, in.c1 %d, in.c2 %d\n", OFFSET_TOK,
    			in.v, in.c1, in.c2);
    */

    if (in.v < OFFSET_TOK && in.v != HALF_FORM && in.v != IMPLICIT_FORM) {
        fprintf(stderr,"illegal vowel val (c[0] is %d) - will use 'a' form\n", in.cons[0]);
    }


#ifdef DEBUG
    /* this switch is for printing debugging stmts only... */
    switch(in.type) {
    case VOWEL_TYPE:
	v = _I_(in.v);
	fprintf(stderr, "process_letter: vowel %d\n", v);

	break;
    case SPECIAL_TYPE:
	c1 = _I_(in.cons[0]);

	fprintf(stderr, "process_letter: special %d\n", c1);
	

	break;
    case CONSONANT_SINGLE_TYPE:
	c1 = _I_(in.cons[0]);
	v = _I_(in.v);
	fprintf(stderr, "process_letter: cons single %d vowel %d\n", c1, v);
	

	break;

    case CONSONANT_DOUBLE_TYPE:
	c1 = _I_(in.cons[0]);
	c2 = _I_(in.cons[1]);
	v = _I_(in.v);

	fprintf(stderr,"process_letter: cons double %d %d vowel %d\n",c1,c2,v);

	break;

    case CONSONANT_MANY_TYPE:
	c1 = _I_(in.cons[0]);
	c2 = _I_(in.cons[1]);
	v = _I_(in.v);

	fprintf(stderr,"process_letter: cons many %d %d vowel %d\n", c1, c2, v);

	break;
    }
#endif /*DEBUG*/

    i = make_letter(G_current_font, in, S_cus, CUS_MAX);

	/* warning: there are no checks in make_letter, so CUS_MAX
         * may be execeeded!
         */
    if (i > CUS_MAX) {
        fprintf(stderr, "Program error: possible stack corruption; CUS_MAX (%d) is too low - increase it to %d atleast\n", CUS_MAX, i);
        fprintf(stderr, "CUS_MAX is defined in itrans.c\n");
	exit(10);
    }

    if (i > 0) {

	if (S_output_type == OUTPUT_POSTSCRIPT) {
	    cus_to_ps(S_cus, G_current_font->psfm, S_comm);
	} else if (S_output_type == OUTPUT_HTML7 || S_output_type == OUTPUT_HTML8 || S_output_type == OUTPUT_HTML_UTF8) { 
	    cus_to_html(S_cus, G_current_font->psfm, S_comm);
	} else { 
	    cus_to_tex(S_cus, G_current_font->psfm, S_comm);
	}

	fputs(S_comm, stdout);

	if (S_output_type == OUTPUT_POSTSCRIPT) printf("\n");
    }

    return TRUE;
}

/* =================================================================== */
int main(int argc, char *argv[])
{
    extern int yydebug;
    int ch;
    extern char * optarg;
    char*	cptr;

#ifdef DEBUG
#ifdef sgi
    mallopt(M_DEBUG, TRUE);
#endif
#endif

    for (ch = 0; ch < FONTS_MAX; ch++) S_allfonts[ch] = NULL;

    S_currfontsize = 0;
    G_verbose = 0;
    S_output_type = OUTPUT_NONE;

	/* --- Collect Args */
    while ((ch = getopt(argc, argv, "hH78PUf:o:i:v")) != EOF) {
	switch (ch) {
	case 'h':
	case 'H':
	   S_usage(stderr);
	   exit(0);
	   break;
	case 'v':
	   G_verbose++;
	   break;
	case '7':
	case '8':
	case 'U':
	case 'P':
	   if (S_output_type == OUTPUT_NONE) {

	       if (ch == '7') set_output_type(OUTPUT_HTML7);
	       else if (ch == '8') set_output_type(OUTPUT_HTML8);
	       else if (ch == 'U') set_output_type(OUTPUT_HTML_UTF8);
	       else if (ch == 'P') set_output_type(OUTPUT_POSTSCRIPT);

	   } else {
	       fprintf(stderr,"Error: %c incorrect. Output type already specified earlier in arguments?\n", ch);
	       S_usage(stderr);
	       exit(10);
	   }
	   if (ch == 'P' && S_currfontsize <= 0) S_currfontsize = 20;
	   break;
	case 'f':
	   sscanf(optarg, "%d", &S_currfontsize);
	   break;
	case 'i':
	   if (!freopen(optarg, "r", stdin)) {
	       fprintf(stderr,"Error: Could not open %s for reading\n", optarg);
	       exit(20);
	   }
	   break;
	case 'o':
	   if (!freopen(optarg, "w", stdout)) {
	       fprintf(stderr,"Error: Could not open %s for writing\n", optarg);
	       exit(30);
	   }
	   break;
	default:
	   S_usage(stderr);
           exit(40);
	   break;
	} /* switch */
    }

    for (ch = 0; ch < FONTS_MAX; ch++) {
	S_allfonts[ch] = NULL;
    }
    init_markers(S_markers, MARKERS_MAX);

    G_current_font = S_markers[0].curr_font; /* default font */

    /* dump_font(S_allfonts[0], stderr); */
    if (G_verbose >= 2) {
	cptr = (char*) getenv(ITRANS_PATH);
	if (cptr) {
	    fprintf(stderr, "itrans: defined in environment: %s is \"%s\"\n", ITRANS_PATH, cptr);
	} else {
	    fprintf(stderr, "itrans: using default value: \"%s\"\n", ITRANS_PATH_DEF);
	}
    }

    yydebug = 0;

    if (S_output_type == OUTPUT_POSTSCRIPT || S_output_type == OUTPUT_TEX) {
        S_start_processing();
    }

    yyparse();

    if (S_output_type == OUTPUT_POSTSCRIPT) {
	outps_end();
	printf("%%%% ITRANS Font Files List\n");
	for (ch = 0; ch < MARKERS_MAX; ch ++) {
	    if (S_markers[ch].curr_font && S_markers[ch].curr_font->pname)
	        printf("%%%% Font File Needed: %s\n", S_markers[ch].curr_font->pname);
	}
	printf("%%%% End of ITRANS Font Files List\n");
    } else if (S_output_type == OUTPUT_HTML7 || S_output_type == OUTPUT_HTML8) {
	outhtml_end();
    } else {
	outtex_end();
    }

    return 0;
}
/* =================================================================== */
static void S_start_processing()
{
    static int sAlreadyDone = FALSE;

    char iface[128];

    /* do stuff only once */
    if (sAlreadyDone) {
    	return;
    } else {
	sAlreadyDone = TRUE;
    }

    if (S_output_type == OUTPUT_POSTSCRIPT) {
	printf("%%!PS\n%% *** Need files itrans.pro and [lang].ps before this line..\n");
	sprintf(iface, "(PostScript Interface)");
    } else if (S_output_type == OUTPUT_HTML7) {
	sprintf(iface, "(HTML [ASCII] Interface)");
    } else if (S_output_type == OUTPUT_HTML8) {
	sprintf(iface, "(HTML [ISO Latin1 8-bit] Interface)");
    } else if (S_output_type == OUTPUT_HTML_UTF8) {
	sprintf(iface, "(HTML [Unicode UTF-8] Interface)");
    } else {
	sprintf(iface, "(TeX Interface)");
    }

    if (G_verbose > 0) { /* if -v */
        fprintf(stderr, "** %s %s\n", NAME_VERSION, iface); /* to console */
    }

    if (S_output_type == OUTPUT_POSTSCRIPT) {
	outps_start(S_currfontsize);

	/*
	% BASEFONTNAME should be defined by the user program..
	% so that it refers to the correct font name (bengali font, etc)
	% (itrans.c::main() defines it to be a devenagari font...)
	% just to be upwards compatible with old .ips files...
	*/
	printf("/BASEFONTNAME /DevnacPlain def %% define a default devanagari font\n");

    } else if (S_output_type == OUTPUT_HTML7 || S_output_type == OUTPUT_HTML8 || S_output_type == OUTPUT_HTML_UTF8) {

	outhtml_start(S_currfontsize);

    } else {

	outtex_start();

    }
}
/* =================================================================== */
int process_newline()
{
    if (S_output_type == OUTPUT_POSTSCRIPT) printf("%%\nnewline\n");
    else printf("\n");
    return TRUE;
}
/* =================================================================== */
int process_space(char* what)
{
    if (S_output_type == OUTPUT_POSTSCRIPT) printf("(%s) show ", what);
    else fputs(what, stdout);
    return TRUE;
}
/* =================================================================== */
int process_otherchar(int c)
{
    int i;

    i = make_other(G_current_font, c, S_cus, CUS_MAX);

    if (i > 0) {

        if (S_output_type == OUTPUT_POSTSCRIPT) cus_to_ps(S_cus, G_current_font->psfm, S_comm);
        else if (S_output_type == OUTPUT_HTML7 || S_output_type == OUTPUT_HTML8 || S_output_type == OUTPUT_HTML_UTF8)
	    cus_to_html(S_cus, G_current_font->psfm, S_comm);
        else  cus_to_tex(S_cus, G_current_font->psfm, S_comm);

	fputs(S_comm, stdout);

    } else {
	/* this character is not defined in the IFM file,
	 * assume it is something else that the user needs to output.
	 */
	if (S_output_type == OUTPUT_POSTSCRIPT) {
	    /* in case of PostScript, assume that the user wants the
	     * character to be output using a show command.
	     * Otherwise, such extra characters may very well lead to
	     * errors in the output file.
	     * If the user does desire the characters to be output as they
	     * are, without a (c) show command, that is possible by exiting
	     * the indian language text portion -- use the endmarker to end it.
	     * (that requires a show command, of course).
	     * In case of the { and } characters, do not do anything -
	     * since they are also used to break the lexical scan:
	     * for example the only way to represent the ka-shha ligature
	     * is {k}shha since ksh itself is a consonant
	     */
	    if (c == '(' || c == ')') {
		printf("(\\%03o) show ", c);
	    } else {
		if (c != '{' && c != '}') {
		    printf("(%c) show ", c);
		}
	    }
	    
	} else { /* TeX or Direct Text */

	    putchar(c);

	}
    }

    /* warning checks */
    if (!ispunct(c) && !isdigit(c) && !isspace(c))  {
        if (G_verbose > 0) { /* if -v */
            fprintf(stderr, "Note (around line %d): character [%c] (dec %d) unrecognized by lexer, echoing to output\n", G_lineno, c, c);
        }
    }
    return TRUE;
}
/* =================================================================== */
int process_init_word(char* word)
{
    char wd[LINELEN];
    int i;

    /* if in text/html mode, don't print the words, otherwise print them */
    if (S_output_type == OUTPUT_HTML7 || S_output_type == OUTPUT_HTML8 || S_output_type == OUTPUT_HTML_UTF8) {
	return FALSE;
    }

    /* anything printed here must show up only as a comment */
    /* Both TeX and PostScript use % as the comment char */


    printf("%%");

    if (G_verbose > 1) { /* if -v -v in args */

	/* prev word may contain newlines - suppress them, since extra
	 * newlines may create havoc in TeX.
	 */
	strncpy(wd, word, LINELEN);
	wd[LINELEN-1] = '\0';
	i = strlen(wd);
	for (; i >=0; i --)
	    if (wd[i] == '\n') wd[i] = ' ';

	printf("previous word:%s:", wd);
    }

    printf("\n"); 

    return TRUE;
}
/* =================================================================== */
int process_end_word(char* word)
{
    /*
    Can't print a comment here, since in TeX everything following 
    also becomes a comment, even on the next line.
    And, since the space following the word may not have been
    seen yet, it gets eaten up if a comment is printed here,
    therefore, the word is printed in the function process_init_word()
    instead..clumsy..

    printf("%%\n%% Just saw word [%s]\n", word);
    */
    word[0] = word[0]; /* to prevent unused warning ... */
    return TRUE;
}
/* =================================================================== */
int process_command(char in[])
{
    char com[1024], *cptr;
    float amt;

    strcpy(com, in);

    if (S_output_type == OUTPUT_TEX) printf("\\%s", in); /* TeX, just copy commands */
    else if (S_output_type == OUTPUT_POSTSCRIPT) { /* is postscript out */

        if (!strncmp(com, "kern", 4)) { /* check if kern command */
	    cptr = &com[4];
	    if (sscanf(cptr, "%f", &amt) != 1) {
	        fprintf(stderr, "illegal kern amount [%f]\n", amt);
	        return FALSE;
	    }
	    printf("%s %.3f mul 0.0 rmoveto %% output of a kern command\n",
			    EMSIZE, -amt);
        } else if (!strncmp(com, "-", 1)) { /* check if \- command */
	    printf(" \\- ");
        } else {
            fprintf(stderr, "Warning line %d, PostScript out: command (%s) not implemented yet.\n", G_lineno, in);
        }
    } else { /* Text mode - HTML latin1 or Unicode UTF-8 etc */

	printf("\\%s", in);
	/* just echo command as is, so it can be seen, and post-processed */

	/* fprintf(stderr, "Warning line %d. Text out: command (%s) not implemented yet.\n", G_lineno, in); */
    }

    return TRUE;

}
/* =================================================================== */
void process_istart(int marker_tok)
{
    int i = marker_tok - IMARKER_TOK;

    S_start_processing(); /* ok to call more than once */

    /* set the correct IFM file, and echo out font settings, if any.. */
    G_current_font = S_markers[i].curr_font;
    if (!G_current_font) {
	fprintf(stderr, "Error: No IFM file specified for language %s (line %d)\n", 
			S_markers[i].name, G_lineno);
	fprintf(stderr, "        (see the example files in the doc directory for correct usage)\n");
	fprintf(stderr, "        (statements: #indianifm=... and #indianfont=...., etc)\n");

	exit(50);
    }

    /* Check for output validity */
    if (S_output_type == OUTPUT_POSTSCRIPT && ! (G_current_font->prop & PS_FONT) ) {
	fprintf(stderr, "Font %s is for TeX output only - cannot do PostScript out\n", G_current_font->fname);

	/***** EXITS HERE *****/
	exit(60);
    }

    /* Check for output validity */
    if (S_output_type == OUTPUT_TEX && ! (G_current_font->prop & TEX_FONT) ) {
	fprintf(stderr, "Font %s is for HTML or UTF_8 or PostScript output only - cannot do TeX out\n", G_current_font->fname);

	/***** EXITS HERE *****/
	exit(70);
    }

    /* Check for output validity */
    if ((S_output_type == OUTPUT_HTML7 || S_output_type == OUTPUT_HTML8) && ! (G_current_font->prop & TXT_FONT) ) {
	fprintf(stderr, "Font %s is for PostScript or TeX or UTF_8 output only - cannot do Latin1 HTML Text out\n", G_current_font->fname);

	/***** EXITS HERE *****/
	exit(75);
    }

    /* Check for output validity */
    if ((S_output_type != OUTPUT_HTML_UTF8) && (G_current_font->prop & UNICODE_FONT) ) {
	fprintf(stderr, "Font %s is for UTF_8 Text output only - cannot do any other output mode: -U required\n", G_current_font->fname);

	/***** EXITS HERE *****/
	exit(80);
    }

    if (S_output_type == OUTPUT_TEX) printf("{"); /* provide an
				* enclosing scope for indian
    				* language text stuff for TeX output
				*/

    /* echo out font settings */
    if (S_markers[i].curr_fontcmd) printf("%s", S_markers[i].curr_fontcmd);

    /* if in TeX, output space, prevents next  letter, if non-ITRANS
     * (like F etc), from combining with command
     */
    if (S_output_type == OUTPUT_TEX) printf(" ");
}
/* =================================================================== */
void process_iend()
{
    /* echo out font settings, if #endfont defined (usually for HTML only) */
    if (S_markers[ENDMARKER_INDEX].curr_fontcmd) printf("%s", S_markers[ENDMARKER_INDEX].curr_fontcmd);

    if (S_output_type == OUTPUT_TEX) printf("}"); /* close the enclosing scope
				* for indian language text stuff for TeX output
				* (see process_istart())
				*/


    if (S_output_type != OUTPUT_HTML7 && S_output_type != OUTPUT_HTML8 && S_output_type != OUTPUT_HTML_UTF8)
	printf("%% End of Indic Script text.\n");

}
/* =================================================================== */
int setifm(int marker_tok, char* ifmtok)
{
    int i = marker_tok - IMARKER_TOK;
#ifdef DEBUG
    fprintf(stderr, "loading in language %d at line %d %s\n", marker_tok,
    		G_lineno, ifmtok);
#endif /*DEBUG*/

    /* first make sure some output type  is selected */
    if (S_output_type == OUTPUT_NONE) {
	set_output_type(OUTPUT_TEX); /* default is TeX output */
    }

    S_start_processing(); /* ok to call more than once */

    if (!find_load_ifm(&S_markers[i], S_allfonts, ifmtok)) {
	 fprintf(stderr, "Error: could not load in IFM file (line %d, %s)\n", G_lineno, ifmtok);
	 exit(33);
    }

    return TRUE;
}
/* =================================================================== */
int setfontcmd(int marker_tok, char ifmtok[])
{
    int i = marker_tok - IMARKER_TOK;
    char* eqp;

#ifdef DEBUG
    fprintf(stderr, "setting font for language %d at line %d %s\n", marker_tok,
    		G_lineno, ifmtok);
#endif /*DEBUG*/

    if (i < 0 || i > ENDMARKER_INDEX) {
	fprintf(stderr, "Error: line %d - internal program error: undefined marker (%d)\n", i, G_lineno);

	/***** EXITS HERE *****/
	exit(85);
    }

    if (i != ENDMARKER_INDEX && !S_markers[i].curr_font) {
	fprintf(stderr, "Error: line %d - there must be a setifm comand before setfont command\n",
		G_lineno);
	fprintf(stderr, "     i.e., #indianifm=..... and then #indianfont=...\n");

	/***** EXITS HERE *****/
	exit(90);
    }

    eqp = strchr(ifmtok, '=');
    if (eqp == NULL) {
        fprintf(stderr, "Illegal Indian Font name line: %s (line %d) \n",
			ifmtok, G_lineno);
        return FALSE;
    }

    eqp ++;

    /* store the given string, even it is NULL (means a reset?) */
    if (S_markers[i].curr_fontcmd) free(S_markers[i].curr_fontcmd);
    S_markers[i].curr_fontcmd = NULL;

    S_markers[i].curr_fontcmd = (char*)malloc(sizeof(char)*(strlen(eqp)+1));

    strcpy(S_markers[i].curr_fontcmd, eqp);

    if (G_verbose > 1)
      fprintf(stderr,"Set font command to: (%s)\n",S_markers[i].curr_fontcmd);

#if 0
old code.
now #indianfont arg is not a TeX font, but any TeX command.
so, cannot use hyphenchar here.
users has to provide \hyphenchar themselves.

	/* in TeX mode, inhibit hyphenation if no hyphenchar defined in IFM file */
	if (S_output_type == OUTPUT_TEX) {
	    if (S_markers[i].curr_font && S_markers[i].curr_font->hyphenchar >= 0) {
		/* hyphenation enabled */
                printf("\\hyphenchar%s=%d %% itrans:hyphenate words using this char\n",
			 S_markers[i].curr_fontcmd, S_markers[i].curr_font->hyphenchar);
	    } else {
		/* hyphenation disabled */
                printf("\\hyphenchar%s=-1 %% itrans:do not hyphenate words using this font\n",
			 S_markers[i].curr_fontcmd);
	    }

	}
#endif

    S_start_processing(); /* ok to call more than once */

    return TRUE;
}
/* =================================================================== */
static void S_usage(FILE *fp)
{
    fprintf(fp, S_copyright, NAME_VERSION, "");

    fprintf(fp,"Usage: itrans [-v] [-P [-f <fontsize>]] [-7|8] [-i <infile>] [-o <outfile>] [-h|H]\n");
    fprintf(fp,"            -P for PostScript output [TeX output by default]\n");
    fprintf(fp,"            -f fontsize, for PostScript (-P required)\n");
    fprintf(fp,"            -7 for HTML output, 7-bit ASCII output]\n");
    fprintf(fp,"            -8 for HTML output, 8-bit ISO Latin1 output]\n");
    fprintf(fp,"            -U for HTML output, UTF-8 Unicode output]\n");
    fprintf(fp,"            -i input file name, stdin by default\n");
    fprintf(fp,"            -o output file name, stdout by default\n");
    fprintf(fp,"            -v verbose mode (add'l -v makes it more verbose)\n");
    fprintf(fp,"            -h|H for this message\n");
}
/* =================================================================== */
/* hook for the lexer */
int hook_get_lang_tok()
{
    if (G_current_font) {
	return G_current_font->lang_tok;
    }
    
    return HINDI_TOK; /* default, some harmless value... */
}

/* hook for the lexer */
int hook_set_default_vowel(int token)
{
    if (token == A_TOK || token == HALF_TOK) {
	S_endwordvowel = token;
    } else {
	fprintf(stderr, "Error: line %d. #endwordvowel= can only be a or .h\n", G_lineno);
        return FALSE;
    }
    return TRUE;
}

/* hook for the parser */
int hook_get_default_vowel()
{
    /* first priority to #endwordvowel command in input text */
    if (S_endwordvowel != -1) {
	return S_endwordvowel;
    }

    /* if that is not set, use the DEFAULTVOWEL from the IFM file */
    if (G_current_font) {
	return G_current_font->default_vowel;
    }
    
    /* if neither present, it is HALF_TOK */
    return HALF_TOK; /* default, if a consonant is missing a vowel,
		      * print it in half form
		      * this is normally defined in the IFM file
		      * using keyword DEFAULTVOWEL
		      * is either A_TOK (hindi, marathi) or HALF_TOK (others)
		      */
}

int set_output_type(int text_TeX_PostScript)
{
    if (S_output_type != text_TeX_PostScript && S_output_type != OUTPUT_NONE) {
	fprintf(stderr, "Error: line %d - two conflicting #output=[type] comands seen\n",
		G_lineno);

	/***** EXITS HERE *****/
	exit(81);
    }

    if (  text_TeX_PostScript == OUTPUT_HTML7 ||
          text_TeX_PostScript == OUTPUT_HTML8 ||
	  text_TeX_PostScript == OUTPUT_HTML_UTF8) {

        outhtml_mode(text_TeX_PostScript);

    } else if (text_TeX_PostScript != OUTPUT_TEX &&
               text_TeX_PostScript != OUTPUT_POSTSCRIPT) {

	fprintf(stderr, "Error: line %d - incorrect output type #output=[type]\n",
		G_lineno);

	/***** EXITS HERE *****/
	exit(83);
    }
    S_output_type = text_TeX_PostScript;

    return TRUE;
}

/* =================================================================== */
int print_disc_hyphen()
{
    if (S_output_type == OUTPUT_TEX && G_current_font->hyphenchar >= 0) {
        printf("\\-"); /* discretionary hyphen - TeX mode */
        return TRUE;
    }
    return FALSE;
}
/* =================================================================== */
