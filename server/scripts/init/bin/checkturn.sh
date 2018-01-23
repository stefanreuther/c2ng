#!/bin/sh
#
#  PlanetsCentral - Check Turn
#
#  First parameter is game directory, "games/$gid"
#  Second parameter is player number
#
#  Exit status is -- add 1 to get database status
#    0 - ok
#    1 - yellow
#    2 - red
#    3 - damaged
#    4 - stale
#   10 - problem
#

if test -z "$1" || test -z "$2"; then
  echo "usage: $0 <gamedir> <player>"
  exit 3
fi
gamedir="$1"
player="$2"


# Read Configuration
. "$gamedir/c2host.ini"

# Check configuration
if test -x "$game_host_path/$game_host_program"; then
  :
else
  echo "Error: host program '$game_host_path/$game_host_program' does not exist."
  exit 10
fi

# Copy turn file from transfer into game directory
cp "$gamedir/in/new/player$player.trn" "$gamedir/data/player$player.trn" || exit 10

# Run host to check status
"$game_host_path/$game_host_program" -c$player "$gamedir/data" "$game_host_path"
code=$?

# Remove turn again
rm -f "$gamedir/data/player$player.trn"

# Evaluate status
if test $code = 0; then
  # ok
  mv "$gamedir/in/new/player$player.trn" "$gamedir/in/player$player.trn"
  exit 0
elif test $code = 2; then
  # stale
  rm -f "$gamedir/in/new/player$player.trn"
  exit 4
elif test $code = 64; then
  # yellow
  mv "$gamedir/in/new/player$player.trn" "$gamedir/in/player$player.trn"
  exit 1
elif test $code -ge 128; then
  # red
  rm -f "$gamedir/in/new/player$player.trn"
  exit 2
else
  # damaged
  rm -f "$gamedir/in/new/player$player.trn"
  exit 3
fi
