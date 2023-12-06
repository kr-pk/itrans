/*
 *========================================================================== 
 * Copyright 1991-1996 Avinash Chopde, All Rights Reserved.
 */

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/itops.c,v 1.1.1.1 1996/11/29 01:45:34 avinash Exp $";

#include "itrans.h"

/* =================================================================== */
int cus_to_ps(comp_unit_t* cus, /* chain of PostScript Chars to output */
	      pschar_t fm[256],
	      char pscomm[]) /* the postscript commands returned here */
{
    int dx, dy, pcode;
    int curry = 0;
    int currx = 0;

    strcpy(pscomm, "(");
    pscomm += strlen(pscomm); /* point to start to next segment */

    while (cus) {
	dx = cus->deltax;
	dy = cus->deltay;
	pcode = cus->u_pschar;

#ifdef DEBUG
fprintf(stderr, "tops: constructing char: dx %d dy %d pcode %d\n", dx, dy, pcode);
#endif /*DEBUG*/

	/* compute the dx to be used */
	if (cus->dtype_x == D_ORG) {
	    dx -= currx;
	}
	/* compute the dy to be used */
	if (cus->dtype_y == D_CUR) {
	    dy += curry;
	}

	if (dx != 0 || dy != 0) {
	    if (*(pscomm - 1) == '(') *--pscomm = '\0';
	    else {
		sprintf(pscomm, ") show ");
		pscomm += strlen(pscomm);
	    }

	    sprintf(pscomm, "%.3f %s %.3f %s rmoveto (",
			dx / 1000.0, EMTOPS, dy / 1000.0, EMTOPS);
	    pscomm += strlen(pscomm);
	    
	}

	if (pcode != NO_PSCHAR) {
	    /* output this character -- if printable, just output 
	     * the char itself, otherwise, use the octal code.
	     * "(" will screw up -- (() show! - so treat it specially
	     * same thing holds for ")"
	     */ 
	    if (isascii(pcode) && isprint(pcode) && pcode != '(' && pcode != ')') {
		*pscomm++ = pcode;
	    } else { 
		sprintf(pscomm, "\\%03o", pcode);
		pscomm += strlen(pscomm);
	    }
	}
	
	/* if any Y movement was made, restore back the correct
	 * Y coordinate (the baseline)
	 */
	if (dy != 0) {
	    if (*(pscomm - 1) == '(') *--pscomm = '\0';
	    else {
		sprintf(pscomm, ") show ");
		pscomm += strlen(pscomm);
	    }

	    sprintf(pscomm, "%.3f %s %.3f %s rmoveto (",
			0.0, EMTOPS, - dy / 1000.0, EMTOPS);
	    pscomm += strlen(pscomm);
	}

	/* if dtype_x == D_ORG, then restore the current point to currx */
	if (cus->dtype_x == D_ORG) {
	    dx = currx - cus->deltax;

	    if (pcode >= 0 && fm[pcode].w > 0) dx -= fm[pcode].w;

	    sprintf(pscomm, "%.3f %s %.3f %s rmoveto (",
			dx / 1000.0, EMTOPS, 0.0, EMTOPS);
	    pscomm += strlen(pscomm);
	}

	/* make some error checks */
	if (cus->dtype_x == D_ORG && pcode == NO_PSCHAR) {
	    fprintf(stderr, "Error: some PCC stmt has a ox coord for 'none' pcode - is meaningless\n");
	    /* is meaningless since after every character that is placed
	     * using ox, the current postion is always restored after it is
	     * printed.
	     */
	}
	/* modify the curr coordinates */
	/* update current position for the x coord */
	if (cus->dtype_x == D_CUR) {
	    currx += cus->deltax;
	    if (pcode >= 0 && fm[pcode].w > 0) currx += fm[pcode].w;
	} else {
	    /* is D_ORG, nothing to do, the current position is kept intact */
	}

	if (pcode == NO_PSCHAR && cus->dtype_y == D_CUR) {
	    curry += cus->deltay;
	}
	
	cus = cus->next;
    } /* while cus */

    if (*(pscomm - 1) == '(') *--pscomm = '\0';
    else {
	sprintf(pscomm, ") show ");
	pscomm += strlen(pscomm);
    }
    strcpy(pscomm, "\n");
    return TRUE;
}
/* =================================================================== */
int outps_start(int fsize)
{
    printf("/%s %d def /endy endy %s sub def 50 endy moveto\n",
			EMSIZE, fsize, EMSIZE);
    /* normalfont does not seem to work, need the EMSIZE up above 
    printf("%d normalfont\n", fsize);
    without the EMSIZE def and moveto above get errors about
    nocurrentpoint from PostScript
    */
    return TRUE;
}
int outps_end()
{
    printf("\nshowpage\n");
    return TRUE;
}
/* ============================^ itops.c ^ =========================== */
