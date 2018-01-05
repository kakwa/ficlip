#!/bin/sh

ret=0
VALGRIND_CMD="valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --error-exitcode=42"

for f in *test*
do
if [ -f $f ] && [ -x $f ]
then
    $VALGRIND_CMD ./$f
    ret=$(( $? + $ret ))
fi
done

exit $ret
