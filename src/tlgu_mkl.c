/*
 *========================================================================== 
 * Copyright 1991-1992 Avinash Chopde, All Rights Reserved.
 */

#include "itrans.h"

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/tlgu_mkl.c,v 1.2 1997/05/17 19:10:53 avinash Exp $";

/* =================================================================== */
/* Handle a telugu letter (possibly with multiple consonants) */
/* see ichar.c also */

/******
		(font_t* fptr, the font data structure to use
		letter_t dlet, letter to convert 
		comp_unit_t pcus[], the comp units that make this
				    letter - all u_pschar are valid
				    PostScript codes, or NO_PSCHAR
				    
		int	size_pcus) array size of ecus- number of elements 
********/
int telugu_make_letter(font_t* fptr,
		letter_t dlet,
		comp_unit_t pcus[],
		int	size_pcus)
{
    int num_pcus = 0;
    comp_unit_t *cus, *imp;
    letter_t tl;
    int i;

#ifdef DEBUG
fprintf(stderr, "tlgumkl: dlet.n %d, v %d, cons0 %d\n",
dlet.n, dlet.v, dlet.cons[0]);
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
#ifdef DEBUG
fprintf(stderr, "tlgu: cs_v_s: current count %d cus\n", num_pcus);
#endif

        break;
	
    case CONSONANT_DOUBLE_TYPE:
    case CONSONANT_MANY_TYPE:
	/* telugu has no ligatures, but the base consonant 
	 * in a list of consonants is the FIRST consonant!
	 * Therefore, given a multiple consonant akshara,
	 * C1 + C2 + C3 ... + CN + V
	 * the first thing printed is C1 + V, then all the
	 * C2..N consonants in consonant conjunct form (NOT half forms).
	 * Dec 1998: Even though Telgu has no ligatures/conjuncts, 
	 * the order in which components are printed is dependent on the
	 * font, so it is necessary to look for C1-C2-V form in the IFM
	 * file before using C1-V and C2-conjunct, etc.
	 */

	/* first, check for complete definition - C1-C2-...-V */
	if (get_cus_explicit(fptr, dlet, &cus, &imp)) {
	    num_pcus += add_cus_list(fptr->psfm, &pcus[num_pcus], cus, imp);
#ifdef DEBUG
fprintf(stderr, "tlgu: cons double/many: current count %d cus\n", num_pcus);
#endif
	} else {
	    /* exact defn not found, break it up */
	    tl.type = CONSONANT_SINGLE_TYPE;
	    tl.n = 1;
	    for (i = 0; i < dlet.n; i ++) {
		tl.cons[0] = dlet.cons[i];
		if (i == 0) { /* first consonant, use vowel form */
		    tl.v = dlet.v;
		} else {
		    tl.v = CONJUNCT_FORM;
		}
		
		/* construct this single consonant */
		if (!get_cus_explicit(fptr, tl, &cus, &imp)) {
		    /* error - message is printed by get_cus_explicit.. */
		    return 0; /* 0 elements added */
		}

		/* add the list of comp units to the output array,
		 * taking care of any implicit pointers 
		 */
		num_pcus += add_cus_list(fptr->psfm, &pcus[num_pcus], cus, imp);
    #ifdef DEBUG
    fprintf(stderr, "tlgu: cons double/many: current count %d cus\n", num_pcus);
    #endif
	    }
	}
        break;

    case SPECIAL_TYPE:
	/* handled after this switch */
        break;
        
    default:
	fprintf(stderr, "telugu_make_letter: error illegal letter type %d\n", dlet.type);
        break;
    } /* switch */

     /* handle special character */
    if (dlet.s > 0) {
	num_pcus += add_special(fptr, dlet.s, &pcus[num_pcus],
				size_pcus - num_pcus);
    }
	
    /* need to chain the pcus correctly */
    for (i = 0; i < num_pcus; i ++) {
	pcus[i].next = &pcus[i+1];
    }
    if (num_pcus > 0) pcus[num_pcus-1].next = NULL;
    
    return num_pcus;
} /* telugu_make_letter() */
/* ============================^ tlgu_mkl.c ^ =========================== */
