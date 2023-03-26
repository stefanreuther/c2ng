#!/bin/sh
#
#  Verify the help file
#
#  Converts the help file into a temporary c2docmanager repository
#  and uses its --verify feature.
#
#  Assumes 'make install' has been done with target '.build/result',
#  alternatively, pass path name.
#
#  If there are no problems, output is empty.
#

builddir=${1:-.build/result}
if ! test -x "$builddir/bin/c2docmanager"; then
  echo "usage: $0 [path-to-installation]"
  exit 1
fi

# Temporary directory
dir=/tmp/vh$$
mkdir -p "$dir"

dm() {
  $builddir/bin/c2docmanager --single="$dir" "$@"
}

# Import files; modelled after the same logic that builds the actual doc repository for PlanetsCentral.
dm add-group --id="pcc2" --name="PCC2"
dm import-help --remove-source --below="pcc2" --id="c2ng-current" --name="PCC2ng help" \
   $builddir/share/resource/pcc2help.xml \
   $builddir/share/resource/pcc2interpreter.xml

# Verify
dm verify --warn-only

# Remove temporary files
rm -rf "$dir"
