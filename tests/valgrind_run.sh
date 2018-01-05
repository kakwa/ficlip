#!/bin/sh

VALGRIND_CMD="valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --error-exitcode=42"

for f in *-test*
do
$VALGRIND_CMD ./$f
done
