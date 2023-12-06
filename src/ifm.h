#ifndef IFM_H
#define IFM_H

/*
 *========================================================================== 
 * Copyright 1991-1992 Avinash Chopde, All Rights Reserved.
 */

/* static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/ifm.h,v 1.1.1.1 1996/11/29 01:45:34 avinash Exp $"; */

#include <stdio.h>

#if defined(__STDC__) || (PROTO_C)
#   define P(s)	s
#else
#   define P(s) ()
#endif

/* Lines from the IFM file may contain these tokens: CC, CCS, CCADD,
 * word, number, etc
 * Examples:
(start of indian script composite char metrics:)
itrans: StartINDIAN
(for postscript fonts:
 indian name,  font program file, AFM metric file (or none, is absent)
itrans: POSTSCRIPT marathi devnac.ps devnac.afm
(for metafont fonts (TeX): indian name - with the TFM metric file)
itrans: TEX sanskrit dvng10.tfm
(for direct text output fonts: indian name - with no metric files)
itrans: TEXT_TTY roman
(indian script composite char metrics:)
itrans: CC a 2 ; PCC 97 0 0 ; PCC 129 -70 0 ;
(special composite char metrics added:)
itrans: CCADD normA ;
(define equivalence bewteen khadi's:)
itrans: CCS normAkp normA ;
(end of indian script composite char metrics:)
itrans: EndINDIAN
 *
 */

/* tokens returned by pifm.c, values must be > 0 and distinct */
#define COMMENT_IFMTAG		1
#define IFM_IFMTAG		2
#define ENDIFM_IFMTAG		3
#define CC_IFMTAG		4
#define CCS_IFMTAG		5
#define CCADD_IFMTAG		6
#define PCC_IFMTAG		7
#define DNAME_IFMTAG		8
#define NUMBER_IFMTAG		9
#define IMPLICIT_IFMTAG		10
#define NONE_IFMTAG		11
#define PSF_IFMTAG		12
#define TEXF_IFMTAG		13
#define DORG_IFMTAG		14
#define DCUR_IFMTAG		15
#define HCHAR_IFMTAG		16
#define DEFVOWEL_IFMTAG		17
#define TXTF_IFMTAG		18
#define UNICODEF_IFMTAG		19

extern int G_ifm_lineno; /* current line number, IFM file */

/** extern functions */
extern int get_ifm_token	P((FILE* fp, char* word));
extern void reset_pifm		P((void));
extern void tfmload		P((FILE* tfmfp, font_t* font, int scaledsize));

/* ===========================^ ifm.h ^============================== */
#undef P
#endif /* IFM_H */
