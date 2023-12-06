/*
 *   26 April 1992
 *   abc: taken from the dvips package written by Toman Rokicki
 *   Did get permission from Tomas to use his source file.
 *   Anyway, this file now looks quite different from the original,
 *   I only kept the parts I needed.
 *   
 *   Loads a tfm file-- just read in the TFM widths.
 */
//static char S_RCSID[] = "$Header: /home/cvsroot/itrans/nextrel/src/tfmload.c,v 1.3 1997/05/17 21:02:42 avinash Exp $";

#include "itrans.h"
#include <stdio.h>

/* abc: definitions from structures.h */
/*
 *   Type declarations.  integer must be a 32-bit signed; shalfword must
 *   be a sixteen-bit signed; halfword must be a sixteen-bit unsigned;
 *   quarterword must be an eight-bit unsigned.
 */
typedef long integer;
typedef char boolean;
typedef double real;
typedef short shalfword ;
typedef unsigned short halfword ;
typedef unsigned char quarterword ;
typedef short Boolean ;

/*
 *   These are the external routines it calls:
extern integer scalewidth() ;
 */
/* abc: scalewidth.c */
/*
 *   Code to scale dimensions.  Takes two thirty-two bit integers, multiplies
 *   them, divides them by 2^20, and returns the thirty-two bit result.
 *   The first integer, the width in FIXes, can lie between -2^24 and 2^24-1.
 *   The second integer, the scale factor, can lie between 0 and 2^27-1.  The
 *   arithmetic must be exact.  The answer is truncated to an integer.
 *
 *   Since this math is special, we put it in its own file.  It is the only
 *   place in the program where such accuracy is required.
 */

integer
scalewidth( register integer a, register integer b )
{
  register integer al, bl ;
  al = a & 32767 ;
  bl = b & 32767 ;
  a >>= 15 ;
  b >>= 15 ;
  return ( ((al*bl/32768) + a*bl+al*b)/32 + a*b*1024) ;
}

static FILE* tfmfile;

static shalfword tfmbyte (void);
static halfword tfm16 (void);
static integer tfm32 (void);

static
shalfword
tfmbyte ()
{
  return(getc(tfmfile)) ;
}

static
halfword
tfm16 ()
{
  register halfword a ; 
  a = tfmbyte () ; 
  return ( a * 256 + tfmbyte () ) ; 
} 

static
integer
tfm32 ()
{
  register integer a ; 
  a = tfm16 () ; 
  if (a > 32767) a -= 65536 ;
  return ( a * 65536 + tfm16 () ) ; 
} 

void tfmload(FILE* tfmfp, font_t* font, int scaledsize)
{
   register shalfword i ;
   register integer li ;
   shalfword nw, hd ;
   shalfword bc, ec ;
   integer scaled[256] ;
   halfword chardat[256] ;
   pschar_t*	psch = font->psfm;

   tfmfile = tfmfp;
/*
 *   Next, we read the font data from the tfm file, and store it in
 *   our own arrays.
 */
   li = tfm16() ; hd = tfm16() ;
   bc = tfm16() ; ec = tfm16() ;
   nw = tfm16() ;
   li = tfm32() ; li = tfm32() ; li = tfm32() ; li = tfm16() ;
   li = tfm32() ;
#if 0
   /* abc: do not have the fontdesc... structure (from structures.h 
    * in the dvips package.
    */
   if (li && curfnt->checksum)
      if (li!=curfnt->checksum) {
         (void)sprintf(errbuf,"Checksum mismatch in %s", name) ;
         error(errbuf) ;
       }
   li = (integer)(alpha * (real)tfm32()) ;
   if (li > curfnt->designsize + fsizetol ||
       li < curfnt->designsize - fsizetol) {
      (void)sprintf(errbuf,"Design size mismatch in %s", name) ;
      error(errbuf) ;
   }
#else
   /* abc: do not read in the checksum, etc, so need to consume the
    * four bytes...
    */
   (void) tfm32();
#endif

   for (i=2; i<hd; i++)
      li = tfm32() ;
   for (i=0; i<256; i++)
      chardat[i] = 256 ;
   for (i=bc; i<=ec; i++) {
      chardat[i] = tfmbyte() ;
      li = tfm16() ;
      li = tfmbyte() ;
   }
   for (i=0; i<nw; i++)
      scaled[i] = scalewidth(tfm32(), scaledsize) ;

#if 0 /* abc - this func did not open the file, it should not close it!!  */
   (void)fclose(tfmfile) ;
#endif

   for (i=0; i<256; i++)
      if (chardat[i]!= 256) {
         li = scaled[chardat[i]] ;
	 psch[i].w = (int) li; /* width */
	 psch[i].llx = psch[i].lly = psch[i].urx = psch[i].urx = 0;
#if 0
         curfnt->chardesc[i].TFMwidth = li ;
         curfnt->chardesc[i].pixelwidth = ((integer)(conv*li+0.5)) ;
         curfnt->chardesc[i].flags = (curfnt->resfont ? EXISTS : 0) ;
#endif
      }
}
