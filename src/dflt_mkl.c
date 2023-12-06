/*
 *========================================================================== 
 * Copyright 1991-1994 Avinash Chopde, All Rights Reserved.
 */

#include "itrans.h"

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/dflt_mkl.c,v 1.3 1997/05/17 19:10:51 avinash Exp $";

/* =================================================================== */
static int S_get_cus_implicit(font_t* fptr, /* the font data structure to use */
		letter_t dlet, /* letter searched for */
		comp_unit_t** cus, /* will return a pointer to the cus here */
		comp_unit_t** imp, /* will return implicit char creation ptr */
		int* lastform,
		int* lastlig); /* whether last two cons have a ligature */
static int S_add_cus_cm(font_t* fptr, /* the font data structure to use */
		    letter_t dlet, /* letter searched for */
		    comp_unit_t* pcus, /* the output cus */
		    int size_pcus,
		    comp_unit_t* cus, /* the cus to add */
		    int	lastform, /* last form to use */
		    int	lastlig); /* whether a ligature exists for the last 2 */
/* =================================================================== */
/* Handle a letter (possibly complex) */

/******
		(font_t* fptr, the font data structure to use
		letter_t dlet, letter to convert 
		comp_unit_t pcus[], the comp units that make this
				    letter - all u_pschar are valid
				    PostScript codes, or NO_PSCHAR
				    
		int	size_pcus) array size of ecus- number of elements 
********/
int dflt_make_letter(font_t* fptr,
		letter_t dlet,
		comp_unit_t pcus[],
		int	size_pcus)
{
    int num_pcus; /* number of postscript chars that were added to pcus */
    int i, found;
    comp_unit_t*	cus;  /* comp units that make the form of
				* the given letter dlet
				*/
    comp_unit_t*	imp;  /* comp units that make the implicit form
				* of the given letter dlet
				*/
    comp_unit_t*	lcus;
    int lastform = 999, lastlig = 99;
    int called_implicit = FALSE;

    num_pcus = 0;
    found = FALSE;

    if (dlet.n > CONS_MAX) {
	fprintf(stderr, "dflt_make_letter: too many consonants in letter %d\n", dlet.n);
        return 0;
    }

#ifdef DEBUG
fprintf(stderr, "in dflt_make_letter, type %d, n %d cons %d %d, vow %d\n",
	dlet.type, dlet.n, _I_(dlet.cons[0]), _I_(dlet.cons[1]), dlet.v);
#endif

    switch( dlet.type ) {
    case VOWEL_TYPE:
    case CONSONANT_SINGLE_TYPE:
	if (!get_cus_explicit(fptr, dlet, &cus, &imp)) {
	    /* error - message is printed by get_cus_explicit.. */
	    return 0; /* 0 elements added */
	}

	/* add the list of comp units to the output array,
	 * taking care of any implicit pointers 
	 */
	num_pcus += add_cus_list(fptr->psfm, &pcus[num_pcus], cus, imp);

        break;
    case CONSONANT_DOUBLE_TYPE:
    case CONSONANT_MANY_TYPE:
	/* default action for handling multiple consonant
	 * aksharas.
	 * Devanagari script rules are followed.
	 * Search for any ligatures, and use them.
	 * (unless the user has restricted ligatures by making use
	 * of the NOLIG_TOKEN -- the {} chars in the input stream
	 * in between two consonants disable ligatures)
	 */
	/* first see if a direct definition exists for this */
	if (!get_cus_explicit(fptr, dlet, &cus, &imp)) {

	    /* no explicit definition, so use heuristics to get the clist */
	    called_implicit = TRUE;
	    if (!S_get_cus_implicit(fptr, dlet, &cus, &imp, &lastform, &lastlig)) {
		/* error - message is printed by get_cus_implicit.. */
		return 0; /* 0 elements added */
	    }
	}
	
	/* add the list of comp units to the output array,
	 * taking care of any implicit pointers 
	 */
	if (!imp) {
	    /* implicit cu pointer is empty - check if it is needed */
	    found = FALSE;
	    lcus = cus;
	    while (!found && lcus) {
		if (lcus->u_pschar == IMPLICIT_PSCHAR) found = TRUE;
		lcus = lcus->next;
	    }
	}
	
	if (!found || imp) {
	    /* no implicit char in the cus list, or if implicit char
	     * is present, the imp list is non-empty.
	     * So, the normal function add_cus_list() can handle this
	     */
#ifdef DEBUG
fprintf(stderr, "calling add_cus_list: !found || imp\n");
#endif
	    num_pcus += add_cus_list(fptr->psfm, &pcus[num_pcus], cus, imp);
	} else {
	    /* call a special function which knows how to construct the
	     * implicit cu list for a multiple consonant akshara
	     */
#ifdef DEBUG
fprintf(stderr, "calling S_add_cus_cm\n");
#endif
	    if (!called_implicit) {
		/* call it to get the right cus, and to get lastform & lastlig*/
		called_implicit = TRUE;
		if (!S_get_cus_implicit(fptr, dlet, &cus, &imp, &lastform, &lastlig)) {
		    /* error - message is printed by get_cus_implicit.. */
		    return 0; /* 0 elements added */
		}
	    }
	    num_pcus += S_add_cus_cm(fptr, dlet, &pcus[num_pcus],
				     size_pcus - num_pcus, cus,
	    			     lastform, lastlig);
	}
        break;

    case SPECIAL_TYPE:
	/* handled after this switch */
        break;
        
    default:
	fprintf(stderr, "dflt_make_letter: error illegal letter type %d\n", dlet.type);
        break;
    } /* switch */
    
     /* handle special character */
    if (dlet.s > 0) {
	num_pcus += add_special(fptr, dlet.s, &pcus[num_pcus],
				size_pcus - num_pcus);
    }
	
#ifdef DEBUG
fprintf(stderr, "dflt_make_letter::created CUS containging %d cus\n", num_pcus);
#endif /*DEBUG*/

	/* keep the next pointers in the pcus correct */
    for (i = 0; i < num_pcus; i ++) {
	pcus[i].next = &pcus[i+1];
    }
    if (num_pcus > 0) pcus[num_pcus-1].next = NULL;

    return num_pcus;

} /* dflt_make_letter() */
/* =================================================================== */
/* S_add_cus_cm will create a CUS list for given consonant many.
 * It will use consonant half-forms, or consonant-consonant-half-forms
 * to create a complete list for the given list of consonants.
 * Note that it will check for existence of only 2-consonant conjuncts,
 * even if a 3-consonant conjunct exists, it will be not be used to
 * construct the CUS list.
 * So, given C1+C2+C3+C4+C5... etc, it will only try two at a time, so
 * even if C1+C2+C3 is defined in the IFM file, it will not be used,
 * instead C1+C2, then C3+C4, etc will be used.
 */
static int S_add_cus_cm(font_t* fptr, /* the font data structure to use */
		    letter_t dlet, /* letter searched for */
		    comp_unit_t* pcus, /* the output cus */
		    int size_pcus,
		    comp_unit_t* cus, /* the cus to add */
		    int	lastform, /* last form to use */
		    int	lastlig) /* whether a ligature exists for the last 2 */
{
    comp_unit_t* lcus = cus;
    int i, pcode, j, c1, c2, nolig;
    int num_pcus = 0;
    letter_t	rcons; /* for recursive calls */
    
    /* collect all the PS chars in lcus into the output array */
    while (lcus) {

		/* get the PS code of this unit */
	pcode = lcus->u_pschar;

#ifdef DEBUG
fprintf(stderr, "while lcus in S_add_cus_cm: pcode is %d\n", pcode);
#endif /*DEBUG*/
	if (pcode == NO_PSCHAR || pcode >= 0) {
	    /* valid PostScript code */

	    num_pcus += add_cus(fptr->psfm, &pcus[num_pcus], *lcus);

	} else if (pcode == IMPLICIT_PSCHAR) {
	    /* implies that no implicit char defined for this letter, and
	     * the letter is complex, and has to be split into
	     * simpler parts
	     * The way it is done is that every consonant is
	     * printed out in CONJUNCT_FORM, except the last
	     * consonant, which is printed out in its IMPLICIT_FORM.
	     ************
	     * What to do here is driven by the CONSONANTS_MANY case
	     * in S_get_cus_implicit()
	     * Of course, be careful, this could also be CONS..DOUBLE...
	     * in case the IFM file contains SAME_AS CCS statements
	     * but no implicit char defns...
	     */
#ifdef DEBUG
fprintf(stderr, "S-add_cus_cm: pcode IMPLICIT, no icus, looping for conjunct forms\n");
#endif /*DEBUG*/

	    for (i = 0; i < dlet.n; i ++) {

#ifdef DEBUG
fprintf(stderr, "S-add_cus_cm-letter i %d dlet.n %d lastlig %d\n", i, dlet.n, lastlig);
#endif
		/* check if a ligature has been defined for
		 * (current char, next char) pair, use it if available
		 */
		if (i < (dlet.n-1)) { /* i.e, two or more consonants left..*/

		    c1 = _I_(dlet.cons[i]); c2 = _I_(dlet.cons[i+1]);
		    if (i == (dlet.n-1)) j = lastform; /* last chars */
		    else j = CONJUNCT_FORM; /* still more to come,so use conjunct form*/
		    nolig = ((dlet.nolig[i]) || (lastlig && i == (dlet.n-3)));
#ifdef DEBUG
fprintf(stderr, "S-add_cus_cm nolig %d\n", nolig);
#endif
			/* if last lig exists, do not use the 
			 * secondlast consonant with the thirdlast consonant.
			 * -- just display the thirdlast consonant in
			 * conjunct  form.
			 */
		    if (!nolig && fptr->ligatures[c1][c2].cus &&
		            fptr->ligatures[c1][c2].cus[IMPLICIT_FORM]) {
			    /* just check for existence of the IMPLICIT_FORM-
			     * if it exists, the conjunct -form will also
			     * exist---other way around is not true!.
			     */
#ifdef DEBUG
		    fprintf(stderr, "C1: %d, C2 %d, cus(%d)\n", c1, c2,
		            fptr->ligatures[c1][c2].cus[j]);
#endif /*DEBUG*/
			/* found ligature! */
		        if (i == (dlet.n-2)) j = lastform; /* last chars */
		        else j = CONJUNCT_FORM; /* more to come,so use conjunct form*/
			rcons.n = 2; rcons.v = j;
			rcons.type = CONSONANT_DOUBLE_TYPE;
			rcons.cons[0] = dlet.cons[i];
			rcons.cons[1] = dlet.cons[i+1];
			rcons.nolig[0] = FALSE;
			i++; /* have used up next char too */
		    } else {
			/* no ligature defined, draw a conjunct  form */
			rcons.n = 1; rcons.v = CONJUNCT_FORM;
			rcons.type = CONSONANT_SINGLE_TYPE;
			rcons.cons[0] = dlet.cons[i];
		    }
		} else { /* this is the last char */
		    rcons.n = 1; rcons.v = lastform;
		    rcons.type = CONSONANT_SINGLE_TYPE;
		    rcons.cons[0] = dlet.cons[i];
		}

		/* recursive call */
		rcons.s = -1; /* no special or accent */
		j = dflt_make_letter(fptr, rcons, &pcus[num_pcus],
				     size_pcus - num_pcus);
		num_pcus += j;
	    }
	} else { /* ERROR */
	    fprintf(stderr, "ERROR ERROR: %d illegal pschar in Form comp units\n", pcode);
	}


	lcus = lcus->next;
    } /* while (lcus) */

    return num_pcus;

} /* S_add_cus_cm() */
/* =================================================================== */
static comp_unit_t S_implicit_cu = {IMPLICIT_PSCHAR, 0, 0, 0, 0, NULL};
/* get the list of CUs for a multiple consonant akshara
 * NOTE: call this for CONSONANT_DOUBLE or CONSONANT_MANY only.
 * Call this only if get_cus_explicit fails to find a definition for the
 * given consonant conjunct.
 */

static int S_get_cus_implicit(font_t* fptr, /* the font data structure to use */
		letter_t dlet, /* letter searched for */
		comp_unit_t** cus, /* will return a pointer to the cus here */
		comp_unit_t** imp, /* will return implicit char creation ptr */
		int*	lastform,
		int*	lastlig)
		       /* imp may be NULL on return, which implies that
			* the calling function has to create
			* the implicit definition of the multiple
			* consonant akshara itself.
			*/
			/* lastform: CONSDOUBLE/MANY return form to use
			 * of the last consonant (usually
			 * IMPLICIT_FORM)
			 */
			/* lastlig: will return TRUE if the last two
			 * two consonants have a ligature
			 */
{
    int i, form, c1, c2, c2only, found, nolig;
    comp_unit_t *clist, *lcus;
    comp_unit_t *cimp;
    dchar_t  dc, *next;
    int	ra_hack = FALSE;

    *cus = NULL;
    *imp = NULL;;
    *lastlig = FALSE;
    
    if (dlet.type != CONSONANT_DOUBLE_TYPE &&
        dlet.type != CONSONANT_MANY_TYPE) {

	fprintf(stderr, "Program ERROR: get_cus_implicit can only be called for a multiple consonant letter\n");
	return FALSE;
    }
    
    c2only = FALSE; /* flag is used for CONSDOUBLE/CONSMANY forms.
		     * If the ligature is not found, then the whole
		     * character is assumed to follow the form
		     * for the c2 consonant.
		     * BUT, that implies that the clist for c2
		     * contains a reference to the "implicit" char,
		     * so that c1-conjunct -form also gets printed
		     * (by a recursive call).
		     * If c2 does not contain a reference to an
		     * implicit char, then one is stuffed in.
		     */

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
fprintf(stderr, "get_cm ccus form for: c1 %d, c2 %d, form %d type %d\n", c1, c2, form, dlet.type);
#endif /*DEBUG*/

    switch (dlet.type) {
    case CONSONANT_DOUBLE_TYPE:
	*lastform = IMPLICIT_FORM;
	if (!nolig) {
	    dc = fptr->ligatures[c1][c2];
	    if (dc.cus) {
		*lastlig = TRUE;
	        clist = dc.cus[form];
	        cimp = dc.cus[IMPLICIT_FORM];
	    }
	}
	/* If clist is NULL, and same_as is NULL,
	 * consider the last consonant, and follow the form for
	 * its CONS..SINGLE form
	 * NOTE: cimp cannot be looked for!
	 */
#ifdef DEBUG
fprintf(stderr, "consdouble: before same_as clist is %d cimp %d (sameas %d)\n", (int)clist, (int)cimp, dc.same_as);
#endif /*DEBUG*/
	if (!clist && !dc.same_as) {
	    dc = fptr->khadi[c2]; /* to follow same_as ptr */
	    if (dc.cus) {
		clist = dc.cus[form];
		c2only = TRUE;
	    }
	}
#ifdef DEBUG
fprintf(stderr, "consdouble: after same_as clist is %d cimp %d (new sameas %d)\n", (int)clist, (int)cimp, dc.same_as);
#endif /*DEBUG*/
	break;
    case CONSONANT_MANY_TYPE:
	*lastform = IMPLICIT_FORM;

	/* hack to handle ra-cons-cons-cons etc
	 * check if ra-conjunct has just 1 CU with NO_PSCHAR in it.
	 * If yes, use consonants 0, 1 to get CUlist.
	 * NOTE: for this to work (to get the hindi half-ra), the
	 * CC line for ra-conjunct should be:
itrans: CC ra-conjunct 1 ; PCC none 0 0 ;
	 * and there must be a CCS ra-<cons> line, but no
	 * CC ra-<cons>-implicit line.
	 * (see dvng.ifm for examples).
	 * The reason there must be no CC ra-<cons>-implicit line is to
	 * prevent S_add_cus_cm() from calling dflt_make_letter()
	 * recursively for ra-<cons>, we need it to call
	 * dflt_make_letter() for just ra-<conjunct>...
	 */
	c1 = _I_(dlet.cons[0]);
	dc = fptr->khadi[c1];
	lcus = NULL;
	if (dc.cus) lcus = dc.cus[CONJUNCT_FORM];
	if (lcus && lcus->next == NULL) {
	    /* only one char code in this list */
	    if (lcus->u_pschar == NO_PSCHAR) {
		ra_hack = TRUE;
#ifdef DEBUG
		fprintf(stderr, "RA-HACK IS TRUE!\n");
#endif
	    }
	}
	/* ok, now, can either choose cons[0], cons[1] or 
	 * cons[n-2],cons[n-1] to get the implicit chain.
	 * But if cons[n-2] is RA, then don't choose n-2,n-1
	 * This can be detected if the lcus for cons[n-2] has a NO_PSCHAR
	 * in it.
	 * All this crap to handle the hindi ra-half!
	 */
	c1 = _I_(dlet.cons[dlet.n - 2]);
	dc = fptr->khadi[c1];
	lcus = NULL;
	if (dc.cus) lcus = dc.cus[CONJUNCT_FORM];
	if (lcus && lcus->next == NULL) {
	    /* only one char code in this list */
	    if (lcus->u_pschar == NO_PSCHAR) {
	        /* now check whether a ligature for n-2 and n-3 exists.
	         * in which case the RA will be consumed, or else we
	         * need to include it */
		c1 = _I_(dlet.cons[dlet.n - 3]);
		c2 = _I_(dlet.cons[dlet.n - 2]);
		dc = fptr->ligatures[c1][c2];
		if (dc.cus && !dlet.nolig[dlet.n - 3]) {
		    /* if previous ligature exists, and it is not suppressed,
		     * then turn on ra_hack
		     */
		    ra_hack = TRUE;
#ifdef DEBUG
		    fprintf(stderr, "RA-HACK IS TRUE since cons[n-2] has NO_PSCHAR!\n");
#endif
		}
	    }
	}

	if (ra_hack) {
	    c1 = _I_(dlet.cons[0]);
	    c2 = _I_(dlet.cons[1]);
	    nolig = dlet.nolig[0]; /* if TRUE, then do not use ligature */
	} else {
	    c1 = _I_(dlet.cons[dlet.n - 2]);
	    c2 = _I_(dlet.cons[dlet.n - 1]);
	    nolig = dlet.nolig[dlet.n-2]; /* if TRUE, then do not use ligature*/
	}

	/* if many consonants, two possibilities:
	 * consider the last two consonants, and follow the form for
	 * their ligature....
	 * (Note that this sometimes creates wrong characters,
	 * especially with the ra-cons-cons ligatures....
	 * (User can always prevent this by using the NOLIG_TOK - {} chars
	 * in the input
	 */
	if (!nolig) {
	    dc = fptr->ligatures[c1][c2];
	    if (dc.cus)  {
		clist = dc.cus[form];
		if (!ra_hack) *lastlig = TRUE;
	    }
#ifdef DEBUG
fprintf(stderr, "consmany: before same_as clist is %d cimp %d (sameas %d)\n", (int)clist, (int)cimp, dc.same_as);
#endif
	}

	if (nolig || (!clist && !dc.same_as)) {
		/* if user has prohibited the ligature from
		 * being used, or if no ligature was found....
	         * consider the last consonant, and follow the form for
 		 * its CONS..SINGLE form
	Following this path also creates funny <cons>....<cons>ra-<cons>
	sequences, i.e, the ra-<consonant> at the end of a letter
	screws up things since in hindi mode the ra-<consonant> uses
	some funky char lists .. (see dvnc.ifm, dvng.ifm)
	BUT the damage here is preferable to be one if the ligature is used..
	 */
	 
	    dc = fptr->khadi[c2];
	    if (dc.cus) clist = dc.cus[form];
	    c2only = TRUE;
	 }
#ifdef DEBUG
fprintf(stderr, "consmany: clist is %d cimp %d sameas %d c1 %d\n", (int)clist, (int)cimp, dc.same_as, c1);
#endif /*DEBUG*/

	break;
    case VOWEL_TYPE:
    case CONSONANT_SINGLE_TYPE:
    case SPECIAL_TYPE:
	fprintf(stderr, "Program ERROR: get_cus_implicit can only be called for a multiple consonant (2 or more) letter\n");
	return FALSE;
    } /* switch */

    if (clist && c2only) {
       /* this clist has been obtained from the c2 consonant.
	* Since c1 is omitted here, make sure that clist contains
	* a reference to "implicit", that way dflt_make_letter will
	* add in the chars for c1 too.
	*/
	lcus = clist;
	found = FALSE;
	while (lcus) {
	    if (lcus->u_pschar == IMPLICIT_PSCHAR) found = TRUE;
	    lcus = lcus->next;
	}
	if (!found) {
	    *lastform = dlet.v;
	    clist = &S_implicit_cu;
	} /* XXX abc: couldn;t this be a problem ?!!!
	   * needs investigation, or print a warning, this code
	   * should never be executed
	   */
    }

    /* follow the same_as pointers until get required clist */
    next = dc.same_as;
    while (!clist && next) {
	if (next->cus)  clist = next->cus[form];
#ifdef DEBUG
fprintf(stderr, "getcons: following sam_as pointer clist is %d\n", (int)clist);
#endif /*DEBUG*/
	next = next->same_as;
    }

    if (!clist) {
	fprintf(stderr, "*** Warning (line %d): Font (in %s) is missing this letter:\n",
			G_lineno, fptr->fname);
	for (i = 0;  i < dlet.n; i ++) {
	    form = dlet.cons[i];
	    fprintf(stderr, "*** Consonant %d: token (%d), IFM name (%s)\n",
			i,form,G_ifm_map[form-A_TOK].codename);
	}
	if (dlet.s > 0)
	    fprintf(stderr, "*** Accent/Special: token (%d), IFM name (%s)\n", dlet.s,
					G_ifm_map[dlet.s-A_TOK].codename);
	if (dlet.v < A_TOK)
	    fprintf(stderr, "*** Vowel: token (%d), IFM name (%s)\n",
				dlet.v+OFFSET_TOK, G_ifm_map[dlet.v].codename);
	else
	    fprintf(stderr, "*** Vowel: token (%d), IFM name (%s)\n", dlet.v,
					G_ifm_map[dlet.v-A_TOK].codename);


	fprintf(stderr, "***\n");
        return FALSE;
    }

    *cus = clist;
    *imp = cimp;
    return TRUE;
} /* S_get_cus_implicit() */
/* =========================^ dflt_mkl.c ^ ======================= */
