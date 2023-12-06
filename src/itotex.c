/*
 *========================================================================== 
 * Copyright 1991-1992 Avinash Chopde, All Rights Reserved.
 */

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/itotex.c,v 1.1.1.1 1996/11/29 01:45:34 avinash Exp $";

#include "itrans.h"

/* =================================================================== */
/* define macro names to be used*/
/* name of \newbox to be created */
#define	BOX_NAME	"zErOdEpTh"
/* name of kern macro - \def\..#1{{\kern#1em}} */
#define KERN_NAME	"kRn"
/* name of subscript macro - see macro defn in itotex_start() */
#define SUBS_NAME	"sBs"

/* =================================================================== */
static void S_get_texchar(char tchr[], int pcode);
/* =================================================================== */
/****
int cus_to_tex(comp_unit_t* cus, chain of PostScript Chars to output
	      pschar_t fm[256], the char widths (in 1/1000 EM units)
	      char texcomm[]) the tex commands returned here
*****/
int cus_to_tex(comp_unit_t* cus,
	      pschar_t fm[256],
	      char texcomm[])
{
    int dx, dy, pcode;
    char tchr[16];
    int curry = 0;
    int currx = 0;

#ifdef DEBUG
fprintf(stderr, "in totex\n");
#endif /* DEBUG */

    while (cus) {
	dx = cus->deltax;
	dy = cus->deltay;
	pcode = cus->u_pschar;

#ifdef DEBUG
fprintf(stderr, "totex: constructing char: dx %d dy %d pcode %d curr %d, %d\n", dx, dy, pcode, currx, curry);
#endif /* DEBUG */

	/* compute the dx to be used */
	if (cus->dtype_x == D_ORG) {
	    dx -= currx;
	}
	/* compute the dy to be used */
	if (cus->dtype_y == D_CUR) {
	    dy += curry;
	}

	/* apply dx */
	if (dx != 0) {
	    sprintf(texcomm, "\\%s{%.3f}", KERN_NAME, dx / 1000.0);
	    texcomm += strlen(texcomm);
	}

	/* print char, applying dy if necessary */
	if (pcode != NO_PSCHAR) {
	    S_get_texchar(tchr, pcode);
	    if (dy != 0) {
	        sprintf(texcomm, "\\%s{%.3f}{%s}", SUBS_NAME, dy/1000.0, tchr);
	        texcomm += strlen(texcomm);
			/* above macro creates a raised box, but assigns
			 * it a height and depth of 0, so that it does
			 * change the interline spacing.
			 * box name (zERoDeptH) could cause namespace
			 * collision with
			 * the users text, so use a strange mix...
			 */
	    } else { /* dy is zero... */
		sprintf(texcomm, "%s", tchr);
		texcomm += strlen(texcomm);
	    }
	} /* pcode != NO_PSCHAR */

	/* if dtype_x == D_ORG, then restore the current point to currx */
	/* (no need to restore Y back to original, since the Y basline
	 *  never changes...)
	 */
	if (cus->dtype_x == D_ORG) {
	    dx = currx - cus->deltax;

	    if (pcode >= 0 && fm[pcode].w > 0) dx -= fm[pcode].w;

	    sprintf(texcomm, "\\%s{%.3f}", KERN_NAME, dx / 1000.0);
	    texcomm += strlen(texcomm);
	}

	/* make some error checks */
	if (cus->dtype_x == D_ORG && pcode == NO_PSCHAR) {
	    fprintf(stderr, "Error: some PCC stmt has a ox coord for 'none' pcode - is meaningless\n");
	    /* is meaningless since after every character that is placed
	     * using ox, the current postion is always restored after it is
	     * printed.
	     */
	}

	if (cus->dtype_y == D_ORG && pcode == NO_PSCHAR && cus->deltay != 0) {
		fprintf(stderr,
"itotex.c:Error:cus contain an non-zero Ydelta for NO_PSCHAR (type_y DORG)\n"); 
		/* can't give any more info here, anyway, this error should
		 * have been caught when reading in the IFM file....
		 * (see font.c::fillup_font())
		 * This implies moving the Y baseline, which cannot
		 * be done!
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

	/* if required, update the curry position */
	if (pcode == NO_PSCHAR && cus->dtype_y == D_CUR) {
	    curry += cus->deltay;
	}

#ifdef DEBUG
fprintf(stderr, "totex: (width was %d) end loop curr %d, %d\n",
pcode >= 0 ? fm[pcode].w : 0, currx, curry);
#endif /* DEBUG */

	cus = cus->next;
    } /* while cus */

    return TRUE;
}
/* =================================================================== */
static void S_get_texchar(char tchr[], int pcode)
{
    char c = (char) pcode;
    
    if (isascii(c) && isalpha(c)) sprintf(tchr, "%c", c);
    else sprintf(tchr, "{\\char%d}", pcode);
}
/* =================================================================== */
int outtex_start()
{
    printf("\\newbox\\%s\n", BOX_NAME); /* box used by cus_to_tex()*/
    printf("\\def\\%s#1{{\\kern#1em}}\n", KERN_NAME);
    printf("\\def\\%s#1#2{{\\setbox\\%s=\\hbox{\\raise#1em\\hbox{#2}}%%\n\\ht\\%s=0pt\\dp\\%s=0pt\\box\\%s}}\n",SUBS_NAME,BOX_NAME,BOX_NAME,BOX_NAME,BOX_NAME);
    return TRUE;
}
int outtex_end()
{
    return TRUE;
}
/* ============================^ itotex.c ^ =========================== */
