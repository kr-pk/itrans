@ECHO OFF
REM PC version of dodeans (not tested!)
REM Do not de-ansify header files!
SET F=iyacc.y
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
rename tmpxx %F%
SET F=itrans.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=ilex.l
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=font.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=pifm.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=ichar.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=itotex.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=itohtml.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=itops.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=utils.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=ytabc
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=lexyyc
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=tfmload.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=marker.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=iother.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=dflt_mkl.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=tmil_mkl.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=tlgu_mkl.c
echo De-Ansifying File %F% .....
./deansify < %F% > tmpxx
delete %F%
SET F=
REM done
