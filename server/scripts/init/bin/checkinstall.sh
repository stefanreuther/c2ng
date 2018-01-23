#!/bin/sh
#
#  PlanetsCentral - Check Installation
#
#  Run from hostdata root.
#

# Utilities not tested for: echo test
# Builtins not tested for: eval exit set export

success=true

# Required directories
for i in bin defaults games shiplist tools
do
  test -d $i || { echo "Missing directory: $i"; success=false; }
done

# Required files
for i in bin/runhost.sh bin/runmaster.sh bin/checkturn.sh bin/updateconfig.pl defaults/planet.nm defaults/race.nm defaults/storm.nm defaults/xyplan.dat
do
  test -r $i || { echo "Missing file: $i"; success=false; }
done

# Required utilities
if $success; then
  tar czf /tmp/chki$$.tgz bin/runhost.sh        || { echo "Missing/failed utility: tar";   success=false; }
  gzip -d </tmp/chki$$.tgz           >/dev/null || { echo "Missing/failed utility: gzip";  success=false; }
  rm -f /tmp/chki$$.tgz                         || { echo "Missing/failed utility: rm";    success=false; }
  zip /tmp/chki$$.zip bin/runhost.sh >/dev/null || { echo "Missing/failed utility: zip";   success=false; }
  unzip -v /tmp/chki$$.zip           >/dev/null || { echo "Missing/failed utility: unzip"; success=false; }
  cp /tmp/chki$$.zip /tmp/chki$$.cpy            || { echo "Missing/failed utility: cp";    success=false; }
  rm -f /tmp/chki$$.zip /tmp/chki$$.cpy         || { echo "Missing/failed utility: rm";    success=false; }
  perl -e 'use 5; exit 0'                       || { echo "Missing/failed utility: perl";  success=false; }
  echo foo | grep -E 'a|o'           >/dev/null || { echo "Missing/failed utility: grep";  success=false; }
fi

# Check userdata
for i in u d r
do
  test -d $i && { echo "Found user directory in host tree: $i"; success=false; }
done

# Result
if $success; then
  echo "+++ Success +++"
  exit 0
else
  echo "+++ FAILURE +++"
  exit 1
fi
