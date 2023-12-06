/*
 *========================================================================== 
 * Copyright 1996-2001 Avinash Chopde, All Rights Reserved.
 */

//static char S_RCSID[] = "$Header:$";

#include "itrans.h"

static int S_current_mode = OUTPUT_HTML7;

static void S_html_escape(int c, char out[]);
static int S_to_utf8(int c, char out[]);

/* =================================================================== */
/* just creates a array of "pcodes" -- all dx, dy is ignored - screen
 * fonts assumed.
 * Output is in HTML format
 * For ISO-8859-1 (Latin1) output, either 7-bit or 8-bit mode
 * can be choosen:
 * raw characters output for char codes 0..127 (except for &lt; etc), and
 * if 7-bit: chars 128..255 are output using &###; codes 
 * if 8-bit: chars 128..255 are output directly (no transformation)
 * For ISO-10646/Unicode (UTF-8) output, the input IFM file is assumed
 * to have Unicode code points (cus->u_pschar), and the output is
 * made in UTF-8 format.
 */
int cus_to_html(comp_unit_t* cus, /* chain of font characters to output */
	      pschar_t fm[256], /* unused */
	      char out[]) /* the characters returned here */
{
    int pcode, n;
    char utf8[16];

    out[0] = '\0';

    while (cus) {
	pcode = cus->u_pschar;

#ifdef DEBUG
fprintf(stderr, "tohtml: constructing char: pcode %d\n", pcode);
#endif /*DEBUG*/

	if (pcode != NO_PSCHAR) {

	    n = 1;

	    if (S_current_mode == OUTPUT_HTML_UTF8) {
	        n = S_to_utf8(pcode, utf8);
	    }

	    /* output this character -- char code will be between 0 & 255
	     */ 
	    if (n == 1) {
		/* is ASCII output mode, or UTF-8 but this char is ASCII */
		S_html_escape(pcode, out);
	    } else {
		strcat(out, utf8);
	    }

	    out += strlen(out);
	}
	
	cus = cus->next;
    } /* while cus */

    *out = '\0';
    return TRUE;
}
/* =================================================================== */
int outhtml_start(int fsize)
{
    return TRUE;
}
int outhtml_end()
{
    return TRUE;
}

int outhtml_mode(int m)
{
    int prev = S_current_mode;

    if (m == OUTPUT_HTML8 || m == OUTPUT_HTML7 || m == OUTPUT_HTML_UTF8) {
	S_current_mode = m;
    } else {
        fprintf(stderr, "Program error: outhtml_mode called with illegal value (%d)\n", m);
    }
    return (prev);
}

/* =================================================================== */
static void S_html_escape(int c, char out[])
{
    if (c == '<') {
	sprintf(out, "&lt;");
    } else if (c == '>') {
	sprintf(out, "&gt;");
    } else if (c == '&') {
	sprintf(out, "&amp;");
    } else if (c == '"') {
	sprintf(out, "&#%03d;", c); /* does &quot exist? */
    } else if (isprint(c)) {
	out[0] = (char) c;
	out[1] = '\0';
    } else { /* print out char code */
	if (S_current_mode == OUTPUT_HTML7) {
	    sprintf(out, "&#%03d;", c);
	} else { /* 8_BITS or UTF_8 */
	    out[0] = (char) c;
	    out[1] = '\0';
	}
    }
}
/* =================================================================== */
/* convert integer to UTF-8 */
static int S_to_utf8(int c, char out[])  /* putwchar(c) */
{
  int i = 0;

  if (c < 0x80) {
    out[i++] = (c);
  }
  else if (c < 0x800) {
    out[i++] = (0xC0 | (c>>6));
    out[i++] = (0x80 | (c & 0x3F));
  }
  else if (c < 0x10000) {
    out[i++] = (0xE0 | (c>>12));
    out[i++] = (0x80 | ((c>>6) & 0x3F));
    out[i++] = (0x80 | (c & 0x3F));
  }
  else if (c < 0x200000) {
    out[i++] = (0xF0 | (c>>18));
    out[i++] = (0x80 | ((c>>12) & 0x3F));
    out[i++] = (0x80 | ((c>>6) & 0x3F));
    out[i++] = (0x80 | (c & 0x3F));
  }

  out[i] = '\0';
  return i;
}
/* ============================^ itohtml.c ^ =========================== */
