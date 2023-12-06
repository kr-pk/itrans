%e 2200
%p 5300
%a 4800

%{
/* above directives increase the various table sizes used by Lex */

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
 * Author:  Avinash Chopde, <avinash@aczoom.com>
 *
 */

///*NOTUSED*/static char S_RCSID[] = "$Header$";

#include "itrans.h"
#include "ifm.h"

#include "y_tab.h"
	/* not including y.tab.h, see makefile which renames it to
	 * y_tab.h for compatibility with DOS machines
	 */

#ifdef YYSTYPE
extern YYSTYPE  yylval;
#else
extern int yylval;
#endif

extern int G_lineno;
extern char G_endprev_str[]; /* defined in iyacc.y, used to pass string
			      * to process
			      */

static int S_lex_ichar(int t);
static int S_print(char* t);
static int S_skipcomments(void);
static void S_include_file(char* input_line);

static int	S_marker_tok = ENDMARKER_TOK; /*which marker being scanned...*/
static int	S_startmarker_tok = IMARKER_TOK; /*which start marker was just scanned...*/
static int	S_use_csx = FALSE; /* whether accepting CS/CSX input alphabet*/

/* users may use shorter markers than #indian ... #endindian etc
 * the ShortMARKERs stand for equivalents to #indian .. #endindan
   SMARKER_START is equivalent to #<lang> which was encountered last (default = #indian).
   SMARKER_END   is equivalent to any end marker
   But, to protect the user document, by default these equivalences
   are turned off. To turn them on, the user should type in
   #useshortmarkers
   #ignoreshortmarkers will turn them off again.
   From release 4.0, made a single "toggle" marker - ##.
   Rel 5.x: default short markers are ON, users need to do #ignoreshortmarkers if needed.
 */

#define SMARKER_START	"##"
#define SMARKER_END	"##"
/* note: have to edit the lex <pattern> <action> statement, too,
 * somewhere around line 140 in this file.
 * From release 4.0, made a single "toggle" marker - ##.
 * If the old separate markers are needed, the rules for recognizing that
 * are available at the end of this file (#if 0'ed out).
 */
 
/* Release 4.04 -- remove AM_TOK. AM is not really needed, and its presence
 * complicates things when I need to pass Devanagari text through Tamil,
 * etc..
 * It is easier to define anusvara for these languages, and that works
 * much better...(both M and .n get recognized as anusvara..
 *
<ITEXT>"aM"		{S_lex_ichar(AM_TOK); return (yylval =  AM_TOK);}
 */
 
static int      S_use_shortmarkers = TRUE;

/* Release 5.00 -- added the #include=<filename> command, works as if given
 * file was read in at that point 
 */

struct	include_stack_t {
    YY_BUFFER_STATE	yy_buffer;
    int			save_lineno;
};

#define	INCLUDE_DEPTH_MAX	64
struct include_stack_t	S_include_stack[64];
int	S_include_stack_index;
%}

%START ITEXT DCOMMAND

FONTDEF		""|[a-zA-Z0-9_,;|\-\.\\"<>+=/ \t]*[a-zA-Z0-9_,;|\-\.\\"<>+=/]+

/*****************
% FONTDEF is used for patterns in HTML and LaTeX, like:
% #hindifont=<FONT FACE="Xdvng">
% #endfont=</FONT>
% where every #hindi should print <FONT FACE="Xdvng"> & corresponding
% #endhindi should print </FONT>
******************/

/* ASCN is ASC without newline. UONLY is only extended Unicode, no ASCII. */
ASC     [\x00-\x7f]
ASCN    [\x00-\t\v-\x7f]
U       [\x80-\xbf]
U2      [\xc2-\xdf]
U3      [\xe0-\xef]
U4      [\xf0-\xf4]

UANY    {ASC}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}
UANYN   {ASCN}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U} 
UONLY   {U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}

%%

"\\%"			{   
			  /* seen \% in the input, complete current
			   * word first, and then echo out
			   * the chars in G_endprev_str.
			   */
			    if (in_letter()) {
				S_print("ichar \\ %");
				strcpy(G_endprev_str, "\\%");
				return (yylval=ENDPREV_TOK);
			    } else {
				ECHO;
			    }
			}

<ITEXT>"\\"		{   
			   /* seen a single backslash */
			   if (in_letter()) {
				/* need to complete current word, send
				 * a ENDPREV token to the parser.
				 */
				S_print("ichar \\");
				/* unput('\\');   ?? use yyless() here */
			        yyless(0);
			        G_endprev_str[0] = '\0';
			        return (yylval=ENDPREV_TOK);
			    } else {
				BEGIN  DCOMMAND;
			    }
			}

<ITEXT>"%"		{ S_print("ichar %");
		          if (in_letter()) {
				/* need to complete current word, send
				 * a ENDPREV token to the parser.
				 */
			     /* unput('%'); handle comment next time around */
			     yyless(0);
			  }
			  return S_skipcomments();
			  /* is a comment, until end-of-line */
			}

"%"			{ S_skipcomments(); putchar('\n'); }

"##"[ \t]*	{ /* begin/end ITEXT if user has set it on */
			if (S_use_shortmarkers) {
			    if (S_marker_tok == ENDMARKER_TOK) {
				BEGIN ITEXT;
				return (S_marker_tok = S_startmarker_tok);
			    } else {
				BEGIN 0;
				return (S_marker_tok = ENDMARKER_TOK);
			    }
			} else ECHO;
		} 

<ITEXT>"a"		{S_lex_ichar(A_TOK); return (yylval =  A_TOK);	}
<ITEXT>"aa"		{S_lex_ichar(AA_TOK); return (yylval =  AA_TOK);}
<ITEXT>"A"		{S_lex_ichar(AA_TOK); return (yylval =  AA_TOK);}
<ITEXT>"i"		{S_lex_ichar(I_TOK); return (yylval =  I_TOK);	}
<ITEXT>"ii"		{S_lex_ichar(II_TOK); return (yylval =  II_TOK);}
<ITEXT>"I"		{S_lex_ichar(II_TOK); return (yylval =  II_TOK);}
<ITEXT>"u"		{S_lex_ichar(U_TOK); return (yylval =  U_TOK);	}
<ITEXT>"U"		{S_lex_ichar(UU_TOK); return (yylval =  UU_TOK);}
<ITEXT>"uu"		{S_lex_ichar(UU_TOK); return (yylval =  UU_TOK);}
<ITEXT>"R^i"		{S_lex_ichar(RI_TOK); return (yylval =  RI_TOK);}
<ITEXT>"RRi"		{S_lex_ichar(RI_TOK); return (yylval =  RI_TOK);}
<ITEXT>"R^I"		{S_lex_ichar(RII_TOK); return (yylval =  RII_TOK);}
<ITEXT>"RRI"		{S_lex_ichar(RII_TOK); return (yylval =  RII_TOK);}
<ITEXT>"L^i"		{S_lex_ichar(LI_TOK); return (yylval =  LI_TOK);}
<ITEXT>"LLi"		{S_lex_ichar(LI_TOK); return (yylval =  LI_TOK);}
<ITEXT>"L^I"		{S_lex_ichar(LII_TOK); return (yylval =  LII_TOK);}
<ITEXT>"LLI"		{S_lex_ichar(LII_TOK); return (yylval =  LII_TOK);}
<ITEXT>"e"		{S_lex_ichar(AY_TOK); return (yylval =  AY_TOK);}
<ITEXT>"E"		{S_lex_ichar(AAY_TOK); return (yylval =  AAY_TOK);}
<ITEXT>"ai"		{S_lex_ichar(AI_TOK); return (yylval =  AI_TOK);}
<ITEXT>"o"		{S_lex_ichar(O_TOK); return (yylval =  O_TOK);	}
<ITEXT>"O"		{S_lex_ichar(OO_TOK); return (yylval =  OO_TOK);}
<ITEXT>"au"		{S_lex_ichar(AU_TOK); return (yylval =  AU_TOK);}
<ITEXT>"H"		{S_lex_ichar(AHA_TOK); return (yylval =  AHA_TOK);}
<ITEXT>"k"		{S_lex_ichar(KA_TOK); return (yylval =  KA_TOK);}
<ITEXT>"q"		{S_lex_ichar(KADOT_TOK); return (yylval =  KADOT_TOK);}
<ITEXT>"kh"		{S_lex_ichar(KHA_TOK); return (yylval =  KHA_TOK);}
<ITEXT>"K"		{S_lex_ichar(KHADOT_TOK); return (yylval = KHADOT_TOK);}
<ITEXT>"gh"		{S_lex_ichar(GHA_TOK); return (yylval =  GHA_TOK);}
<ITEXT>"g"		{S_lex_ichar(GA_TOK); return (yylval =  GA_TOK);}
<ITEXT>"G"		{S_lex_ichar(GADOT_TOK); return (yylval =  GADOT_TOK);}
<ITEXT>"N^"		{S_lex_ichar(NGA_TOK); return (yylval =  NGA_TOK);}
<ITEXT>"~N"		{S_lex_ichar(NGA_TOK); return (yylval =  NGA_TOK);}
<ITEXT>"Ch"		{S_lex_ichar(CHHA_TOK); return (yylval =  CHHA_TOK);}
<ITEXT>"chh"		{if (S_use_csx) yyless(1); /* unput last h */
			 S_lex_ichar(CHHA_TOK); return (yylval =  CHHA_TOK);}
<ITEXT>"ch"		{if (S_use_csx) {
			    S_lex_ichar(CHHA_TOK); return (yylval =  CHHA_TOK);}
			else {
			    S_lex_ichar(CHA_TOK); return (yylval =  CHA_TOK);}}
<ITEXT>"j"		{S_lex_ichar(JA_TOK); return (yylval =  JA_TOK);}
<ITEXT>"z"		{S_lex_ichar(JADOT_TOK); return (yylval =  JADOT_TOK);}
<ITEXT>"J"		{S_lex_ichar(JADOT_TOK); return (yylval =  JADOT_TOK);}
<ITEXT>"jh"		{S_lex_ichar(JHA_TOK); return (yylval =  JHA_TOK);}
<ITEXT>"JN"		{S_lex_ichar(JNH_TOK); return (yylval =  JNH_TOK);}
<ITEXT>"~n"		{S_lex_ichar(JNH_TOK); return (yylval =  JNH_TOK);}
<ITEXT>"T"		{S_lex_ichar(TTA_TOK); return (yylval =  TTA_TOK);}
<ITEXT>"Th"		{S_lex_ichar(TTHA_TOK); return (yylval =  TTHA_TOK);}
<ITEXT>"D"		{S_lex_ichar(DDA_TOK); return (yylval =  DDA_TOK);}
<ITEXT>".D"		{S_lex_ichar(DDADOT_TOK);return (yylval=DDADOT_TOK);}
<ITEXT>"Dh"		{S_lex_ichar(DDHA_TOK); return (yylval =  DDHA_TOK);}
<ITEXT>".Dh"		{S_lex_ichar(DDHADOT_TOK);return (yylval=DDHADOT_TOK);}
<ITEXT>"N"		{S_lex_ichar(NNA_TOK); return (yylval =  NNA_TOK);}
<ITEXT>"t"		{S_lex_ichar(TA_TOK); return (yylval =  TA_TOK);}
<ITEXT>"th"		{S_lex_ichar(THA_TOK); return (yylval =  THA_TOK);}
<ITEXT>"d"		{S_lex_ichar(DA_TOK); return (yylval =  DA_TOK);}
<ITEXT>"dh"		{S_lex_ichar(DHA_TOK); return (yylval =  DHA_TOK);}
<ITEXT>"n"		{S_lex_ichar(NA_TOK); return (yylval =  NA_TOK);}
<ITEXT>"^n"		{S_lex_ichar(NNX_TOK); return (yylval =  NNX_TOK);}
<ITEXT>"p"		{S_lex_ichar(PA_TOK); return (yylval =  PA_TOK);}
<ITEXT>"ph"		{S_lex_ichar(PHA_TOK); return (yylval =  PHA_TOK);}
<ITEXT>"f"		{S_lex_ichar(PHADOT_TOK); return (yylval = PHADOT_TOK);}
<ITEXT>"bh"		{S_lex_ichar(BHA_TOK); return (yylval =  BHA_TOK);}
<ITEXT>"b"		{S_lex_ichar(BA_TOK); return (yylval =  BA_TOK);}
<ITEXT>"m"		{S_lex_ichar(MA_TOK); return (yylval =  MA_TOK);}
<ITEXT>"y"		{S_lex_ichar(YA_TOK); return (yylval =  YA_TOK);}
<ITEXT>"Y"		{S_lex_ichar(YYA_TOK); return (yylval =  YYA_TOK);}
<ITEXT>"r"		{S_lex_ichar(RA_TOK); return (yylval =  RA_TOK);}
<ITEXT>"v"		{S_lex_ichar(VA_TOK); return (yylval =  VA_TOK);}
<ITEXT>"w"		{S_lex_ichar(VA_TOK); return (yylval =  VA_TOK);}
<ITEXT>"Sh"		{S_lex_ichar(SHHA_TOK); return (yylval =  SHHA_TOK);}
<ITEXT>"shh"		{S_lex_ichar(SHHA_TOK); return (yylval =  SHHA_TOK);}
<ITEXT>"sh"		{S_lex_ichar(SHA_TOK); return (yylval =  SHA_TOK);}
<ITEXT>"s"		{S_lex_ichar(SA_TOK); return (yylval =  SA_TOK);}
<ITEXT>"h"		{S_lex_ichar(HA_TOK); return (yylval =  HA_TOK);}
<ITEXT>"ld"		{S_lex_ichar(LDA_TOK); return (yylval =  LDA_TOK);}
<ITEXT>"L"		{S_lex_ichar(LDA_TOK); return (yylval =  LDA_TOK);}
<ITEXT>"l"		{S_lex_ichar(LA_TOK); return (yylval =  LA_TOK);}
<ITEXT>"ksh"		{S_lex_ichar(KSHA_TOK); return (yylval =  KSHA_TOK);}
<ITEXT>"x"		{S_lex_ichar(KSHA_TOK); return (yylval =  KSHA_TOK);}
<ITEXT>"dny"		{S_lex_ichar(GYA_TOK); return (yylval =  GYA_TOK);}
<ITEXT>"GY"		{S_lex_ichar(GYA_TOK); return (yylval =  GYA_TOK);}

<ITEXT>"R"		{S_lex_ichar(RRA_TOK); return (yylval =  RRA_TOK);}

<ITEXT>"AUM"		{S_lex_ichar(AUM_TOK); return (yylval =  AUM_TOK);}
<ITEXT>"OM"		{S_lex_ichar(AUM_TOK); return (yylval =  AUM_TOK);}
<ITEXT>"SRI"		{S_lex_ichar(SRI_TOK); return (yylval =  SRI_TOK);}
<ITEXT>"^r"		{S_lex_ichar(RA_HALF_TOK);return (yylval=RA_HALF_TOK);}
<ITEXT>".r"		{S_lex_ichar(RA_HALF_TOK);return (yylval=RA_HALF_TOK);}
<ITEXT>"M"	      {S_lex_ichar(ANUSVARA_TOK);return(yylval=ANUSVARA_TOK);}
<ITEXT>".n"	      {S_lex_ichar(ANUSVARA_TOK);return(yylval=ANUSVARA_TOK);}
<ITEXT>".h"		{S_lex_ichar(VIRAAM_TOK);return(yylval=VIRAAM_TOK);}
<ITEXT>".c"		{S_lex_ichar(CHANDRA_TOK);return(yylval= CHANDRA_TOK);}
<ITEXT>".N"	    {S_lex_ichar(CHANDRA_BN_TOK);return(yylval=CHANDRA_BN_TOK);}
<ITEXT>".a"	     {S_lex_ichar(AVAGRAHA_TOK);return(yylval=AVAGRAHA_TOK);}
<ITEXT>"'"	     { if (S_use_csx) {
                         S_lex_ichar(AVAGRAHA_TOK);return(yylval=AVAGRAHA_TOK);}
		       else {
			    strcpy(G_endprev_str, yytext);
			    return (yylval=ENDPREV_TOK);
			}
		     }

<ITEXT>{UONLY}	        {S_print("got utf-8 char");
                         /* this rule should come before the csx codes 342, etc */
			 strcpy(G_endprev_str, yytext);
			 /* return ENDPREV for any punctuation, etc char */
			 return (yylval=ENDPREV_TOK);
			}

<ITEXT>"\340"		{S_lex_ichar(AA_TOK); return (yylval =  AA_TOK);}
<ITEXT>"\342"		{S_lex_ichar(AA_TOK); return (yylval =  AA_TOK);}
<ITEXT>"\343"		{S_lex_ichar(II_TOK); return (yylval =  II_TOK);}
<ITEXT>"\344"		{S_lex_ichar(II_TOK); return (yylval =  II_TOK);}
<ITEXT>"\345"		{S_lex_ichar(UU_TOK); return (yylval =  UU_TOK);}
<ITEXT>"\347"		{S_lex_ichar(RI_TOK); return (yylval =  RI_TOK);}
<ITEXT>"\350"		{S_lex_ichar(RI_TOK); return (yylval =  RI_TOK);}
<ITEXT>"\351"		{S_lex_ichar(RII_TOK); return (yylval =  RII_TOK);}
<ITEXT>"\352"		{S_lex_ichar(RII_TOK); return (yylval =  RII_TOK);}
<ITEXT>"\353"		{S_lex_ichar(LI_TOK); return (yylval =  LI_TOK);}
<ITEXT>"\354"		{S_lex_ichar(LI_TOK); return (yylval =  LI_TOK);}
<ITEXT>"\355"		{S_lex_ichar(LII_TOK); return (yylval =  LII_TOK);}
<ITEXT>"\356"		{S_lex_ichar(LII_TOK); return (yylval =  LII_TOK);}
<ITEXT>"\357"		{S_lex_ichar(NGA_TOK); return (yylval =  NGA_TOK);}
<ITEXT>"\360"		{S_lex_ichar(NGA_TOK); return (yylval =  NGA_TOK);}
<ITEXT>"c"		{S_lex_ichar(CHA_TOK); return (yylval =  CHA_TOK);}
<ITEXT>"\244"		{S_lex_ichar(JNH_TOK); return (yylval =  JNH_TOK);}
<ITEXT>"\361h"		{S_lex_ichar(TTHA_TOK); return (yylval =  TTHA_TOK);}
<ITEXT>"\362h"		{S_lex_ichar(TTHA_TOK); return (yylval =  TTHA_TOK);}
<ITEXT>"\361"		{S_lex_ichar(TTA_TOK); return (yylval =  TTA_TOK);}
<ITEXT>"\362"		{S_lex_ichar(TTA_TOK); return (yylval =  TTA_TOK);}
<ITEXT>"\363h"		{S_lex_ichar(DDHA_TOK); return (yylval =  DDHA_TOK);}
<ITEXT>"\364h"		{S_lex_ichar(DDHA_TOK); return (yylval =  DDHA_TOK);}
<ITEXT>"\363"		{S_lex_ichar(DDA_TOK); return (yylval =  DDA_TOK);}
<ITEXT>"\364"		{S_lex_ichar(DDA_TOK); return (yylval =  DDA_TOK);}
<ITEXT>"\365"		{S_lex_ichar(NNA_TOK); return (yylval =  NNA_TOK);}
<ITEXT>"\366"		{S_lex_ichar(NNA_TOK); return (yylval =  NNA_TOK);}
<ITEXT>"\367"		{S_lex_ichar(SHA_TOK); return (yylval =  SHA_TOK);}
<ITEXT>"\370"		{S_lex_ichar(SHA_TOK); return (yylval =  SHA_TOK);}
<ITEXT>"\371"		{S_lex_ichar(SHHA_TOK); return (yylval =  SHHA_TOK);}
<ITEXT>"\372"		{S_lex_ichar(SHHA_TOK); return (yylval =  SHHA_TOK);}
<ITEXT>"\327"		{S_lex_ichar(LDA_TOK); return (yylval =  LDA_TOK);}

<ITEXT>"\237"		{S_lex_ichar(RRA_TOK); return (yylval =  RRA_TOK);}

<ITEXT>"\247"	      {S_lex_ichar(ANUSVARA_TOK);return(yylval=ANUSVARA_TOK);}
<ITEXT>"\374"	      {S_lex_ichar(ANUSVARA_TOK);return(yylval=ANUSVARA_TOK);}
<ITEXT>"\375"	      {S_lex_ichar(ANUSVARA_TOK);return(yylval=ANUSVARA_TOK);}
<ITEXT>"\376"		{S_lex_ichar(AHA_TOK); return (yylval =  AHA_TOK);}
<ITEXT>"\377"		{S_lex_ichar(AHA_TOK); return (yylval =  AHA_TOK);}

<ITEXT>[ \t]		{S_print("ichar \t");return (yylval =  BLANK_TOK);}
<ITEXT>[\n]	    {S_print("ichar \n");G_lineno++;return(yylval=NEWLINE_TOK);}

<ITEXT>"{}" 	    {	S_print("ichar {}");
		        if (in_letter()) {
			    strcpy(G_endprev_str, yytext);
			    return (yylval=NOLIG_TOK);
			} else {
			    strcpy(G_endprev_str, yytext);
			    return (yylval=ENDPREV_TOK);
		        }
		    }

<ITEXT>"_" 	    {	S_print("ichar _");
		        if (in_letter()) {
			    /* do nothing! this char inside a akshara
			     * is used to break the lexical scan.
			     */
			} else {
			    strcpy(G_endprev_str, yytext);
			    return (yylval=ENDPREV_TOK);
		        }
		    }

<ITEXT>. 	    {	S_print("ichar .");
			strcpy(G_endprev_str, yytext);
			/* return ENDPREV for any punctuation, etc char */
			return (yylval=ENDPREV_TOK);
		    }

<DCOMMAND>[a-zA-Z][\.\-a-zA-Z0-9]* {
			S_print("dcommand a-z");
			BEGIN ITEXT;
			return (COMMAND_TOK);
		}

<DCOMMAND>- 	{
			S_print("dcommand \\-");
			BEGIN ITEXT;
			return (COMMAND_TOK);
		}

<DCOMMAND>.	{
			S_print("dcommand other");
			BEGIN ITEXT;
			sprintf(G_endprev_str, "\\%c", yytext[0]);
				/* just one char matched*/
			return (yylval=ENDPREV_TOK);
		}

\#indian[ \t]*		{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= IMARKER_TOK);}
\#endindian[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#marathi[ \t]*		{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= MARATHI_TOK);}
\#endmarathi[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#hindi[ \t]*		{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= HINDI_TOK);}
\#endhindi[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#tamil[ \t]*		{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= TAMIL_TOK);}
\#endtamil[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#sanskrit[ \t]*	{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= SANSKRIT_TOK);}
\#endsanskrit[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#bengali[ \t]*		{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= BENGALI_TOK);}
\#endbengali[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#telugu[ \t]*		{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= TELUGU_TOK);}
\#endtelugu[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#malayalam[ \t]*	{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= MALAYALAM_TOK);}
\#endmalayalam[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#gujarati[ \t]*	{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= GUJARATI_TOK);}
\#endgujarati[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#oriya[ \t]*		{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= ORIYA_TOK);}
\#endoriya[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#kannada[ \t]*		{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= KANNADA_TOK);}
\#endkannada[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#gurmukhi[ \t]*	{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= GURMUKHI_TOK);}
\#endgurmukhi[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}
\#roman[ \t]*		{BEGIN ITEXT; return (S_marker_tok=S_startmarker_tok= ROMAN_TOK);}
\#endroman[ \t]*	{BEGIN 0;     return (S_marker_tok = ENDMARKER_TOK);}

\#indianifm=[a-zA-Z0-9_\-\.]*	{yylval = IMARKER_TOK; return SETIFM_TOK;}
\#marathiifm=[a-zA-Z0-9_\-\.]*	{yylval = MARATHI_TOK; return SETIFM_TOK;}
\#hindiifm=[a-zA-Z0-9_\-\.]*	{yylval = HINDI_TOK; return SETIFM_TOK;}
\#tamilifm=[a-zA-Z0-9_\-\.]*	{yylval = TAMIL_TOK; return SETIFM_TOK;}
\#sanskritifm=[a-zA-Z0-9_\-\.]*	{yylval = SANSKRIT_TOK; return SETIFM_TOK;}
\#bengaliifm=[a-zA-Z0-9_\-\.]*	{yylval = BENGALI_TOK; return SETIFM_TOK;}
\#teluguifm=[a-zA-Z0-9_\-\.]*	{yylval = TELUGU_TOK; return SETIFM_TOK;}
\#malayalamifm=[a-zA-Z0-9_\-\.]* {yylval = MALAYALAM_TOK; return SETIFM_TOK;}
\#gujaratiifm=[a-zA-Z0-9_\-\.]*	{yylval = GUJARATI_TOK; return SETIFM_TOK;}
\#oriyaifm=[a-zA-Z0-9_\-\.]*	{yylval = ORIYA_TOK; return SETIFM_TOK;}
\#kannadaifm=[a-zA-Z0-9_\-\.]*	{yylval = KANNADA_TOK; return SETIFM_TOK;}
\#gurmukhiifm=[a-zA-Z0-9_\-\.]*	{yylval = GURMUKHI_TOK; return SETIFM_TOK;}
\#romanifm=[a-zA-Z0-9_\-\.]*	{yylval = ROMAN_TOK; return SETIFM_TOK;}

\#indianfont={FONTDEF} {yylval = IMARKER_TOK; return SETFONT_TOK;}
\#hindifont={FONTDEF}	{yylval = HINDI_TOK; return SETFONT_TOK;}
\#tamilfont={FONTDEF} {yylval = TAMIL_TOK; return SETFONT_TOK;}
\#marathifont={FONTDEF} {yylval = MARATHI_TOK; return SETFONT_TOK;}
\#sanskritfont={FONTDEF} {yylval = SANSKRIT_TOK; return SETFONT_TOK;}
\#bengalifont={FONTDEF} {yylval = BENGALI_TOK; return SETFONT_TOK;}
\#telugufont={FONTDEF} {yylval = TELUGU_TOK; return SETFONT_TOK;}
\#malayalamfont={FONTDEF} {yylval = MALAYALAM_TOK;return SETFONT_TOK;}
\#gujaratifont={FONTDEF} {yylval = GUJARATI_TOK; return SETFONT_TOK;}
\#oriyafont={FONTDEF} {yylval = ORIYA_TOK; return SETFONT_TOK;}
\#kannadafont={FONTDEF} {yylval = KANNADA_TOK; return SETFONT_TOK;}
\#gurmukhifont={FONTDEF} {yylval = GURMUKHI_TOK; return SETFONT_TOK;}
\#romanfont={FONTDEF} {yylval = ROMAN_TOK; return SETFONT_TOK;}

\#endfont={FONTDEF}	{yylval = ENDMARKER_TOK; return SETFONT_TOK;}

\#useshortmarkers	{
				S_use_shortmarkers = TRUE;
				if (in_letter()) {
				    G_endprev_str[0] = '\0'; /* NULL string */
				    return (yylval=ENDPREV_TOK);
				}
			}

\#ignoreshortmarkers	{
				S_use_shortmarkers = FALSE;
				if (in_letter()) {
				    G_endprev_str[0] = '\0'; /* NULL string */
				    return (yylval=ENDPREV_TOK);
				}
			}

\#usecsx		{
				S_use_csx = TRUE;
				if (in_letter()) {
				    G_endprev_str[0] = '\0'; /* NULL string */
				    return (yylval=ENDPREV_TOK);
				}
			}

\#ignorecsx		{
				S_use_csx = FALSE;
				if (in_letter()) {
				    G_endprev_str[0] = '\0'; /* NULL string */
				    return (yylval=ENDPREV_TOK);
				}
			}

\#endwordvowel=a	{
				hook_set_default_vowel(A_TOK);
				if (in_letter()) {
				    G_endprev_str[0] = '\0'; /* NULL string */
				    return (yylval=ENDPREV_TOK);
				}
			}

\#endwordvowel=.h	{
				hook_set_default_vowel(HALF_TOK);
				if (in_letter()) {
				    G_endprev_str[0] = '\0'; /* NULL string */
				    return (yylval=ENDPREV_TOK);
				}
			}

\#include=[a-zA-Z0-9_/\-\.\\]+[ \t]*	{
			/* include given file, nest files */
			S_include_file(yytext);
			}

\#output=[ \t]*UTF_8	{
			/* specifies output type - HTML, TeX, or Postscript */
			set_output_type(OUTPUT_HTML_UTF8);
			if (in_letter()) {
			    G_endprev_str[0] = '\0'; /* NULL string */
			    return (yylval=ENDPREV_TOK);
			}
			}

\#output=[ \t]*HTML_7	{
			/* specifies output type - HTML, TeX, or Postscript */
			set_output_type(OUTPUT_HTML7);
			if (in_letter()) {
			    G_endprev_str[0] = '\0'; /* NULL string */
			    return (yylval=ENDPREV_TOK);
			}
			}

\#output=[ \t]*HTML_8	{
			/* specifies output type - HTML, TeX, or Postscript */
			set_output_type(OUTPUT_HTML8);
			if (in_letter()) {
			    G_endprev_str[0] = '\0'; /* NULL string */
			    return (yylval=ENDPREV_TOK);
			}
			}

\#output=[ \t]*TeX	{
			/* specifies output type - HTML, TeX, or Postscript */
			set_output_type(OUTPUT_TEX);
			if (in_letter()) {
			    G_endprev_str[0] = '\0'; /* NULL string */
			    return (yylval=ENDPREV_TOK);
			}
			}

\#output=[ \t]*PostScript	{
			/* specifies output type - HTML, TeX, or Postscript */
			set_output_type(OUTPUT_POSTSCRIPT);
			if (in_letter()) {
			    G_endprev_str[0] = '\0'; /* NULL string */
			    return (yylval=ENDPREV_TOK);
			}
			}

<<EOF>>			{
			    if (--S_include_stack_index < 0) {
				yyterminate();
			    } else {
				yy_delete_buffer( YY_CURRENT_BUFFER );
				yy_switch_to_buffer( S_include_stack[S_include_stack_index].yy_buffer);
				G_lineno = S_include_stack[S_include_stack_index].save_lineno;
				if (G_verbose > 0) {
				    fprintf(stderr, "-- Done with included file (back to line %d).\n", G_lineno);
				}
			    }
			}

\n		{S_print("got newline\n");G_lineno++; ECHO;		}
.		{S_print("everything else! got .");ECHO;		}

%%

static int S_lex_ichar(int t)
{
#ifdef DEBUG
    if (t > 255) fprintf(stderr, "LEXER: have seen <%s> tok %d (I %d)\n",
				yytext, t, _I_(t));
    else fprintf(stderr, "LEXER: seen char %c (dec %d)\n", t, t);
#endif /*DEBUG*/
    return TRUE;
}
static int S_print(char* t)
{
#ifdef DEBUG
    fprintf(stderr, "LEXERprint: have seen <%s> rule is %s\n",
				yytext, t);
#endif /*DEBUG*/
    return TRUE;
}

static int S_skipcomments()
{
     int c;
     if (in_letter()) {
	 G_endprev_str[0] = '\0';
	 return (yylval=ENDPREV_TOK);
     } else {
	 /* this the next time around, eat up the comment */
	 putchar('%');
	 while ( (c = input()) != EOF && c != '\n')
		putchar(c);

	 G_lineno++;
	 return (yylval=NEWLINE_TOK);
     }
}

/* include yywrap here so do not need -lfl when using flex */
#ifndef yywrap /* just in case it was ifdef'ed - I believe unix lex may do this */
int yywrap() { return 1; }
#endif

static void S_include_file(char* input_line)
{
    char	fname[PATH_MAX];
    char	fopened[PATH_MAX];
    char*	cptr;
    int		i;
    FILE*	fp;
    char*	ienv;

    cptr = strchr(input_line, '=');

    i = cptr ? sscanf(cptr+1, "%s", fname) : 0;
    if (i != 1) {
	fprintf(stderr, "Error Line %d: include command invalid :%s\n", G_lineno, input_line);
	return;
    }

    /* get the search path... */
    ienv = (char*) getenv(ITRANS_PATH);
    if (!ienv) {
	ienv = strchr(ITRANS_PATH_DEF, '=') + 1;
    }

    if ( !ienv ) {
	fprintf(stderr, "Program error (ITRANSPATH not found?): S_include_file() got NULL env var\n");
	return;
    }

    fp = search_fopen(ienv, fname, "r", fopened);
    if (!fp) {
	 fprintf(stderr, "Error Line %d: could not include file :%s\n", G_lineno, input_line);
	 return;
    }

    S_include_stack[S_include_stack_index].yy_buffer = YY_CURRENT_BUFFER;
    S_include_stack[S_include_stack_index].save_lineno = G_lineno;

    S_include_stack_index++;

    if (G_verbose > 0) {
        fprintf(stderr, "-- Including file %s (at line %d)\n", fopened, G_lineno);
    }

    G_lineno = 1;
    yyin = fp;
    S_include_stack[S_include_stack_index].yy_buffer = yy_create_buffer(yyin, YY_BUF_SIZE);
    S_include_stack[S_include_stack_index].save_lineno = -1;
    yy_switch_to_buffer( S_include_stack[S_include_stack_index].yy_buffer );

}

