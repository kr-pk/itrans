/*
 *========================================================================== 
 * Copyright 1991-1992 Avinash Chopde, All Rights Reserved.
 */

#include "itrans.h"
#include "ifm.h"

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/font.c,v 1.10 1998/09/13 00:49:37 avinash Exp $";

/* definitions used by decode_name() */
#define	ALL_CHARS	-1
	/* ALL_CHARS must be zero-1, see decode_name() */
#define	NO_CHAR		-2

/* =================================================================== */
static int S_ccadd(FILE* ifmfp);
static void S_clear_ccadds(void);
static int S_ccs(FILE* ifmfp, font_t *font);
static int S_cc(FILE* ifmfp, font_t *font, int no_oxcy);
static int S_fillup_afm(FILE* afmfp, font_t* font);
static int S_fillup_afm_none(font_t* font);
static int S_fillup_tfm(FILE* tfmfp, font_t* font);

/* The extra_map[] stores user defined khadi names --
 * these are only used in "same-as" (CCS) IFM tag
 * Cannot be generated directly by the user.
 * See also the definition of G_ifm_map ahead in this file.....
 */
static ifm_enc_t S_ifm_extra_map[NUMEXTRA];

/* global variable - need a pointer into current font_t's consmany */
static font_t*	S_current_fillup_font = NULL;

/* =================================================================== */
/* initialise font with empty values */
void init_font(font_t* font)
{
    int i, j;

    font->prop = UNKNOWN_FONT;
    font->name[0] = '\0';
    font->fname[0] = '\0';
    font->pname = NULL;
    font->consmany = NULL;
    font->numconsmany = 0;
    font->maxconsmany = 0;
    font->hyphenchar = -1;
    font->default_vowel = HALF_TOK;

    for (i = 0; i < NUMKHADI; i ++) {
	font->khadi[i].cus = NULL;
	font->khadi[i].same_as = NULL;
    }

    for (i = 0; i < NUMCHARS; i ++) {
        for (j = 0; j < NUMCHARS; j ++) {
	    font->ligatures[i][j].cus = NULL;
	    font->ligatures[i][j].same_as = NULL;
        }
    }
    for (i = 0; i < 10; i ++) {
	font->digits[i] = NULL;
    }

    for (i = 0; i < NUMPSCHARS; i ++) {
	font->psfm[i].w = UNDEF_CHAR_WIDTH;
	font->psfm[i].llx = 0;
	font->psfm[i].lly = 0;
	font->psfm[i].urx = 0;
	font->psfm[i].ury = 0;
    }
} /* init_font() */

/* =================================================================== */
font_t*	find_font(allfonts_t af, char fname[])
{
    font_t* f;
    int i;

#ifdef DEBUG
    fprintf(stderr, "looking for font (%s)\n", fname);
#endif

    f = NULL;
    for (i = 0; i < FONTS_MAX;  i ++) {

#ifdef DEBUG
	if (af[i]) fprintf(stderr, "font at i is (%s)\n", af[i]->fname);
#endif

	if (af[i] && !strcmp(fname, af[i]->fname)) {
	   f = af[i];
#ifdef DEBUG
	   fprintf(stderr, "FOUND FONT!\n");
#endif
	   break;
	}
    }
    return f;
}
/* =================================================================== */
/* fillup the font data by reading in the IFM file supplied */
/* assumed that init_font() has been called before.... */

int fillup_font(font_t* font, char ifmfname[])
{
    int		i;
    char	word[256];
    char	ifmword[256];
    int		ifmtoken;
    FILE*	ifmfp;
    char	fmfname[NAMELEN];
    char	ifmfopened[PATH_MAX];
    FILE*	fmfp = NULL;
    char*	ienv;
    char*	dp;
    int		errflg= 0;
    int		c0, c1, v;
    dcharcm_t	cm;
    int		no_oxcy = TRUE ; /* IFM file does not have ox+- / cy+- deltas */
    
    if (S_current_fillup_font) {
	fprintf(stderr, "Program error (recursive call to fillup_font?)\n");
	return FALSE;
    }

    S_current_fillup_font = NULL;

    /* get the search path... */
    ienv = (char*) getenv(ITRANS_PATH);
    if (!ienv) {
	ienv = strchr(ITRANS_PATH_DEF, '=') + 1;
    }

    ifmfp = search_fopen(ienv, ifmfname, "r", ifmfopened);
    if (!ifmfp) {
	 fprintf(stderr, "Error: could not open %s for reading\n", ifmfname);
	 return FALSE;
    }

    if (G_verbose > 0) {
	 if (strcmp(ifmfname, ifmfopened))
	     fprintf(stderr, " (reading IFM file %s) ", ifmfopened);
    }

    S_current_fillup_font = font;

	/* -------- FILLUP khadi */
	/* FILL UP THE DEVNAGARI CHARS */

    reset_pifm(); /* reset the IFM parser, set line count to zero etc */

    while ( !errflg && (ifmtoken = get_ifm_token(ifmfp, ifmword)) != 0) {
	/* line format is:
	   itrans: CC <ifm> n ; PCC <pscharnum> <deltax> <deltay> ;
	   itrans: CCADD <newifmname> ;
	   itrans: CCS <ifmname> <otherifmname> ;
	 */

	switch (ifmtoken) {
	case PSF_IFMTAG:

	    /* example: itrans: POSTSCRIPT marathi devnac.ps devnac.afm */
	    ifmtoken = get_ifm_token(ifmfp, ifmword);
	    if (ifmtoken != DNAME_IFMTAG) {
		fprintf(stderr, "Error in parsing ifm font metrics - expect a lang name after POSTSCRIPT\n");
	        errflg++; break;
	    }
	    strcpy(word, ifmword);
	    ifmtoken = get_ifm_token(ifmfp, ifmword);
	    if (ifmtoken != DNAME_IFMTAG) {
		fprintf(stderr, "Error in parsing ifm font metrics - expect a filename after POSTSCRIPT %s\n", word);
	        errflg++; break;
	    }
	    /* the PostScript font program file name ... */
	    i = strlen(ifmword) + 1;
	    font->pname = (char*) malloc(i * sizeof(char));
	    strcpy(font->pname, ifmword);

	    /* get the AFM file name */
	    ifmtoken = get_ifm_token(ifmfp, ifmword);
	    if (ifmtoken != NONE_IFMTAG && ifmtoken != DNAME_IFMTAG) {
		fprintf(stderr, "Error in parsing ifm font metrics - expect a filename (or 'none') after POSTSCRIPT [language] %s ...\n", word);
	        errflg++; break;
	    }
	    strcpy(fmfname, ifmword);

	    /* strip of path name stuff, and copy the ifm file name */
	    dp = strrchr(ifmfname, DIRSEP);
	    if (!dp) dp = ifmfname;
	    else dp++;
	    strcpy(font->fname, dp);

	    strcpy(font->name, word);
	    /* decode language name */
	    font->lang_tok = get_lang_token(word);
	    if (font->lang_tok < 0) {
		fprintf(stderr, "Warning: unrecognizable language [%s] in FONT statement\n", word);
	    }
	    
	        /* -------- FILLUP ENCODING NAMES */
	        /* XXX - fill up font with G_ifm_map - maybe different
		 * fonts will use different names for ka, kha, etc!
		 * I hope not.
		 */
            for (i = 0; i < NUMCHARS; i ++) {
	        font->enc[i] = G_ifm_map[i];
            }

		/* fill in font Metrics */

	    if (ifmtoken == NONE_IFMTAG) {

		/* no file to read in */
		fmfp = NULL;
		S_fillup_afm_none(font); /* nulls the data cheat! */
	        font->prop = PS_FONT; /* use font for PostScript
					* output only - AFM file is
					* missing, so no TFM file can be
					* present, too.
					*/

	    }  else {

		/* fill in PostScript Metrics */
	        fmfp = search_fopen(ienv, fmfname, "r", NULL);
	        if (!fmfp) {
		    fprintf(stderr, "font.c::Error: could not open %s for reading (afm)\n", fmfname);
		    errflg++; break;
	        } 
 		S_fillup_afm(fmfp, font);
	        fclose(fmfp);
	        font->prop = TEX_PS_FONT; /* use font for both PostScript
					   * and TeX output
					   * Since the AFM file is present,
					   * the TFM file should also be
					   * available (the program
					   * afm2tfm in the DVIPS package)
					   */
		no_oxcy = FALSE; /* got metrics, so ok to use ox+- or cy+- */
	    }

	    break;

	case TXTF_IFMTAG:
	case UNICODEF_IFMTAG:

 	    /* example: itrans: TEXT_TTY roman */
	    i = get_ifm_token(ifmfp, ifmword);
	    if (i != DNAME_IFMTAG) {
		fprintf(stderr, "Error in parsing ifm font metrics - expect a lang name after TEXT_TTY or TEXT_UNICODE\n");
	        errflg++; break;
	    }
	    strcpy(word, ifmword);
	    font->pname = "";

	    /* strip of path name stuff, and copy the ifm file name */
	    dp = strrchr(ifmfname, DIRSEP);
	    if (!dp) dp = ifmfname;
	    else dp++;
	    strcpy(font->fname, dp);

	    strcpy(font->name, word);
	    /* decode language name */
	    font->lang_tok = get_lang_token(word);
	    if (font->lang_tok < 0) {
		fprintf(stderr, "Warning: unrecognizable language [%s] in FONT statement\n", word);
	    }
	    
	        /* -------- FILLUP ENCODING NAMES */
	        /* XXX - fill up font with G_ifm_map - maybe different
		 * fonts will use different names for ka, kha, etc!
		 * I hope not.
		 */
            for (i = 0; i < NUMCHARS; i ++) {
	        font->enc[i] = G_ifm_map[i];
            }

	    /* fill in font Metrics, no file to read in */
	    fmfp = NULL;
	    S_fillup_afm_none(font); /* nulls data -  cheat! */
	    if (ifmtoken == TXTF_IFMTAG) font->prop = TXT_TEX_PS_FONT; /* supports Text/TeX/PS output */
	    if (ifmtoken == UNICODEF_IFMTAG) font->prop = UNICODE_FONT; /* supports UNICODE only output */

	    break;

	case TEXF_IFMTAG:

		/* example: itrans: TEX sanskrit dvng10.tfm */
		/* implies font is Metafont desc, supply any TFM file
		 * name of an existing file
		 */

	    ifmtoken = get_ifm_token(ifmfp, ifmword);
	    if (ifmtoken != DNAME_IFMTAG) {
		fprintf(stderr, "Error in parsing ifm font metrics - expect a lang name after TEX\n");
	        errflg++; break;
	    }
	    strcpy(word, ifmword);

	    /* get the TFM file name */
	    ifmtoken = get_ifm_token(ifmfp, ifmword);
	    if (ifmtoken != NONE_IFMTAG && ifmtoken != DNAME_IFMTAG) {
		fprintf(stderr, "Error in parsing ifm font metrics - expect a filename (or 'none') after TEX %s ...\n", word);
	        errflg++; break;
	    }
	    strcpy(fmfname, ifmword);


	    /* strip of path name stuff, and copy the IFM file name */
	    dp = strrchr(ifmfname, DIRSEP);
	    if (!dp) dp = ifmfname;
	    else dp++;
	    strcpy(font->fname, dp);

	    strcpy(font->name, word);
	    /* decode font name */
	    font->lang_tok = get_lang_token(word);
	    if (font->lang_tok < 0) {
		fprintf(stderr, "Warning: unrecognizable language [%s] in TEX statement\n", word);
	    }
	    
	        /* -------- FILLUP ENCODING NAMES */
	        /* XXX - fill up font with G_ifm_map - maybe different
		 * fonts will use different names for ka, kha, etc!
		 * I hope not.
		 */
            for (i = 0; i < NUMCHARS; i ++) {
	        font->enc[i] = G_ifm_map[i];
            }


	    font->prop = TEX_FONT; /* use font for TeX output only */
		/* fill in font Metrics */
	    if (ifmtoken == NONE_IFMTAG) {
		/* no file to read in */
		fmfp = NULL;
		S_fillup_afm_none(font); /* nulls the data */
		if (G_verbose >= 2) {
		    /* don't print this message all the time because most
		    IFM files do not use ox+- and cy+-
		    */
		    fprintf(stderr, "Warning: no TFM file name supplied for this TeX font - may create problems if the IFM file (%s) uses ox+- or cy+- coordinates,\n",
			ifmfname);
		    fprintf(stderr, "   or if the IFM file requires 0 width glyphs to have deltas reapplied.\n");
	        }
	    }  else {

		/* fill in TFM Metrics (width only) */
	        fmfp = search_fopen(ienv, fmfname, READBIN_MODE, NULL);
	        if (!fmfp) {
		    fprintf(stderr, "font.c::Error: could not open %s for reading (tfm)\n", fmfname);
		    errflg++; break;
	        } 
		S_fillup_tfm(fmfp, font);
	        fclose(fmfp);
		no_oxcy = FALSE; /* got metrics, so ok to use ox+- or cy+- */
	    }
	    break;

	case HCHAR_IFMTAG:
	    ifmtoken = get_ifm_token(ifmfp, ifmword);
	    if (ifmtoken != NUMBER_IFMTAG) {
		fprintf(stderr, "Error in parsing ifm font metrics - expect a number after HYPHENCHAR\n");
	        errflg++; break;
	    }
	    if (sscanf(ifmword, "%d", &ifmtoken) != 1) {
		fprintf(stderr, "Warning: (line %d) illegal value in IFM file (looking for HYPHENCHAR number)\n", G_ifm_lineno);
		errflg++; goto get_out; /* return FALSE; */
	    }
	    font->hyphenchar = ifmtoken;

	    break;

	case DEFVOWEL_IFMTAG:
	    ifmtoken = get_ifm_token(ifmfp, ifmword);
	    if (ifmtoken != DNAME_IFMTAG) {
		fprintf(stderr,
		"Error in parsing indian font metrics - expect a vowel after DEFAULTVOWEL\n");
		errflg++; goto get_out; /* return FALSE; */
	    }

	    /* decode ifm name */
	    i = decode_name(ifmword, &c0, &c1, &cm, &v);
#ifdef DEBUG
	    fprintf(stderr, "got def vowel :%s: type is %d v %d c0 %d c1 %d\n",
			 ifmword, i, v, c0, c1);
#endif
	    if (i == VOWEL_TYPE) {
		v = _T_(c0); /* the vowel is ok, just convert form to tok */
	    } else {
		if (!strcmp(ifmword, "half"))  {
		    /* half_tok is ok */
		    v = HALF_TOK;
		} else {
		    fprintf(stderr,
		    "Error in parsing indian font metrics - vowel incorrect after DEFAULTVOWEL\n");
		    errflg++; goto get_out; /* return FALSE; */
		}
	    }

#ifdef DEBUG
	    fprintf(stderr, "got def vowel %d\n", v);
#endif

	    font->default_vowel = v;

	    break;

	case CCADD_IFMTAG:
	    if (!S_ccadd(ifmfp)) {
		fprintf(stderr,"**** line %d - illegal CCADD line (last token %s)\n",
			G_ifm_lineno, ifmword);
	        errflg++;
	    }
	    break;

	case CCS_IFMTAG:
	    if (!S_ccs(ifmfp, font)) {
		fprintf(stderr,"**** line %d - illegal CCS line (last token %s)\n",
			G_ifm_lineno, ifmword);
	        errflg++;
	    }
	    break;
	case CC_IFMTAG:
	    if (!S_cc(ifmfp, font, no_oxcy)) {
		fprintf(stderr,"**** line %d - illegal CC line (last token %s)\n",
			G_ifm_lineno, ifmword);
	        errflg++;
	    }
	    break;
	case COMMENT_IFMTAG:
	    break;
	default:
		fprintf(stderr,"**** around line %d (+-1) - warning: unrecognized IFM token %s\n",
			G_ifm_lineno, ifmword);
		if (ifmtoken == PCC_IFMTAG)
		    fprintf(stderr,"**** around line %d (+-1) - warning: maybe the CC count is incorrect -- too many PCC's\n",
			G_ifm_lineno);
	    break;
	} /* switch */

    } /* while get_ifm_token(ifmfp, ifmword) - reading in IFM statements */

    /* clear any CCADD's this file may have added, so that each IFM
     * file can use upto NUMEXTRA CCADD codenames.
     * Without this clearing, the TOTAL number of CCADD statements in
     * ALL the IFM files is limited to NUMEXTRA, this way, EACH file
     * can use upto NUMEXTRA codenames...
     */
    S_clear_ccadds();

get_out:

    S_current_fillup_font = NULL;

#ifdef DEBUG
    /* for debugging, print out widths of characters in the font */
    fprintf(stderr, "Printing Zero Width Characters in %s\n", font->name);
    for (i = 0 ; i < NUMPSCHARS; i ++ ) {

	if (font->psfm[i].w == UNDEF_CHAR_WIDTH) 
	    continue;

	if (font->psfm[i].w == 0) {
	    fprintf(stderr, "TFMMetric: char %d is of zero width\n", i);
	}
    }
#endif

    if (!errflg) return TRUE;
    else return FALSE;

} /* fillup_font() */
    
/* ==================================================================== */
static int S_fillup_afm(FILE* afmfp, font_t* font)
{
    int		i;
    pschar_t*	psch;
    char	line_in[LINELEN], *inc;

    /* fill up the postscript font metrics */

    psch = font->psfm;
    while (fgets(line_in, LINELEN, afmfp)) {
	/* line format is: C 33; WX 301; N exclam; B 93 -9 207 685; */

	if (line_in[0] != 'C' || line_in[1] != ' ') continue;
	inc = &line_in[1];

	    /* read in character code */
	if (sscanf(inc, "%d", &i) != 1 || i < 0 || i >= NUMPSCHARS) {
 
 	    if (i != -1)
 	        fprintf(stderr, "Warning: illegal line [%s] in AFM file\n", line_in);
 
	    continue;
	}

	inc = strchr(inc, 'W'); /* look for W.... */
	inc = strchr(inc, ' '); /* look for <space> after W.... */

	sscanf(inc, "%d", &psch[i].w); /* read in width */

	inc = strrchr(inc, 'B'); /* look for ....B */
	inc = strchr(inc, ' '); /* look for <space> after ..B */

	    /* read in bounding box */
	if (sscanf(inc, "%d%d%d%d", &psch[i].llx, &psch[i].lly, &psch[i].urx, &psch[i].urx) != 4) {
	    fprintf(stderr, "Warning: illegal B values - line [%s] in AFM file\n", line_in);
	}

    } /* while fgets() */

    return TRUE;
}
/* ==================================================================== */
/* this func fills up the font metrics with default values */
static int S_fillup_afm_none(font_t* font)
{
    pschar_t*	psch;
    int i;

    psch = font->psfm;
    for (i = 0 ; i < NUMPSCHARS; i ++ ) {

	psch[i].w = UNDEF_CHAR_WIDTH; /* just some non-zero value, unused really..*/
			/* make it non-zero so that ichar.c::S_add_cus()
			 * is fooled into thinking that all chars have
			 * non-zero width, and it does not have to apply
			 * a reverse delta to back up
			 * Maybe this backup on zero width chars
			 * was a bad idea anyway..
			 */
	psch[i].llx = psch[i].lly = psch[i].urx = psch[i].urx = 0;

    }
    return TRUE;
}
/* ==================================================================== */
/* this func reads in the TFM file, and fills in the char widths */
static int S_fillup_tfm(FILE* tfmfp, font_t* font)
{
    pschar_t*	psch;
    int i;

    /* fill up the TeX font metrics */

    psch = font->psfm;
    for (i = 0 ; i < NUMPSCHARS; i ++ ) {

	psch[i].w = UNDEF_CHAR_WIDTH; /* just some non-zero value, unused really..*/
			/* make it non-zero so that ichar.c::S_add_cus()
			 * is fooled into thinking that all chars have
			 * non-zero width, and it does not have to apply
			 * a reverse delta to back up
			 * Maybe this backup on zero width chars
			 * was a bad idea anyway..
			 */
	psch[i].llx = psch[i].lly = psch[i].urx = psch[i].urx = 0;

    }

    /* now load in the widths of characters defined in the TFM file */
    tfmload(tfmfp, font, 1000); /* TFM widths are in fractions
				 * of the point size, for us, we use
				 * 1000 units to a EMSIZE, so
				 * scale by 1000.
				 */

    return TRUE;
}
/* ==================================================================== */
/* line format:
itrans: CCADD <newifmname> ;
* can also be used to add CONSONANT_MANY tags:
itrans: CCADD sa-dha-ra ; 
*/
static int S_icurrextra = 0;
static int S_ccadd(FILE* ifmfp)
{

    int ifmtoken;
    char word[256];
    int cctype, ccl0, ccl1, ccform;
    char	ifmword[256];
    dcharcm_t	cccm;

    ifmtoken = get_ifm_token(ifmfp, ifmword);
    if (ifmtoken != DNAME_IFMTAG) {
	fprintf(stderr, "Error in parsing indian font metrics - expect a word after CCADD\n");
        return FALSE;
    }
    strcpy(word, ifmword);

    /* decode ifm name */
    cctype = decode_name(word, &ccl0, &ccl1, &cccm, &ccform);
    if (cctype) {
	/* some other ifm file also used this codename, so no need to add
	 * it again....
	 */
	fprintf(stderr, "Note: new codename :%s: - already exists in khadi\n", word);
	return TRUE;
    }

    if (cccm.count >= 3) {
	/* add a new cons many */
	if (S_current_fillup_font->numconsmany >= S_current_fillup_font->maxconsmany) {
	    /* need to allocate more, run out */
	    int	n = S_current_fillup_font->numconsmany + CONSMANY_ALLOC_SIZE;
	    int size = n*sizeof(dcharcm_t); /* allocate these many bytes */

	    S_current_fillup_font->consmany =
		(dcharcm_t*) my_realloc(S_current_fillup_font->consmany, size);
	    if ( !S_current_fillup_font->consmany ) {
		fprintf(stderr, "Program error (realloc failed ?): S_ccadd () got NULL consmany\n");
		return FALSE;
	    }
	    S_current_fillup_font->maxconsmany = n;
	}
	ccl0 = S_current_fillup_font->numconsmany;
	S_current_fillup_font->consmany[ccl0] = cccm;
	strncpy( S_current_fillup_font->consmany[ccl0].word, word, 
		    sizeof( S_current_fillup_font->consmany[ccl0].word));
	S_current_fillup_font->consmany[ccl0].word[
	    sizeof( S_current_fillup_font->consmany[ccl0].word ) - 1] = '\0';

	S_current_fillup_font->numconsmany++;

	/*
	fprintf(stderr, "CCADD: cons many added at location %d\n", ccl0);
	*/
	
	/* validity check */
	if (ccform != NO_CHAR) {
	    fprintf(stderr,"**** line %d - illegal CCADD line CONS MANY should not have a vowel (%d)\n",
		    G_ifm_lineno, ccform);
	    return FALSE;
	}

	return TRUE;
    }

    /* add in ifm_extra map */

    /* check for overflow ... */
    if (S_icurrextra >= NUMEXTRA) {
	fprintf(stderr, "(line %d) Fatal Error in parsing indian font metrics - too many user-defined codenames\n", G_ifm_lineno);
	exit(10);
    }

    S_ifm_extra_map[S_icurrextra].token = LASTCHAR+1 + S_icurrextra;
    strncpy(S_ifm_extra_map[S_icurrextra].codename, word, CNAMELEN);
    S_ifm_extra_map[S_icurrextra].codename[CNAMELEN-1] = '\0';
    S_icurrextra++;
#ifdef DEBUG
fprintf(stderr, "added CCADD tag, word is %s, tok %d (num %d)\n", S_ifm_extra_map[S_icurrextra-1].codename,S_ifm_extra_map[S_icurrextra-1].token, S_icurrextra);
#endif /* DEBUG */
    return TRUE;

} /* S_ccadd */

/* clear all the codenames added here (to prepare for the next IFM
 * file, this function called at the end of fillup_font()
 */
static void S_clear_ccadds()
{
    int i;

    S_icurrextra = 0;
    for (i = 0; i < NUMEXTRA; i ++) {
	S_ifm_extra_map[i].codename[0] = '\0';
	S_ifm_extra_map[i].token = -999;
    }
}
/* ==================================================================== */
/* line format:
itrans: CC <ifmname> n ; PCC <pscharnum> <deltax> <deltay> ;
 */
static int S_cc(FILE* ifmfp, font_t *font, int no_oxcy)
{
    int i, j, ifmtoken;
    char word[256];
    int cctype, ccl0, ccl1, ccform;
    dcharcm_t	cccm;
    int isconsd, isconsmany, isdgit, iskhadi;
    comp_unit_t*	pcu = NULL;
    comp_unit_t**	ppcu = NULL;
    int numpcc, pcode, dx, dy;
    char	ifmword[256];

    ifmtoken = get_ifm_token(ifmfp, ifmword);
    if (ifmtoken != DNAME_IFMTAG) {
	fprintf(stderr, "(line %d, token %d) Error in parsing indian font metrics - expect a word after CC\n", G_ifm_lineno, ifmtoken);
        return FALSE;
    }
    strcpy(word, ifmword);

    cctype = decode_name(word, &ccl0, &ccl1, &cccm, &ccform);
    if (!cctype) {
	fprintf(stderr, "Error: CC: illegal codename :%s:\n", word);
	return FALSE;
    }

    isconsd = FALSE; isdgit = FALSE; iskhadi = FALSE; isconsmany = FALSE;
    if (cctype == CONSONANT_DOUBLE_TYPE) isconsd = TRUE;
    else if (cctype == DIGIT_TYPE) isdgit = TRUE;
    else if (cctype == CONSONANT_SINGLE_TYPE) iskhadi = TRUE;
    else if (cctype == VOWEL_TYPE) iskhadi = TRUE;
    else if (cctype == SPECIAL_TYPE) iskhadi = TRUE;
    else if (cctype == CONSONANT_MANY_TYPE) isconsmany = TRUE;
    else {
	fprintf(stderr, "Error: CC: illegal codename, ;%s: line %d\n", word, G_ifm_lineno);
	return FALSE;
    }

#ifdef DEBUG
    fprintf(stderr, "name: %s, decoded %d %d %d type %d\n", word, ccl0, ccl1, ccform,cctype);
#endif /* DEBUG */

	/* look for n - number of PCC segments */
    ifmtoken = get_ifm_token(ifmfp, ifmword);
    if (ifmtoken != NUMBER_IFMTAG) {
	fprintf(stderr, "(line %d, token %d yytesxt %s) Error in parsing indian font metrics - expect a number after CC word.. \n", G_ifm_lineno, ifmtoken, ifmword);
        return FALSE;
    }
    word[0] = 0; strcpy(word, ifmword);
    if (sscanf(word, "%d", &numpcc) != 1) {
	fprintf(stderr, "Warning: (line %d) illegal value in IFM file (looking for number of PCC segments)\n", G_ifm_lineno);
	return FALSE;
    }

    if (isconsd) {
	ppcu = font->ligatures[ccl0][ccl1].cus;
    } else if (iskhadi) {
	ppcu = font->khadi[ccl0].cus;
    } else if (isdgit) {
	ppcu = &font->digits[ccl0];
    } else if (isconsmany) {
	ppcu = font->consmany[ccl0].cus;
    }

    if (!ppcu) { /* first time, allocate */
	ppcu = (comp_unit_t**) malloc(NUMFORMS*sizeof(*ppcu));
	if (isconsd) {
	    font->ligatures[ccl0][ccl1].cus = ppcu;
	} else if (iskhadi) {
	    font->khadi[ccl0].cus = ppcu;
	} else {
	    fprintf(stderr, "Program error: ppcu is NULL and not khadi/consd line %d\n", G_ifm_lineno);
	    return FALSE;
	}
	for (i = 0; i < NUMFORMS; i ++) {
	    ppcu[i] = (comp_unit_t*) NULL;
	}
    }
    /* pcu = (comp_unit_t*) malloc(numpcc*sizeof(*pcu)); */
    pcu = (comp_unit_t*) malloc(numpcc*sizeof(comp_unit_t));
    if (isconsd) {
	font->ligatures[ccl0][ccl1].cus[ccform] = pcu;
    /*
    fprintf(stderr, "fillip: filling ligature %d %d %d\n", ccl0, ccl1, ccform);
    */
    } else if (iskhadi) {
	font->khadi[ccl0].cus[ccform] = pcu;
    } else if (isdgit) {
	/*
        fprintf(stderr, "filling up digits: %d numpcc %d\n", ccl0, numpcc);
	*/
	font->digits[ccl0] = pcu;
    } else if (isconsmany) {
	font->consmany[ccl0].cus[ccform] = pcu;
	/*
        fprintf(stderr, "filling up consmany: %d (%s) form %d numpcc %d\n",
			    ccl0, font->consmany[ccl0].word, ccform, numpcc);
        */
    } else {
	fprintf(stderr, "Program error: cant handle line %d\n", G_ifm_lineno);
	return FALSE;
    }

#ifdef DEBUG
    fprintf(stderr, "filling up PCC's for ccl0 %d l1 %d form %d type %d\n",
    ccl0, ccl1, ccform, cctype);
#endif /* DEBUG */

    for (i = 0; i < numpcc; i ++) {

	/* look for " ; PCC" */
        ifmtoken = get_ifm_token(ifmfp, ifmword);
        if (ifmtoken != PCC_IFMTAG) {
	    fprintf(stderr, "(line %d, token %d ifmword %s)Error in parsing indian font metrics - expect a PCC tag\n", G_ifm_lineno, ifmtoken, ifmword);
            return FALSE;
        }

	    /* defaults */
	pcu->u_pschar = NO_PSCHAR;
	pcu->deltax = 0;
	pcu->deltay = 0;
	pcu->dtype_x = D_CUR; /* see itrans.h - default for x axis */
	pcu->dtype_y = D_ORG; /* see itrans.h - default for y axis */
	if (i == (numpcc - 1)) pcu->next = NULL; /* last comp_unit ... */
	else pcu->next = pcu + 1; /* point to next comp_unit ... */

        ifmtoken = get_ifm_token(ifmfp, ifmword);
        word[0] = 0; strcpy(word, ifmword);
	if (ifmtoken == IMPLICIT_IFMTAG) {
	    pcode = IMPLICIT_PSCHAR;
	} else if (ifmtoken == NONE_IFMTAG) {
	    pcode = NO_PSCHAR;
	} else if (ifmtoken == NUMBER_IFMTAG) {
	    j = sscanf(word, "%i", &pcode); /* octal, hex, or base 10 int */
	    if (pcode < 0 || (pcode > 255 && ! (font->prop & UNICODE_FONT)) ) {
		fprintf(stderr, "(line %d, token %d %s)Error in IFM file. PCC [chacode] is less than 0 or is gt 255 and not UNICODE\n", G_ifm_lineno, ifmtoken, ifmword);
		return FALSE;
	    }
	} else {
	    fprintf(stderr, "(line %d, token %d %s)Error in parsing indian font metrics - expect a number after PCC tag\n", G_ifm_lineno, ifmtoken, ifmword);
	    return FALSE;
        }

	    /* point to dx, could be %d or ox+%d ox-%d */
	ifmtoken = get_ifm_token(ifmfp, ifmword);
	if (ifmtoken == DORG_IFMTAG) {
	    pcu->dtype_x = D_ORG;
	    if (no_oxcy) {
		fprintf(stderr, "Error in parsing indian font metrics - PCC ox but no TFM file specified\n");
		return FALSE;
	    }
	} else if (ifmtoken != NUMBER_IFMTAG) {
	    fprintf(stderr, "Error in parsing indian font metrics - expect a number after PCC ...\n");
            return FALSE;
        }
        word[0] = 0; strcpy(word, ifmword);
	j = sscanf(word, "%d", &dx);

	    /* point to dy, could be %d or cy+%d cy-%d */
	ifmtoken = get_ifm_token(ifmfp, ifmword);
	if (ifmtoken == DCUR_IFMTAG) {
	    if (no_oxcy) {
		fprintf(stderr, "Error in parsing indian font metrics - PCC cy but no TFM file specified\n");
		return FALSE;
	    }
	    pcu->dtype_y = D_CUR;
	} else if (ifmtoken != NUMBER_IFMTAG) {
	    fprintf(stderr, "Error in parsing indian font metrics - expect a number after PCC ... dx ..\n");
            return FALSE;
        }
        word[0] = 0; strcpy(word, ifmword);
	j += sscanf(word, "%d", &dy);

	if (j != 2) {
	    fprintf(stderr, "Warning: IFM file - illegal dx/dy values-\n");
	} else {
	    pcu->u_pschar = pcode;
	    pcu->deltax = dx;
	    pcu->deltay = dy;
	}

	/* check for errors: if NO_PSCHAR, then dy must be zero */
	if (pcode == NO_PSCHAR && dy != 0 && pcu->dtype_x == D_ORG) {
	    fprintf(stderr, "(line %d)Error: PCC none .. must have dy == 0\n",
			G_ifm_lineno);
	    return FALSE;
	}
	/* check for errors: if implicit_form, then dx and dy must be 0 0
	 * since the code cannot handle any dx, dy for implicit definitions.
	 * Workaround is to use PCC none dx ...  before PCC implicit 
	 * in the IFM file. No workaround for dy....
	 */
	if (pcode == IMPLICIT_PSCHAR && (dy != 0 || dx != 0)) {
	    fprintf(stderr, "(line %d)Error: PCC implicit .. must have dx, dy == 0\n",
			G_ifm_lineno);
	    return FALSE;
	}

#ifdef DEBUG
    fprintf(stderr, "pcode %d dx %d dy %d\n", pcode, dx, dy);
#endif /* DEBUG */

	pcu++;
    } /* for i numpcc */

    return TRUE;
} /* S_cc */
/* ==================================================================== */
/* line format: itrans: CCS <ifmname> <otherifmname> ; */
static int S_ccs(FILE* ifmfp, font_t *font)
{
    int ifmtoken;
    char word[256];
    int cctype, ccl0, ccl1, ccform;
    dcharcm_t	cccm;
    int pcctype, pccl0, pccl1, pccform;
    dcharcm_t	pcccm;
    dchar_t*		dptr;
    int			i;
    char	ifmword[256];

    ifmtoken = get_ifm_token(ifmfp, ifmword);
    if (ifmtoken != DNAME_IFMTAG) {
	fprintf(stderr, "(line %d, token %d) Error in parsing indian font metrics - expect a word after CCS\n", G_ifm_lineno, ifmtoken);
        return FALSE;
    }
    strcpy(word, ifmword);

    cctype = decode_name(word, &ccl0, &ccl1, &cccm, &ccform);
    if (!cctype) {
	fprintf(stderr, "Error: CCS:cc: illegal codename :%s:\n", word);
	return FALSE;
    }
    if (cctype == DIGIT_TYPE) { /* error */
	fprintf(stderr, "Error: [CCS] (line %d) cannot use a CCS stmt for a digit\n", G_ifm_lineno);
	return FALSE;
    }

#ifdef DEBUG
fprintf(stderr, "got CCS tag, word is %s type %d\n", word, cctype);
#endif /* DEBUG */

    ifmtoken = get_ifm_token(ifmfp, ifmword);
    if (ifmtoken != DNAME_IFMTAG) {
	fprintf(stderr, "(line %d, token %d) Error in parsing indian font metrics - expect a second word after CCS\n", G_ifm_lineno, ifmtoken);
        return FALSE;
    }
    strcpy(word, ifmword);

    pcctype = decode_name(word, &pccl0, &pccl1, &pcccm, &pccform);
    if (!pcctype) {
	fprintf(stderr, "Error: CCS: pcc: illegal codename :%s:\n", word);
	return FALSE;
    }
    if (pcctype == DIGIT_TYPE || pcctype == CONSONANT_MANY_TYPE) { /* error */
	fprintf(stderr, "Error: [CCS] (line %d) cannot point a CCS stmt to a digit or consmany\n", G_ifm_lineno);
	return FALSE;
    }

#ifdef DEBUG
fprintf(stderr, "otherifmname is %s \n", word);
#endif /* DEBUG */
    dptr = NULL;
    if (pcctype == CONSONANT_DOUBLE_TYPE) {
	if (pccl0 == ALL_CHARS || pccl1 == ALL_CHARS) {
	    fprintf(stderr, "Error: [otherifmname] (line %d) in CCS should not contain *'s\n", G_ifm_lineno);
	    return FALSE;
	} else {
	    dptr = &font->ligatures[pccl0][pccl1];
	}
    } else if (pcctype == CONSONANT_SINGLE_TYPE) {
	if (cctype == CONSONANT_SINGLE_TYPE && pccl0 == ALL_CHARS) {
	    fprintf(stderr, "Error: [otherifmname] (line %d) in CCS must be a single khadi cannot be *.\n", G_ifm_lineno);
	    return FALSE;
	} else {
	    if (pccl0 != ALL_CHARS) dptr = &font->khadi[pccl0];
	}
    } else {
	/* NOTE: CCS stmt cannot have the second word be a consonant many */
	fprintf(stderr, "Error: CCS: cannot handle second codename, line %d\n", G_ifm_lineno);
    }

    if (cctype == CONSONANT_DOUBLE_TYPE) {
#ifdef DEBUG
fprintf(stderr, "ligatures %d %d %d same as %d %d\n", ccl0, ccl1, ccform, pccl0, pccl1);
#endif /* DEBUG */
	if (ccl0 == ALL_CHARS) {
	    if (pccl0 == ALL_CHARS) {
		for (i = FIRSTCONS; i < NUMCHARS; i ++)
		    font->ligatures[i][ccl1].same_as = &font->khadi[i];
	    } else {
		for (i = FIRSTCONS; i < NUMCHARS; i ++)
		    font->ligatures[i][ccl1].same_as = dptr;
	    }
	} else if (ccl1 == ALL_CHARS) {
	    if (pccl0 == ALL_CHARS) {
		for (i = FIRSTCONS; i < NUMCHARS; i ++)
		    font->ligatures[ccl0][i].same_as = &font->khadi[i];
	    } else {
		for (i = FIRSTCONS; i < NUMCHARS; i ++)
		    font->ligatures[ccl0][i].same_as = dptr;
	    }
	} else {
	    font->ligatures[ccl0][ccl1].same_as = dptr;
	}
    } else if (cctype == CONSONANT_SINGLE_TYPE) {
#ifdef DEBUG
fprintf(stderr, "khadi %d %d same as %d %d\n", ccl0, ccl1, pccl0, pccl1);
#endif /* DEBUG */
	font->khadi[ccl0].same_as = dptr;
    } else if (cctype == CONSONANT_MANY_TYPE) {
	font->consmany[ccl0].same_as = dptr;
#ifdef DEBUG
fprintf(stderr, "CCS: consmany %s (at %d) same as %d\n", font->consmany[ccl0].word, ccl0, pccl0);
#endif /* DEBUG */
    } else {
	fprintf(stderr, "Error: CCS: cannot handle codename, line %d\n", G_ifm_lineno);
    }

    return TRUE;
} /* S_ccs() */

/* ==================================================================== */
/* Given a word referring to a full indian language character, decode
 * it into its letter and form parts.
 * For ex:
 * ka-i		is letter "ka" _I_(KA_TOK) and form "i" (I_FORM)
 * a		is letter "a" _I_(A_TOK) and form "a" (A_FORM)
 * am		is letter "am" _I_(AM_TOK) and form "a" (A_FORM)
 * ksha-half	is letter "ksha" _I_(KSHA_TOK) and form (HALF_FORM)
 * da-dha	is (half) letter "da" _I_(DA_TOK) and full letter dha (DHA_TOK)
 * three	is digit type - scanned from the array G_ifm_digit_map.
 * etc
 * Function returns type: VOWEL_TYPE, CONSONANT_SINGLE_TYPE, 
 * CONSONANT_DOUBLE_TYPE, CONSONANT_MANY_TYPE, or 0 for error/not found
 * note: for CONSONANT_MANY, this function uses S_current_fillup_font
 * variable to find the index of the consmany, if present.
 */

#if 0 /* ---------------------------------------------- */
/* old_decode_name around just to test the new routine */
static int old_decode_name(char inword[], int* firstc, int* secondc, int* form);
#endif /*0*/

int decode_name(char inword[], int* firstc, int* secondc, dcharcm_t* cm, int* form)
{
    char word[1024], word1[1024];
    char *start;
    int i, ii, c, l0, l1, f, t;
    dcharcm_t	tcm;

    word[0] = word1[0] = '\0';
    l0 = l1 = f = NO_CHAR;
    t = 0;
    *firstc = *secondc = *form = NO_CHAR;
    tcm.count = 0;
    tcm.same_as = NULL;
    tcm.word[0] = '\0';
    for (i = 0; i < NUMFORMS; i ++) {
	tcm.cus[i] = NULL;
    }
    *cm = tcm;

    strcpy(word, inword);
    start = strtok(word, "-");
    if (!start) return 0; /* ERROR */
    strcpy(word1, start); /* first word is in here now */

    /*
    fprintf(stderr, "in decode_name: %s\n", inword);
    */

    ii = 0;
    while (start && word1[0] && ii <= CONS_MAX) {

	c = NO_CHAR;

	/* decode: could be vowel, cons, or user-defined cons (extra_map)*/

	for (i = FIRSTVOW; i <= LASTVOW; i ++) { /* is it normal vowel */
	    if (!strcmp(G_ifm_map[i].codename, word1)) {
		f = c = i;
		if (ii == 0) { /* first word is a vowel */
		    if (t) {
			fprintf(stderr, "Error: decode_name: got a single vowel, and t is non-zero %d (inword %s, word1 %s)\n", t, inword, word1);
		    }
		    l0 = c;
		    f = IMPLICIT_FORM; /* vowels are always one word only */
		    l1 = NO_CHAR;
		    t = VOWEL_TYPE;
		}
		break;
	    }
	}
	for (i = FIRSTCONS; i <= LASTCHAR; i ++) { /* normal consonant or special */
	    if (!strcmp(G_ifm_map[i].codename, word1)) {
		c = i;
		break;
	    }
	}
	if (c < 0) { /* not a vowel or cons */
	    for (i = 0; i < NUMEXTRA; i ++) { /* check if extra_map? */
		if (!strcmp(S_ifm_extra_map[i].codename, word1)) {
		    c = S_ifm_extra_map[i].token;
		    /* must be consonant single type */
		    l0 = c;
		    l1 = NO_CHAR;
		    t = CONSONANT_SINGLE_TYPE;
		    break;
		}
	    }
	    if (c < 0) { /* check for specials: digits, punctuation, etc */
		for (i = 0; i < 10; i ++) {
		    if (!strcmp(G_ifm_digit_map[i].codename, word1)) {
			c = G_ifm_digit_map[i].token;
			l0 = c;
			t = DIGIT_TYPE;
			break;
		    }
		}
	    }
	}
	if (c < 0) {
	    if (!strcmp("half", word1)) {
		if (f != NO_CHAR) {
		    fprintf(stderr, "Error: decode_name, looking for a form, got a half ? f %d, %d (inword %s, word1 %s)\n", f, c, inword, word1);
		}
		f = c = HALF_FORM;
	    } else if (!strcmp("implicit", word1)) {
		if (f != NO_CHAR) {
		    fprintf(stderr, "Error: decode_name, looking for a form, got implicit ? f %d, %d inword %s, word1 %s\n", f, c, inword, word1);
		}
		f = c = IMPLICIT_FORM;
	    } else if (!strcmp("conjunct", word1)) {
		if (f != NO_CHAR) {
		    fprintf(stderr, "Error: decode_name, looking for a form, got conjunct? f %d, %d (inword %s, word1 %s)\n", f, c, inword, word1);
		}
		f = c = CONJUNCT_FORM;
	    } else if (!strcmp("*", word1)) {
		c = ALL_CHARS;
	    }
	}

	/* do some preliminary parsing of what it is */
	if ((c >= FIRSTCONS && c <= LASTCONS) || (c==ALL_CHARS)) { /* is it a consonant? */
	    if (ii != tcm.count) {
	        fprintf(stderr, "Error: decode_name, looking for a consmany, got a vowel in the middle? ii %d, %d (inword %s, word1 %s)\n", ii, tcm.count, inword, word1);
	    }
	    tcm.cons[tcm.count] = c;
	    tcm.count++;
	}
        if (c > LASTCONS && c <= LASTCHAR) { /* is it a SPECIAL_TYPE?  */
	    if (f != NO_CHAR || t != 0) {
	        fprintf(stderr, "Error: decode_name, looking for a special, got another special? f %d, %d t=%d (inword %s, word1 %s)\n", f, c, t, inword, word1);
	    }
	    t = SPECIAL_TYPE;
	    l0 = c;
	}

	/* get next token */
	start = strtok(NULL, "-");
	if (start) {
	    strcpy(word1, start); /* first word is in here now */
	} else {
	    word1[0] = '\0';
	}

	ii++;
    }

    if (ii >= CONS_MAX && start) {
	fprintf(stderr, "Error: decode_name, got a word with more consonants that CONS_MAX %d? %s\n", CONS_MAX, inword);
    }

    if (!t && (tcm.count == 1 || tcm.count == 2)) { /* is a CONSONANT */
        l0 = tcm.cons[0];

	if (tcm.count == 2) l1 = tcm.cons[1];
	else l1 = NO_CHAR;

	if (tcm.count == 2) t = CONSONANT_DOUBLE_TYPE;
	else t = CONSONANT_SINGLE_TYPE;

    /* DEBUG
    fprintf(stderr, "decode_name: out of loop, tcm says is SINGLE/DOUBLE count %d (inword %s, word1 %s)\n", tcm.count, inword, word1);
    */
	*cm = tcm;
    }
    if (!t && (tcm.count >= 3)) { /* is a CONSONANT_MANY */
	*cm = tcm;
	/* note: for CONSONANT_MANY, this function uses S_current_fillup_font
	** variable to find the index of the consmany, if present.
	**/
	l0 = find_cons_many(S_current_fillup_font, tcm);

	/*
	fprintf(stderr, "decode_name: find_cons_many index %d word %s\n", l0, inword);
	*/
	if (l0 >= 0) { /* found it, so can return a valid type */
	    t = CONSONANT_MANY_TYPE;
	}
	
    /* DEBUG
    fprintf(stderr, "decode_name: out of loop, tcm says is MANY count %d, f= %d, l0 = %d  (inword %s, word1 %s)\n", tcm.count, f, l0, inword, word1);
    */
    }

#if DEBUG
    /* test/compare with old decode name - for debugging only */
    i = old_decode_name(inword, firstc, secondc, form);
    if (t != CONSONANT_MANY_TYPE && tcm.count < 3 && (i != t || *firstc != l0 || *secondc != l1 || *form != f)) {
	fprintf(stderr, "ERROR: %s diff NEW decode l0 %d l1 %d f %d t %d OLD decode: firstc %d s %d form %d, returning %d\n",
	inword, l0, l1, f, t,
	*firstc, *secondc, *form, i);
    }
#endif /* DEBUG */

    *firstc = l0;
    *secondc = l1;
    *form = f;

    return t;
}

/* ==================================================================== */
/* The encoding map: BE CAREFUL of changing the size of this
 * array - MUST always reflect the NUMCHARS defined value in imap.h
 * this array is GLOBAL, error handling in ichar.c (missing chars)
 * uses this data
 * NOTE:
 * All names must be distinct (see imap.h) and the order of
 * tokens here MUST be the same as in y_tab.h
 */
  ifm_enc_t G_ifm_map[NUMCHARS] = {
     {  /* 0 */	A_TOK,		A_CNAME  },
     {  /* 1 */	AA_TOK,		AA_CNAME  },
     {  /* 2 */	I_TOK,		I_CNAME  },
     {  /* 3 */	II_TOK,		II_CNAME  },
     {  /* 4 */	U_TOK,		U_CNAME  },
     {  /* 5 */	UU_TOK,		UU_CNAME  },
     {  /* 6 */	RI_TOK,		RI_CNAME  },
     {  /* 7 */	RII_TOK,	RII_CNAME  },
     {  /* 8 */	LI_TOK,		LI_CNAME  },
     {  /* 9 */	LII_TOK,	LII_CNAME  },
     {  /* 10 */	AY_TOK,		AY_CNAME  },
     {  /* 11 */	AAY_TOK,	AAY_CNAME  },
     {  /* 12 */	AI_TOK,		AI_CNAME  },
     {  /* 13 */	O_TOK,		O_CNAME  },
     {  /* 14 */	OO_TOK,		OO_CNAME  },
     {  /* 15 */	AU_TOK,		AU_CNAME  },
     {  /* 16 */	AM_TOK,		AM_CNAME  },
     {  /* 17 */	AHA_TOK,	AHA_CNAME  },
     {  /* 18 */	HALF_TOK,	HALF_CNAME  },
     {  /* 19 */	IMPLICIT_TOK,	IMPLICIT_CNAME  },
     {  /* 20 */	CONJUNCT_TOK,	CONJUNCT_CNAME  },
     {  /* 21 */	KA_TOK,		KA_CNAME  },
     {  /* 22 */	KHA_TOK,	KHA_CNAME  },
     {  /* 23 */	GA_TOK,		GA_CNAME  },
     {  /* 24 */	GHA_TOK,	GHA_CNAME  },
     {  /* 25 */	NGA_TOK,	NGA_CNAME  },
     {  /* 26 */	CHA_TOK,	CHA_CNAME  },
     {  /* 27 */	CHHA_TOK,	CHHA_CNAME  },
     {  /* 28 */	JA_TOK,		JA_CNAME  },
     {  /* 29 */	JHA_TOK,	JHA_CNAME  },
     {  /* 30 */	JNH_TOK,	JNH_CNAME  },
     {  /* 31 */	TTA_TOK,	TTA_CNAME  },
     {  /* 32 */	TTHA_TOK,	TTHA_CNAME  },
     {  /* 33 */	DDA_TOK,	DDA_CNAME  },
     {  /* 34 */	DDHA_TOK,	DDHA_CNAME  },
     {  /* 35 */	NNA_TOK,	NNA_CNAME  },
     {  /* 36 */	TA_TOK,		TA_CNAME  },
     {  /* 37 */	THA_TOK,	THA_CNAME  },
     {  /* 38 */	DA_TOK,		DA_CNAME  },
     {  /* 39 */	DHA_TOK,	DHA_CNAME  },
     {  /* 40 */	NA_TOK,		NA_CNAME  },
     {  /* 41 */	PA_TOK,		PA_CNAME  },
     {  /* 42 */	PHA_TOK,	PHA_CNAME  },
     {  /* 43 */	BA_TOK,		BA_CNAME  },
     {  /* 44 */	BHA_TOK,	BHA_CNAME  },
     {  /* 45 */	MA_TOK,		MA_CNAME  },
     {  /* 46 */	YA_TOK,		YA_CNAME  },
     {  /* 47 */	YYA_TOK,	YYA_CNAME  },
     {  /* 48 */	RA_TOK,		RA_CNAME  },
     {  /* 49 */	LA_TOK,		LA_CNAME  },
     {  /* 50 */	VA_TOK,		VA_CNAME  },
     {  /* 51 */	SHA_TOK,	SHA_CNAME  },
     {  /* 52 */	SHHA_TOK,	SHHA_CNAME  },
     {  /* 53 */	SA_TOK,		SA_CNAME  },
     {  /* 54 */	HA_TOK,		HA_CNAME  },
     {  /* 55 */	LDA_TOK,	LDA_CNAME  },
     {  /* 56 */	KSHA_TOK,	KSHA_CNAME  },
     {  /* 57 */	GYA_TOK,	GYA_CNAME  },
     {  /* 58 */	NNX_TOK,	NNX_CNAME  },
     {  /* 59 */	NYA_TOK,	NYA_CNAME  },
     {  /* 60 */	RRA_TOK,	RRA_CNAME  },
     {  /* 61 */	KADOT_TOK,	KADOT_CNAME  },
     {  /* 62 */	KHADOT_TOK,	KHADOT_CNAME  },
     {  /* 63 */	GADOT_TOK,	GADOT_CNAME  },
     {  /* 64 */	DDADOT_TOK,	DDADOT_CNAME  },
     {  /* 65 */	DDHADOT_TOK,	DDHADOT_CNAME  },
     {  /* 66 */	JADOT_TOK,	JADOT_CNAME  },
     {  /* 67 */	PHADOT_TOK,	PHADOT_CNAME  },
     {  /* 68 */	RA_HALF_TOK,	RA_HALF_CNAME  },
     {  /* 69 */	ANUSVARA_TOK,	ANUSVARA_CNAME  },
     {  /* 70 */	CHANDRA_TOK, 	CHANDRA_CNAME  },
     {  /* 71 */	CHANDRA_BN_TOK, CHANDRA_BN_CNAME  },
     {  /* 72 */	VIRAAM_TOK,	VIRAAM_CNAME  },
     {  /* 73 */	AVAGRAHA_TOK,	AVAGRAHA_CNAME  },
     {  /* 74 */	SRI_TOK, 	SRI_CNAME  },
     {  /* 75 */	AUM_TOK, 	AUM_CNAME  },
  };
  
  /* simlar map for the digits - used to fill up font_t.digits[] array */
  
  ifm_enc_t G_ifm_digit_map[10] = {
     {  /* 0 */	0,		ZERO_CNAME  },
     {  /* 1 */	1,		ONE_CNAME  },
     {  /* 2 */	2,		TWO_CNAME  },
     {  /* 3 */	3,		THREE_CNAME  },
     {  /* 4 */	4,		FOUR_CNAME  },
     {  /* 5 */	5,		FIVE_CNAME  },
     {  /* 6 */	6,		SIX_CNAME  },
     {  /* 7 */	7,		SEVEN_CNAME  },
     {  /* 8 */	8,		EIGHT_CNAME  },
     {  /* 9 */	9,		NINE_CNAME  },
  };
  
/* ==================================================================== */
/* find_cons_many will look for given consonant conjunct in the given
font's consmany list of consonant conjuncts (with more than 2
consonants).
This does a linear search thru the list, so if the list ever gets too
long, and the input text contains many 3+ consonant conjuncts, things
may get slow...
*/

int find_cons_many(font_t* f, dcharcm_t cm)
{
    int i, j, n;
    int index = -1; /* -1 == not found */
    dcharcm_t*	tcm;

    if (cm.count == 0) {
        fprintf(stderr, "program error: find_cons_many got count 0\n");
	return -1;
    }

    for (i = 0; i < f->numconsmany; i ++) {
	tcm = &f->consmany[i];
	n = tcm->count;
	if (n != cm.count) continue; /* are different, so look for next */

	for (j = 0; j < n; j ++) {
	    if (tcm->cons[j] != cm.cons[j]) break; /* are different */
	}

	if (j == n) {
	    index = i;
	    break; /* found cons many */
	}
    }

    return index;
}

/* =================================================================== */
/* print out font data (for debugging) */
void dump_font(font_t* font, FILE* outfp)
{
    int i;
    comp_unit_t* cus = NULL;

    fprintf(outfp, "font prop: %d\n", font->prop);
    fprintf(outfp, "font name: %s\n", font->name);

    for (i = 0; i < NUMKHADI; i ++) {
	if (font->khadi[i].cus) {
	    cus = font->khadi[i].cus[IMPLICIT_FORM];
	    if (!cus) cus = font->khadi[i].cus[A_FORM];
	}
	if (!cus && font->khadi[i].same_as && !font->khadi[i].same_as->cus[IMPLICIT_FORM])
	    continue;

	fprintf(outfp, "Form a/implicit: Character %d, ", i);
	if (i < NUMCHARS) {
	    fprintf(outfp, "name: %s\n", font->enc[i].codename);
	} else {
	    fprintf(outfp, "name: [CCADD temp name]\n");
	}

	while (cus) { /* NULL test */
	    fprintf(outfp, " cus: pschar %d, delta %d %d; ",
			(cus)->u_pschar, (cus)->deltax, (cus)->deltay);
	    cus = cus->next;
	}
	cus = NULL; 
	if (font->khadi[i].same_as) cus = font->khadi[i].same_as->cus[IMPLICIT_FORM];
	while (cus) { /* NULL test */
	    fprintf(outfp, " cus: pschar %d, delta %d %d; ",
			(cus)->u_pschar, (cus)->deltax, (cus)->deltay);
	    cus = cus->next;
	}
	fprintf(outfp, "\n");
    }

    for (i = 0; i < NUMPSCHARS; i ++) {
	if (font->psfm[i].w > 0) {
	fprintf(outfp, "char %d:: width %d ", i, font->psfm[i].w);
	fprintf(outfp, "llx %d ", font->psfm[i].llx);
	fprintf(outfp, "lly %d ", font->psfm[i].lly);
	fprintf(outfp, "urx %d ", font->psfm[i].urx);
	fprintf(outfp, "ury %d\n", font->psfm[i].ury);
	}
    }
} /* print_font() */

/* ==================================================================== */

#if 0 /* ---------------------------------------------- */
/* old routine decode_name here, just for debugging - renamed old_decode_name */
static int old_decode_name(char inword[], int* firstc, int* secondc, int* form)
{
    char word[256], word1[256], word2[256], word3[256];
    char *start;
    int i, l0, l1, f, t;

    word1[0] = '\0'; word2[0] = '\0'; word3[0] = '\0';
    l0 = l1 = f = NO_CHAR;
    t = 0;
    *firstc = *secondc = *form = NO_CHAR;

    strcpy(word, inword);
    start = strtok(word, "-");
    if (!start) return 0; /* ERROR */
    strcpy(word1, start); /* first word is in here now */

    if ((start = strtok(NULL, "-")) != NULL) { /* look for second word */
        strcpy(word2, start); /* second word is in here now */
        if ((start = strtok(NULL, "-")) != NULL) { /* look for third word */
            strcpy(word3, start); /* third word is in here now */
        }
    }

#ifdef DEBUG
fprintf(stderr, "decoding: word 1 :%s:, 2 :%s:, 3 :%s:\n", word1, word2, word3);
#endif /* DEBUG */

    /* look for l0, could be vowel, cons, or user-defined cons (extra_map) */
    for (i = 0; i < NUMCHARS; i ++) {
	if (!strcmp(G_ifm_map[i].codename, word1)) {
	    l0 = i; break;
	}
    }
    if (l0 < 0) {
        for (i = 0; i < NUMEXTRA; i ++) {
	    if (!strcmp(S_ifm_extra_map[i].codename, word1)) {
	        l0 = S_ifm_extra_map[i].token; break;
	    }
        }
	if (l0 < 0) { /* check for specials: digits, punctuation, etc */
            for (i = 0; i < 10; i ++) {
	        if (!strcmp(G_ifm_digit_map[i].codename, word1)) {
	            l0 = G_ifm_digit_map[i].token;
		    t = DIGIT_TYPE;
		    break;
	        }
	    }
        }
    }
    if (l0 < 0) {
	if (!strcmp("*", word1)) {
	    l0 = ALL_CHARS;
	}
    }

    /* look for l1, must be vowel or cons or allchars */
    if (word2[0]) {
        if (!strcmp("half", word2)) {
	    l1 = HALF_FORM;
        } else if (!strcmp("implicit", word2)) {
	    l1 = IMPLICIT_FORM;
        } else if (!strcmp("conjunct", word2)) {
	    l1 = CONJUNCT_FORM;
        } else {
            for (i = 0; i < NUMCHARS; i ++) {
	        if (!strcmp(G_ifm_map[i].codename, word2)) {
	            l1 = i; break;
	        }
            }
	}
    }
    if (l1 < 0) {
	if (!strcmp("*", word2)) {
	    l1 = ALL_CHARS;
	}
    }


    /* look for form, must be vowel */
    if (word3[0]) {
        if (!strcmp("half", word3)) {
	    f = HALF_FORM;
        } else if (!strcmp("implicit", word3)) {
	    f = IMPLICIT_FORM;
        } else if (!strcmp("conjunct", word3)) {
	    f = CONJUNCT_FORM;
        } else {
            for (i = 0; i <= AHA_FORM; i ++) {
	        if (!strcmp(G_ifm_map[i].codename, word3)) {
	            f = i; break;
	        }
            }
	}
    }

    /* DECODE IT, if t not already known */
    if (!t && (l0 >= ALL_CHARS && (l1 >= FIRSTCONS || l1 == ALL_CHARS) &&
	    strcmp(word2, "half") && strcmp(word2, "implicit") &&
            strcmp(word2, "conjunct"))) {
	/* is a double consonant with possible form (may be absent) */
	/* only if the second word was not half or implicit or conjunct */
	    t = CONSONANT_DOUBLE_TYPE;
#ifdef DEBUG
   fprintf(stderr, "defined as DOUBLE: t is %d\n", t);
#endif /* DEBUG */
	    if (f < A_FORM || f > CONJUNCT_FORM) {
#ifdef DEBUG
		fprintf(stderr, "Warning: Double Consonants (ligatures) must have a valid form (Unless this is a CCS statement!) - assuming IMPLICIT here (%s)\n", inword);
		f = IMPLICIT_FORM;
#endif /* DEBUG */
	    }
    } else if (!t && (l0 >= FIRSTVOW && l0 <= LASTVOW)) {
        /* check if l0 is a vowel */
	f = IMPLICIT_FORM; /* vowels are always one word only */
	l1 = NO_CHAR;
	t = VOWEL_TYPE;
    } else if (!t && (l0 >= 0)) {
        /* must be single cons type, code is two words only */
        f = l1;
	l1 = NO_CHAR;
        if (l0 > LASTCONS && l0 <= LASTCHAR) {
	    t = SPECIAL_TYPE;
	    if (f != IMPLICIT_FORM) fprintf(stderr, "Warning: All Specials must have IMPLICIT_FORM only (%s)\n", inword);
	    f = IMPLICIT_FORM;
        } else {
	    t = CONSONANT_SINGLE_TYPE;
        }
    } else if (!t && (l0 == ALL_CHARS)) {
	f = NO_CHAR;
	l1 = NO_CHAR;
	t = CONSONANT_SINGLE_TYPE;
#ifdef DEBUG
   fprintf(stderr, "l0 is ALL_CHARS, defined as SINGLE: t is %d\n", t);
#endif /* DEBUG */
    } /* else is a new userdefined char */

    /* error checks */
    if (word1[0] && (l0 == NO_CHAR)) {
    	/* not an error - since CCADD statements will use new names
    	 * If this is an error, then fillup_font will flag it as such
    	 */
    }
    if (word2[0] && (l1 == NO_CHAR) && (f == NO_CHAR)) {
        fprintf(stderr, "old_decode_name: parsing IFM file - got (possibly) illegal word2: %s\n",
        		word2);
        t = 0; /* error type */
    }
    if (word3[0] && (f == NO_CHAR)) {
        fprintf(stderr, "old_decode_name: parsing IFM file - got (possibly) illegal word3: %s\n",
        		word3);
        t = 0; /* error type */
    }
    if (word3[0] && (t != CONSONANT_DOUBLE_TYPE)) {
        fprintf(stderr, "old_decode_name: parsing IFM file - got (possibly) illegal codename: %s\n",
        		inword);
        t = 0; /* error type */
    }
    *firstc = l0;
    *secondc = l1;
    *form = f;

#ifdef DEBUG
    if (t == DIGIT_TYPE) fprintf(stderr,"decode name--returning digit type\n");
    fprintf(stderr, "decode l0 %d l1 %d f %d returning %d, f %d s %d form %d\n",
l0, l1, f, t, *firstc, *secondc, *form);
    if (*form < 0) {
	fprintf(stderr, "WARNING: form is NEGATIVE: %d (%s)\n", *form, inword);
    }
#endif /* DEBUG */
    return t;
}
#endif /*0 old_decode_name function ----------------------------------- */

/* ===========================^ font.c ^ ============================== */
