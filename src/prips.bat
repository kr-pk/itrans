REM Shell program to cat PostScript output files generated
REM from itrans (-P option). Prepends the font description file
REM and the prologue file. Assumes Devnagari Output............
REM PC version: creates a output file called OUT.PS

@REM for ItxGuj gujarati font
@REM COPY \itrans\lib\itrans.pro + \itrans\lib\fonts\itxguj.pfa + %1     OUT.PS

@REM for ItxBeng bengali font
@REM COPY \itrans\lib\itrans.pro + \itrans\lib\fonts\itxbeng.pfa + %1    OUT.PS

@REM for CSUtopia-Regular Romanized-Sanskrit font
@REM COPY /b \itrans\lib\itrans.pro + \itrans\lib\fonts\putr8i.pfb + %1  OUT.PS

@REM for Devnac devanagari font
COPY \itrans\lib\itrans.pro + \itrans\lib\fonts\devnac.ps + %1    OUT.PS

