#!/bin/sh
#
#  Test quality quick-check
#
#  This script was mainly used in the cxxtest -> internal framework porting effort,
#  but still can serve to catch some mistakes.
#

u="[4m"
e="[0m"

# Find test names
n=/tmp/names.$$
find test -name "*.cpp" | xargs cat | perl -ne '/^(AFL_TEST|AFL_TEST_NOARG)\("(.*)"/ and print "$2\n"' > $n

# Report duplicate names
echo "${u}Duplicate names:$e"
sort < $n | uniq -d

# Report names unported from old version or old habits. Should be none anymore.
echo "${u}Unported names:$e"
grep -E ':test|It$' < $n | grep -vE '^server\.file\.FileBase:testFiles$'

# Content. Reject some anti-pattersn.
echo "${u}Suspicious:$e"
find test -name "*.cpp" | xargs grep -nE '(ExpressionVerifier|CallReceiver|ValueVerifier|ContextVerifier|CommandHandler)[^(]*\([^(]*"|// AFL_TEST|^ *afl::test::Assert *[^ (]*\("|\ba\("[0-9][0-9]*"|AFL_CHECK_SUCCEEDS\(.*checkFinish\(\)\)|[Mm]ock\("test|, *! +[a-zA-Z].*\)'
# ValueVerifier/ContextVerifier/CallReceiver/Assert with hardcoded name (should be derived from function parameter)
#   'ValueVerifier x("abc")', should be 'ValueVerifier x(a("abc"))'
# unported test
# sub-asserter created with auto-generated name (should have meaningful name)
#   'a("01")' should e.g. be 'a("01. call function")'
# checkFinish() should not be guarded by AFL_CHECK_SUCCEEDS(), it already generates proper tracking
# "! " usually created by alignment in 'check(..., !foo)'; add some heuristic that we don't match existing strings/comments

# Stats
echo "${u}Stats:$e"
echo "  tests: $(wc -l <"$n")"

rm "$n"
