/*
 *========================================================================== 
 * Copyright 1991-1994 Avinash Chopde, All Rights Reserved.
 */

#include "itrans.h"

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/ichar.c,v 1.4 1998/06/13 19:13:35 avinash Exp $";

/* =================================================================== */
/* Handle a letter (possibly complex)
 * depending on the language, different methods of letter
 * constructions are followed.
 * For example, Tamil is the simplest -- has no ligatures.
 *	Telugu uses the last consonant of multi-consonant aksharaas
 *		as the "root" consonant
 *	while Sanskrit uses the first consonant of multi-consonant aksharaas
 *		as the "root" consonant
 * etc
 * The files *_mkl.c contain the code for actual aksharaa construction
 */

/******
		(font_t* fptr, the font data structure to use
		letter_t dlet, letter to convert 
		comp_unit_t pcus[], the comp units that make this
				    letter - all u_pschar are valid
				    PostScript codes, or NO_PSCHAR
				    
		int	size_pcus) array size of ecus- number of elements 
********/
int make_letter(font_t* fptr,
		letter_t dlet,
		comp_unit_t pcus[],
		int	size_pcus)
{
    int num_pcus = 0;
    int tok = fptr->lang_tok;

    if (fptr->prop & UNICODE_FONT) {
       tok = SANSKRIT_TOK; /* use default devanagari-like processing for UTF8 */
    }

    switch( tok ) {
    
    case TAMIL_TOK:
	num_pcus = 
	    tamil_make_letter(fptr, dlet, pcus, size_pcus);
        break;
	
    case KANNADA_TOK: /* kannada and telugu use similar composition rules */
    case TELUGU_TOK:
	num_pcus = 
	    telugu_make_letter(fptr, dlet, pcus, size_pcus);
        break;
    
    case BENGALI_TOK:
    default:
	num_pcus = 
	    dflt_make_letter(fptr, dlet, pcus, size_pcus);
	
        break;
    }
    
    return num_pcus;

} /* make_letter() */
/* =================================================================== */
int add_cus(pschar_t psfm[], comp_unit_t* pcu, comp_unit_t cu)
{
    int n, w;

    *pcu = cu;
    n = 1;

    /* if this is a zero with char, add one more comp unit */
    if (cu.u_pschar >= 0 && cu.u_pschar < NUMPSCHARS) {
	w = psfm[cu.u_pschar].w;
	if (w == 0) { /* reapply delta to get back at current pos*/
	    pcu++;
	    pcu->deltax = -cu.deltax;
	    /* pcu->deltay = -cu.deltay; */
	    pcu->deltay = 0; /* no Y delta, since Y movements are always
			      * restored back to correct Y value
			      * by the cus_to_tex() and cus_to_ps()
			      * functions...
			      */
	    pcu->u_pschar = NO_PSCHAR; /* just delta */
	    n++;
	}
    }

    return n;
}
/* =================================================================== */
int add_cus_list(pschar_t psfm[], comp_unit_t* pcus,
		  comp_unit_t* cus, comp_unit_t* impcus)
{
    int num_pcus = 0;
    comp_unit_t* lcus = cus;
    comp_unit_t* startcus = NULL;
    int pcode;

    /* collect all the PS chars in lcus into the output array */
    while (lcus) {

		/* get the PS code of this unit */
	pcode = lcus->u_pschar;

#ifdef DEBUG
fprintf(stderr, "while lcus in add_cus_list: pcode is %d\n", pcode);
#endif /*DEBUG*/
	if (pcode == NO_PSCHAR || pcode >= 0) {
	    /* valid PostScript code */

	    num_pcus += add_cus(psfm, &pcus[num_pcus], *lcus);

	} else if (pcode == IMPLICIT_PSCHAR && impcus) {
	    /* Need to use the implicit char description, and have a
	     * valid pointer to comp_units that compose the implicit
	     * char.
	     */
	    startcus = impcus;
	    while (startcus) {

#ifdef DEBUG
fprintf(stderr, "while startcus in add_cus_list impcus: pcode is %d, num_pcus %d\n", startcus->u_pschar, num_pcus);
#endif /*DEBUG*/

		    /* error check */
	        if ( startcus->u_pschar != NO_PSCHAR &&
		     startcus->u_pschar < 0 ) {

		    fprintf(stderr, "ERROR: %d illegal pschar in implicit comp units\n", startcus->u_pschar);
	        } else {
		    num_pcus += add_cus(psfm,&pcus[num_pcus],*startcus);
		}

		startcus = startcus->next;
	    } /* while startcus */
	} else { /* if pcode == IMPLICIT */
	    fprintf(stderr, "Error (line %d): in add_cus_list() - pcode is %d, and no imp list\n", G_lineno, pcode);
	    fprintf(stderr, "      This error is possibly due to missing CC lines in the IFM file\n");
	}

 	lcus = lcus->next;

    } /* while lcus */

    return num_pcus;
} /* add_cus_list() */

/* =================================================================== */
/* get the CUs for a CONSONANT_SINGLE or a VOWEL or a SPECIAL type or 
 * CONSONANT_DOUBLE or MANY - as long as the exact definition of this
 * letter is present in the IFM file.
 * (see get_cus_implicit  in dflt_mkl.c for cases where the program
 * has to make an implicit grouping (((C1+C2)+C3)+C4) etc...)
 */

int get_cus_explicit(font_t* fptr, /* the font data structure to use */
		letter_t dlet, /* letter searched for */
		comp_unit_t** cus, /* will return a pointer to the cus here */
		comp_unit_t** imp) /* will return implicit char creation ptr */
{
    int i, form, c1, c2, nolig, icm;
    comp_unit_t *clist;
    comp_unit_t *cimp;
    dchar_t  dc, *next;
    dcharcm_t   dcm, ddcm;

    *cus = NULL;
    *imp = NULL;;

    form = dlet.v;
    if (form != HALF_FORM && form != IMPLICIT_FORM && form != CONJUNCT_FORM)
	form = _I_(form);
    c1 = _I_(dlet.cons[0]);
    c2 = _I_(dlet.cons[1]);
    nolig = dlet.nolig[0]; /* if TRUE, then do not use ligature */

    dc.cus = NULL;
    dc.same_as = NULL;
    clist = NULL;
    cimp = NULL;

#ifdef DEBUG
fprintf(stderr, "get ccus explicit for: c1 %d, form %d type %d\n", c1, form, dlet.type);
#endif /*DEBUG*/

    switch (dlet.type) {
    case CONSONANT_SINGLE_TYPE:
	dc = fptr->khadi[c1];
	if (dc.cus) {
	    clist = dc.cus[form];
	    cimp = dc.cus[IMPLICIT_FORM];
#ifdef DEBUG
fprintf(stderr, "get cus explicit: clist is %d cimp %d\n", (int)clist, (int)cimp);
#endif /*DEBUG*/
	}
	next = dc.same_as;
	break;
    case CONSONANT_DOUBLE_TYPE:
	if (nolig) {
	    return FALSE; /* cannot use explicit defn even if present in IFMfile */
	} else {
	    dc = fptr->ligatures[c1][c2];
	    if (dc.cus) {
	        clist = dc.cus[form];
	        cimp = dc.cus[IMPLICIT_FORM];
	    }
	}
	next = dc.same_as;
	break;
    case CONSONANT_MANY_TYPE:
	for (i = 0; i < dlet.n; i ++) {
	    dcm.cons[i] = _I_(dlet.cons[i]);
	}
	dcm.count = dlet.n;
	/* null all other unused fields */
	dcm.same_as = NULL; dcm.word[0] = '\0';
	for (i = 0; i < NUMFORMS; i ++) {
	    dcm.cus[i] = NULL;
	}

	icm = find_cons_many(fptr, dcm);
	if (icm < 0) {

#ifdef DEBUG
fprintf(stderr, "NOT PRESENT (is ok): get_cus_explicit for: c1 %d, c2 %d... (count %d) form %d\n", dcm.cons[0], dcm.cons[1], dcm.count, form);
#endif /*DEBUG*/
	    return FALSE; /* no explicit defn present in IFMfile */
	}

	ddcm = fptr->consmany[icm];
	if (ddcm.cus) {
	    clist = ddcm.cus[form];
	    cimp = ddcm.cus[IMPLICIT_FORM];
	}
	next = ddcm.same_as;
	break;
    case VOWEL_TYPE:
	dc = fptr->khadi[form];
	form = IMPLICIT_FORM;
	if (dc.cus) {
	    clist = dc.cus[form];
	    cimp = dc.cus[IMPLICIT_FORM];
	}
	next = dc.same_as;
	break;
    case SPECIAL_TYPE:
	dc = fptr->khadi[_I_(dlet.s)];
	form = IMPLICIT_FORM;
	if (dc.cus) {
	    clist = dc.cus[form];
	    cimp = dc.cus[IMPLICIT_FORM];
	}
	next = dc.same_as;
	break;
    default:
	fprintf(stderr, "Program ERROR: get_explicit called for a invalid letter\n");
	return FALSE;
    } /* switch */

    /* follow the same_as pointers until get required clist */
    while (!clist && next) {
	if (next->cus)  clist = next->cus[form];
#ifdef DEBUG
fprintf(stderr, "get cus explicit: following sam_as pointer clist is %d\n", (int)clist);
#endif /*DEBUG*/
	next = next->same_as;
    }

    if (!clist) {
	if (dlet.type != CONSONANT_MANY_TYPE
	    && dlet.type != CONSONANT_DOUBLE_TYPE) {

	    /* this is an error only if dlet is not a consonant conjunct */
	    fprintf(stderr, "*** Warning (line %d): Font (in %s) is missing this letter:\n",
			    G_lineno, fptr->fname);
	    if (dlet.type == SPECIAL_TYPE && dlet.s > 0) {
		fprintf(stderr, "*** Accent/Special: token (%d), IFM name (%s)\n", dlet.s,
					    G_ifm_map[dlet.s-A_TOK].codename);
	    } else { /* not a special, so must be a single, vowel, or digit */
		for (i = 0;  i < dlet.n; i ++) {
		    form = dlet.cons[i];
		    fprintf(stderr, "*** Consonant %d: token (%d), IFM name (%s)\n",
				i,form,G_ifm_map[form-A_TOK].codename);
		}
		if (dlet.v < A_TOK)
		    fprintf(stderr, "*** Vowel: token (%d), IFM name (%s)\n",
					dlet.v+OFFSET_TOK, G_ifm_map[dlet.v].codename);
		else
		    fprintf(stderr, "*** Vowel: token (%d), IFM name (%s)\n", dlet.v,
						G_ifm_map[dlet.v-A_TOK].codename);
	    }
	    fprintf(stderr, "***\n");
	}
        return FALSE;
    }

    *cus = clist;
    *imp = cimp;
    return TRUE;

} /* get_cus_explicit() */
/* =================================================================== */
int add_special(font_t* fptr,
		int spec,
		comp_unit_t pcus[],
		int	size_pcus)
{
    int num_pcus = 0;
    comp_unit_t *cus, *imp;
    letter_t lt;

    lt.n = 0;
    lt.s = spec;
    lt.type = SPECIAL_TYPE;
    lt.v = IMPLICIT_FORM;
    
#ifdef DEBUG
fprintf(stderr, "add_special: special %d\n", lt.s);
#endif
    
    if (!get_cus_explicit(fptr, lt, &cus, &imp)) {
	/* error - message is printed by get_cus_cs.. */
	return 0; /* 0 elements added */
    }

    /* add the list of comp units to the output array,
     * taking care of any implicit pointers 
     */
    num_pcus = add_cus_list(fptr->psfm, &pcus[0], cus, imp);
#ifdef DEBUG
fprintf(stderr, "add_special: after add_cus_list: num_pcus %d cus\n", num_pcus);
#endif

    return num_pcus;
}
/* ============================^ ichar.c ^ =========================== */
