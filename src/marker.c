/*
 *========================================================================== 
 * Copyright 1991-1992 Avinash Chopde, All Rights Reserved.
 */

//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/marker.c,v 1.1.1.1 1996/11/29 01:45:34 avinash Exp $";

#include "itrans.h"

/* =================================================================== */
void init_markers(marker_t *l, int num)
{
    int i;
    for (i = 0; i < num; i ++) {

	null_marker(l + i);

	switch(i + IMARKER_TOK) {
	case MARATHI_TOK:
	    l[i].name = "marathi";
	    break;
	case HINDI_TOK:
	    l[i].name = "hindi";
	    break;
	case ORIYA_TOK:
	    l[i].name = "oriya";
	    break;
	case KANNADA_TOK:
	    l[i].name = "kannda";
	    break;
	case SANSKRIT_TOK:
	    l[i].name = "sanskrit";
	    break;
	case TAMIL_TOK:
	    l[i].name = "tamil";
	    break;
	case BENGALI_TOK:
	    l[i].name = "bengali";
	    break;
	case TELUGU_TOK:
	    l[i].name = "telugu";
	    break;
	case MALAYALAM_TOK:
	    l[i].name = "malyalam";
	    break;
	case GUJARATI_TOK:
	    l[i].name = "gujarati";
	    break;

	case IMARKER_TOK:
	default:
	    l[i].name = "default indian language";
	    break;
	}
    }
}
/* =================================================================== */
void null_marker(marker_t *l)
{
    l->curr_ifmname = NULL;
    l->curr_fontcmd = NULL;
    l->curr_font = NULL;
}
/* =================================================================== */
void clear_marker(marker_t *l)
{
    if (l->curr_ifmname) free(l->curr_ifmname);
    if (l->curr_fontcmd) free(l->curr_fontcmd);

    /* don't free font (it is kept in allfonts also), just make it NULL */

    null_marker(l);
}
/* =================================================================== */
/* cmd is of the form: "\XXXifm=YYY"
 * first of all, the ifm file is searched for, and if not found, is
 * loaded in.
 * Then, the language structure is filled in.
 */
int find_load_ifm(marker_t* l, allfonts_t allf, char ifmtok[])
{
    char name[NAMELEN];
    char *eqp;
    int s;
    font_t* ff;
    
    eqp = strchr(ifmtok, '=');
    if (eqp == NULL) {
        fprintf(stderr, "Illegal Indian IFM name line: %s\n", ifmtok);
        return FALSE;
    }

    eqp ++;
    sprintf(name, "%s", eqp); /* to eat up all white space around it..*/

    if (strlen(name) < 1) {
	if (G_verbose > 1) fprintf(stderr, "Null IFM statement (line %d) -- ignored\n",
		G_lineno);
	return FALSE;

    }

    if (chk_ext(name, ".ifm")) { /* missing .ifm, or wrong extension..*/
        strcat(name, ".ifm");
    }

    ff = find_font(allf, name);
    if (!ff) {

        if (G_verbose > 0) fprintf(stderr, "Loading IFM file %s ...", name);

        /* allocate space for a single font.. */

        ff = (font_t*) malloc (sizeof(*ff));
        init_font(ff);

        s = fillup_font(ff, name);

        if (!s) {
	     fprintf(stderr, "\nError: fillup font error (fill up from %s)\n", name);
	     free(ff);
	     return FALSE;
        }

	/* stuff it in the array of IFM files.... */
	s = 0;
	while (allf[s]) s++; 
	if (s >= FONTS_MAX) {
	    fprintf(stderr, "Warning: cannot load in any more IFM files, will reuse previous location...\n");
	    fprintf(stderr, "(Max IFM files is %d, Current Line is %d, Loading %s)\n", 
				FONTS_MAX, G_lineno, name);
	    s--;
	    free(allf[s]);
	}
	allf[s] = ff;

        if (G_verbose > 0) fprintf(stderr, ".. loaded!\n");

#ifdef DEBUG
        fprintf(stderr, "successfully loaded ifm: %s at loc %d\n", name, s);
#endif /* DEBUG*/
    } else {
        if (G_verbose > 0)
	    fprintf(stderr, "Loading IFM file %s ..... already loaded!\n", name);
    }

    l->curr_font = ff;

    if (l->curr_ifmname) free(l->curr_ifmname);

    eqp = strrchr(name, DIRSEP);
    if (!eqp) eqp = name;
    else eqp++;
    l->curr_ifmname = (char*) malloc( sizeof(char) * (strlen(eqp) + 1));
    strcpy(l->curr_ifmname, eqp);

    return TRUE;
}
/* =================================================================== */
int get_lang_token(char name[])
{
    int i;
    int n = strlen(name);

    for (i = 0; i < n; i ++) name[i] = tolower(name[i]);

    if (!strcmp(name, "marathi"))	return MARATHI_TOK;
    if (!strcmp(name, "hindi"))		return HINDI_TOK;
    if (!strcmp(name, "oriya"))		return ORIYA_TOK;
    if (!strcmp(name, "kannada"))	return KANNADA_TOK;
    if (!strcmp(name, "sanskrit"))	return SANSKRIT_TOK;
    if (!strcmp(name, "tamil"))		return TAMIL_TOK;
    if (!strcmp(name, "bengali"))	return BENGALI_TOK;
    if (!strcmp(name, "telugu"))	return TELUGU_TOK;
    if (!strcmp(name, "malayalam"))	return MALAYALAM_TOK;
    if (!strcmp(name, "gujarati"))	return GUJARATI_TOK;
    if (!strcmp(name, "gurmukhi"))	return GURMUKHI_TOK;
    if (!strcmp(name, "roman"))		return ROMAN_TOK;

    return -1; /* unknown language */
}
/* =================================================================== */
