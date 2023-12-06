#ifndef IMAP_H
#define IMAP_H

/*
 *========================================================================== 
 * Copyright 1991-1992 Avinash Chopde, All Rights Reserved.
 */

/* static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/imap.h,v 1.3 1998/09/13 00:38:46 avinash Exp $"; */

#include "y_tab.h"
	/* not including y.tab.h, see makefile which renames it to
	 * y_tab.h for compatibility with DOS machines
	 */

#define	CNAMELEN	16
	/* max length of the code name of any indian char */

typedef struct {
    int		token; /* char token - assigned by y_tab.h */
    char	codename[CNAMELEN+1]; /* name assigned to it in ifm file */
} ifm_enc_t;

/* Some arbit representation of input token names, this is pretty
 * much free-form - no duplicate names allowed.
 * Used to parse the Devn Metrics file - see font.c
 * XXX
 * Will presumably be used later, to make error messages a bit
 * descriptive as to what actually expected...

 * Note:  names should be letters only (case is significant)
 *        do not use "-" in the names.
 * Any changes here implies that the file font.c needs to be edited:
 * the struct S_ifm_map has to be filled in accordingly.
 */

#define A_CNAME		"a"
#define AA_CNAME	"aa"
#define I_CNAME		"i"
#define II_CNAME	"ii"
#define U_CNAME		"u"
#define UU_CNAME	"uu"
#define RI_CNAME	"ri"
#define RII_CNAME	"rii"
#define LI_CNAME	"li"
#define LII_CNAME	"lii"
#define AY_CNAME	"ay"
#define AAY_CNAME	"aay"
#define AI_CNAME	"ai"
#define O_CNAME		"o"
#define OO_CNAME	"oo"
#define AU_CNAME	"au"
#define AM_CNAME	"am"
#define AHA_CNAME	"aha"
#define HALF_CNAME	"half"
#define IMPLICIT_CNAME	"implicit"
#define CONJUNCT_CNAME	"conjunct"
#define KA_CNAME	"ka"
#define KADOT_CNAME	"kadot"
#define KHA_CNAME	"kha"
#define KHADOT_CNAME	"khadot"
#define GA_CNAME	"ga"
#define GADOT_CNAME	"gadot"
#define GHA_CNAME	"gha"
#define NGA_CNAME	"nga"
#define CHA_CNAME	"cha"
#define CHHA_CNAME	"chha"
#define JA_CNAME	"ja"
#define JADOT_CNAME	"jadot"
#define JHA_CNAME	"jha"
#define JNH_CNAME	"jnh"
#define TTA_CNAME	"tta"
#define TTHA_CNAME	"ttha"
#define DDA_CNAME	"dda"
#define DDADOT_CNAME	"ddadot"
#define DDHA_CNAME	"ddha"
#define DDHADOT_CNAME	"ddhadot"
#define NNA_CNAME	"nna"
#define TA_CNAME	"ta"
#define THA_CNAME	"tha"
#define DA_CNAME	"da"
#define DHA_CNAME	"dha"
#define NA_CNAME	"na"
#define PA_CNAME	"pa"
#define PHA_CNAME	"pha"
#define PHADOT_CNAME	"phadot"
#define BA_CNAME	"ba"
#define BHA_CNAME	"bha"
#define MA_CNAME	"ma"
#define YA_CNAME	"ya"
#define YYA_CNAME	"yya"
#define RA_CNAME	"ra"
#define LA_CNAME	"la"
#define VA_CNAME	"va"
#define SHA_CNAME	"sha"
#define SHHA_CNAME	"shha"
#define SA_CNAME	"sa"
#define HA_CNAME	"ha"
#define LDA_CNAME	"lda"
#define KSHA_CNAME	"ksha"
#define GYA_CNAME	"gya"
#define NYA_CNAME	"nya"
#define NNX_CNAME	"nnx"
#define RRA_CNAME	"rra"
#define RHA_CNAME	"rha"

#define RA_HALF_CNAME	"rahalf"
#define ANUSVARA_CNAME	"anusvara"
#define CHANDRA_CNAME	"chandra"
#define CHANDRA_BN_CNAME	"chandrabindu"
#define VIRAAM_CNAME	"viraam"
#define AVAGRAHA_CNAME	"avagraha"
#define SRI_CNAME	"sri"
#define AUM_CNAME	"aum"

/* miscellaneous characters -- these are not parsed by ilex.l or
 * iyacc.y, they are simply passed on to process_otherchar() in itrans.c
 * But, they may still have definitions  in the IFM file, used mainly
 * to take care of fonts where the digits (numbers) are not in the
 * correct ASCII location.
 * Currently, only digits may be defined in the IFM file.
 * These are not in the S_ifm_map table, so do not affect any other
 * piece of code (unlike the KA_CNAME, etc codes above)
 */
#define ZERO_CNAME	"zero"
#define ONE_CNAME	"one"
#define TWO_CNAME	"two"
#define THREE_CNAME	"three"
#define FOUR_CNAME	"four"
#define FIVE_CNAME	"five"
#define SIX_CNAME	"six"
#define SEVEN_CNAME	"seven"
#define EIGHT_CNAME	"eight"
#define NINE_CNAME	"nine"

/* num indian lang characters */
/* see font.c also */
#define FIRSTCHAR	_I_(A_TOK)
#define LASTCHAR	_I_(AUM_TOK)
#define	NUMCHARS	(LASTCHAR-FIRSTCHAR+1)
#define FIRSTVOW	_I_(A_TOK)
#define LASTVOW		_I_(AHA_TOK)
#define FIRSTCONS	_I_(KA_TOK)
#define LASTCONS	_I_(PHADOT_TOK)
	/* allow for extra chars that the user may define */
#define NUMEXTRA	32
#define	NUMKHADI	(NUMCHARS+NUMEXTRA)
#define	OFFSET_TOK	(DUMMY_TOK+1)

/* num postscript chars - 8 bits = 0 to 255 */
#define	NUMPSCHARS	256

#endif /* IMAP_H */
