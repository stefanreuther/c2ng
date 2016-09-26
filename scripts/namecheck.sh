#!/bin/sh
# Name check

for i in . .. ../.. ../../..; do
    if test -f $i/scripts/namecheck.pl; then
        cd $i
        break
    fi
done
if ! test doxy_docs/Tagfile; then
    echo Please run doxygen.
    exit 1
fi

xsltproc scripts/names.xsl doxy_docs/Tagfile | sort | uniq | perl scripts/namecheck.pl --grammar=scripts/grammar.txt
