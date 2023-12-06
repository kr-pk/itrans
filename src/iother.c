/*
 *========================================================================== 
 * Copyright 1992-1994 Avinash Chopde, All Rights Reserved.
 */

#include "itrans.h"

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/iother.c,v 1.1.1.1 1996/11/29 01:45:34 avinash Exp $";

/* =================================================================== */
/* Handle anything other than letter (digits, etc) if defined in the IFM file*/
/* handles a single character at a time */

/******
		(font_t* fptr, the font data structure to use
		int other, letter to convert 
		comp_unit_t pcus[], the comp units that make this
				    letter - all u_pschar are valid
				    PostScript codes, or NO_PSCHAR
				    
		int	size_pcus) array size of ecus- number of elements 
********/
int make_other(font_t* fptr,
		int other,
		comp_unit_t pcus[],
		int	size_pcus)
{
    int num_pcus; /* number of postscript chars that were added to pcus */
    int i;
    comp_unit_t*	startcus;
    
    num_pcus = 0;

#ifdef DEBUG
fprintf(stderr, "in make_other\n");
#endif
    i = other - '0'; /* assume: characters are ASCII */
    if (isdigit(other) && fptr && fptr->digits[other - '0']) {
	/* the IFM file has defined the CUs for this digit, use them */
	startcus = fptr->digits[i];
	while (startcus) {
#ifdef DEBUG
fprintf(stderr, "while startcus in make_other:\n");
#endif /*DEBUG*/
		/* error check */
	    if ( startcus->u_pschar != NO_PSCHAR &&
		 startcus->u_pschar < 0 ) {

		fprintf(stderr, "ERROR: %d illegal pschar in implicit comp units for digit %c\n", startcus->u_pschar, other);
	    } else {
		num_pcus += add_cus(fptr->psfm, &pcus[num_pcus], *startcus);
	    }
	    startcus = startcus->next;
	    if (num_pcus >= (size_pcus-1)) {
		fprintf(stderr, "ERROR: num_pcus execeeded (%d) for digit %c\n",
				num_pcus, other);
		fprintf(stderr, "Try increasing CUS_MAX in itrans.c and recompile\n");
		break;
	    }
	} /* while startcus */
    
    } else {
	/* NOTHING */
	/* this punctuation char has not been defined in the IFM file
	 * so, don't do anything. 
	 */
    }
	/* keep the next pointers in the pcus correct */
    for (i = 0; i < num_pcus; i ++) {
	pcus[i].next = &pcus[i+1];
    }
    if (num_pcus > 0) pcus[num_pcus-1].next = NULL;

    return num_pcus;

} /* make_other() */
/* =========================^ iother.c ^ ======================= */
