#include <stdio.h>
#include <ctype.h>

typedef struct {
    unsigned char	c;	/* input csx code */
    char*		rep;	/* output ITRANS string */
} table_t;

static table_t s_table[] = {
    'a',	"a",
    'A',	"a",
    '\xE0', 	"aa",
    '\xE2', 	"aa",
    'i',	"i",
    'I',	"i",
    '\xE3', 	"ii",
    '\xE4', 	"ii",
    'u',	"u",
    'U',	"u",
    '\xE5', 	"uu",
    '\xE7', 	".r",
    '\xE8', 	".r",
    '\xE9', 	".R",
    '\xEA', 	".R",
    '\xEB', 	".l",
    '\xEC', 	".l",
    '\xED', 	".L",
    '\xEE', 	".L",
    '\xEF', 	"\"n",
    'g',	"g",
    'G',	"g",
    'k',	"k",
    'K',	"k",
    'h',	"h",
    'H',	"h",
    'j',	"j",
    'J',	"j",
    'e',	"e",
    'E',	"e",
    'o',	"o",
    'O',	"o",
    'c',	"ch",
    'C',	"ch",
    '\xA4', 	"~n",
    '\xF1', 	"T",
    '\xF2', 	"T",
    '\xF3', 	"D",
    '\xF4', 	"D",
    '\xF5', 	"N",
    '\xF6', 	"N",
    't',	"t",
    'T',	"t",
    'd',	"d",
    'D',	"d",
    'n',	"n",
    'N',	"n",
    'p',	"p",
    'P',	"p",
    'b',	"b",
    'B',	"b",
    'm',	"m",
    'M',	"m",
    'y',	"y",
    'Y',	"y",
    'r',	"r",
    'R',	"r",
    'l',	"l",
    'L',	"l",
    'v',	"v",
    'V',	"v",
    '\xF7', 	"sh",
    '\xF8', 	"sh",
    '\xF9', 	"Sh",
    '\xFA', 	"Sh",
    's',	"s",
    'S',	"s",
    '\xFC', 	".n",
    '\xFD', 	".n",
    '\xFE', 	"H",
    '\xFF', 	"H",
    '.',	"\\.",
};

main(int argc, char argv[])
{
    int	c, i, tsize;
    int lineno = 1;

    tsize = sizeof(s_table)/sizeof(s_table[0]);

    fprintf(stderr, "Tsize is %d, [0] is %d [1] is %d, [2] is %d\n",
			tsize, s_table[0].c, s_table[1].c, s_table[2].c);

    while ( (c = getchar()) != EOF ) {

	if (c == '\n') lineno++;

	for (i = 0; i < tsize; i ++) {
	    if (c == s_table[i].c) {
		printf(s_table[i].rep);
		break;
	    }
	}

	if (i >= tsize) {
	    if (isdigit(c) || ispunct(c) || isspace(c)) {
		putchar(c);
		continue;
	    }
	    fprintf(stderr, "Line %d. Unrecognized char code: %c %d, 0%03o, 0x%02x\n",
				lineno, c, c, c, c);
	}
    }
}
