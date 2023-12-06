#   Makefile for itrans using gcc.
#   Linux systems can use this, and so can any other UNIX system that
#   has gcc (and, optionally, flex & yacc) installed.
#   On Windows, use MinGW - install the Dev Kit with C compiler and make
#   ifdefs in this file trigger on WINDIR environment variable for Windows.
#
# This file has to be manually edited with correct locations desired
# for the install.
#
#$Header:$

SHELL=/bin/sh

# change following definitions as required:
# BINDIR will be used to copy the "itrans" executable
# INSTALLDIR will be used for data and doc files.
# Will store the lib/ contrib/ etc folders for itrans
# where the manual page goes
# MANDIR = $(INSTALLDIR)/doc

ifndef WINDIR
  BINDIR=/usr/local/bin
  INSTALLDIR=/usr/share/itrans53
  ILIB=-DITRANSLIB=\"$(INSTALLDIR)/lib\"
  MANDIR = /usr/share/man/man1
else
  INSTALLDIR=C:\\\\itrans-5.3
  BINDIR=$(INSTALLDIR)\\\\src
  ILIB=-DITRANSLIB=\"$(INSTALLDIR)\\\\lib\"
  MANDIR=$(INSTALLDIR)\\\\doc
endif


# for SYSV (and friends which use <string.h> instead of <strings.h>)
# define the c-compiler flag
SYS = -DSYSV

# if your C compiler understands prototypes, define PROTO_C,
# else keep PROTO null
# PROTO =
# On Linux, I presume you are using gcc, and it understands prototypes.
# PROTO=-DPROTO_C=1 -DYY_USE_PROTOS
# On linux YY_USE_PROTOS is always defined
PROTO=-DPROTO_C=1

# code optimizer, and/or debug symbols
# either use
# OPT = -O
# OPT = -O2
# OPT = -g -O
# OPT = -g -O  -DDEBUG=1
OPT = -g -O 

# libraries to include (-lc on most systems)
# FLIBS= -lmalloc -lc # Use this for SGI machines
# FLIBS= -lc # any other UNIX machine...
# FLIBS= -lc # for Lunix (no need to use -lfl)

# C flags to use:
# CFLAGS = -acpp -prototypes $(OPT) $(SYS) $(PROTO) $(ILIB) # SGI
# CFLAGS = -Wall -Wstrict-prototypes $(OPT) $(SYS) $(PROTO) $(ILIB) # gcc
# CFLAGS = $(OPT) $(SYS) $(PROTO) $(ILIB) # any Unix...

CFLAGS = -Wall -Wstrict-prototypes $(OPT) $(SYS) $(PROTO) $(ILIB) # gcc

CC=gcc

# Yacc flags:
YFLAGS= -d

###

SRC =	iyacc.y \
	ilex.l \
	itrans.c \
	marker.c \
	font.c \
	tfmload.c \
	pifm.c \
	ichar.c \
	dflt_mkl.c \
	tmil_mkl.c \
	tlgu_mkl.c \
	iother.c \
	itotex.c \
	itops.c \
	itohtml.c \
	utils.c

OBJ =	itrans.o \
	iyacc.o \
	ilex.o \
	marker.o \
	font.o \
	tfmload.o \
	pifm.o \
	ichar.o \
	dflt_mkl.o \
	tmil_mkl.o \
	tlgu_mkl.o \
	iother.o \
	itotex.o \
	itops.o \
	itohtml.o \
	utils.o

FILES =

.c.o:
	$(CC) -c $(CFLAGS) $<

all : itrans

itrans: iyacc.o ilex.o $(OBJ)
	$(CC) -o itrans $(CFLAGS) $(OBJ) $(FLIBS)

$(OBJ) : itrans.h imap.h ifm.h version.h y_tab.h

# if you need to edit ilex.l, flex is required (AT&T lex will not work)
ilex.c : ilex.l
	flex -v -8 -t ilex.l > ilex.c

# if you need to edit iyacc.y, yacc is required
iyacc.c y_tab.h: iyacc.y
	yacc $(YFLAGS) iyacc.y
	mv y.tab.c iyacc.c
	mv y.tab.h y_tab.h

csx2i:	csx2i.c
	$(CC) $(CFLAGS) csx2i.c -o csx2i

install :
ifndef WINDIR
	-mkdir -p $(BINDIR) $(MANDIR)
	-mkdir -p $(INSTALLDIR)/lib/fonts
	-mkdir -p $(INSTALLDIR)/doc $(INSTALLDIR)/contrib
	cp itrans itrans-post.pl $(BINDIR)/
	-cp itrans.1 $(MANDIR)/
	cd ..; grep -v 'src/' itrans.lst | sed -e 's/^[^/]*\///' | cpio -pvm -R root: $(INSTALLDIR)
	cd ..; grep 'fonts/' itransfn.lst | sed -e 's/^[^/]*\///' | cpio -pvm -R root: $(INSTALLDIR)
	@echo
	@echo "Installed exec: $(BINDIR)/itrans; data files: $(INSTALLDIR)/"
else
	@echo Installed exec: $(BINDIR)\itrans.exe 
	@echo data files: $(INSTALLDIR)\ lib doc
endif

# ------------------------------------------------------------------------
# Jul 2001: nobody probably uses direct PostScript out...
oldstuff:
	cp prips $(BINDIR)/prips
	-cp prips.1 $(MANDIR)/prips.$(MANEXT)

# ------------------------------------------------------------------------
catman : itrans.man prips.man

itrans.man :
	nroff -man itrans.$(MANEXT) > itrans.man

prips.man :
	nroff -man prips.$(MANEXT) > prips.man

# ------------------------------------------------------------------------
clean :
	rm -f $(OBJ) itrans

# use clean-all if you have flex/yacc on your system
clean-all :
	rm -f $(OBJ) ilex.c iyacc.c itrans

# ------------------------------------------------------------------------
