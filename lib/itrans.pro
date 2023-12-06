%!PS
% PostScript Prolog for the itrans program...
% RCSID: $Header: /home/cvsroot/itrans/nextrel/lib/itrans.pro,v 1.1.1.1 1996/11/29 01:45:32 avinash Exp $
%
% ========================================================================== 
% Copyright 1991-1996 Avinash Chopde, All Rights Reserved.
%
% Permission to use, copy, modify and distribute this software and its
% documentation for any purpose is hereby granted without fee, provided that
% the above copyright notice appear in all copies and that both that
% copyright notice and this permission notice appear in supporting
% documentation, and that the name of Avinash Chopde not be used in
% advertising or publicity pertaining to distribution of the software
% without specific, written prior permission.
% Avinash Chopde makes no representations about the suitability of this
% software for any purpose.
% It is provided "as is" without express or implied warranty.
%
% AVINASH CHOPDE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
% INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
% IN NO EVENT SHALL AVINASH CHOPDE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
% CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
% DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
% TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
% OF THIS SOFTWARE.
%
% Author:  Avinash Chopde.
%

/endx 500 def
/endy 790 def
/EMSIZE 40 def
% program redefines EMSIZE value, otherwise use 40 point size

/LINESKIPOFFSET 2 def
/BASELINESKIP EMSIZE LINESKIPOFFSET add def
% height of each line - by default, same as the font point size,
% but .ips user files may redefine LINESKIPOFFSET, if the lineskip needs to be
% different (font size is incorrect, or user wants double spacing, etc)

/checkline {  % -- checkline  -> -- (uses EMSIZE global variable)
    currentpoint
    % (in checkline) == pstack
    /y exch def
    EMSIZE add endx ge { % add EMSIZE to x on stack
    	    /y y BASELINESKIP sub def % go to next line
	    y 10 le { % check if overflow on next page
		/y endy BASELINESKIP sub def
		showpage
	    } if

	    50 y moveto
	}  if % x > endx, reached end of line
    % (end of checkline) == pstack
} def

/newline {
    currentpoint
    % (in checkline) == pstack
    /y exch def
    pop
    /y y BASELINESKIP sub def % go to next line
    y 10 le { % check if overflow on next page
	/y endy BASELINESKIP sub def
	showpage
    } if
    50 y moveto
} def

/newpage { % clear out this page, and start off on a new page
    /y endy BASELINESKIP sub def
    showpage
    50 y moveto
} def

/EM { % expects a em-unit on stack, converts it to PostScript points
    EMSIZE mul
} def

% BASEFONTNAME should be defined by the user program..
% so that it refers to the correct font name (bengali font, etc)
% (itrans.c::main() defines it to be a devenagari font...)

/basefont {
    BASEFONTNAME findfont
} def

% <fontsize in points> normalfont --
/normalfont {		% sets font to be the normal font of BASEFONTNAME
    /EMSIZE exch def
    /BASELINESKIP EMSIZE LINESKIPOFFSET add def
    basefont [EMSIZE 0 0 EMSIZE 0 0] makefont setfont
} def

% <fontsize in points> slantfont --
/slantfont {		% sets up a slanted version of font BASEFONTNAME
    /EMSIZE exch def
    /BASELINESKIP EMSIZE LINESKIPOFFSET add def
    basefont [EMSIZE 0  0.167 EMSIZE mul  EMSIZE 0 0] makefont setfont
} def

% <fontsize in points> expandedfont --
/expandedfont { 	% expanded version of BASEFONTNAME
    /EMSIZE exch def
    /BASELINESKIP EMSIZE LINESKIPOFFSET add def
    basefont [EMSIZE 1.2 mul  0 0 EMSIZE 0 0] makefont setfont
} def

% <fontsize in points> compressedfont --
/compressedfont { 	% compressed (x-axis) version of BASEFONTNAME
    /EMSIZE exch def
    /BASELINESKIP EMSIZE LINESKIPOFFSET add def
    basefont [EMSIZE 0.82 mul  0 0 EMSIZE 0 0] makefont setfont
} def

% <fontsize in points> englishfont --
/englishfont {
    /EMSIZE exch def
    /BASELINESKIP EMSIZE LINESKIPOFFSET add def
    % /Times-Roman
    /Courier-Bold
    findfont EMSIZE scalefont setfont
} def

