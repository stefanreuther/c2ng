#!/bin/sh
#
#  PlanetsCentral - Run Master
#
#  First parameter is game directory, "games/$gid"
#
#  Exit status:
#    1 - master preparation or run failed
#    2 - master postprocessing failed
#    3 - general error
#

if test -z "$1"; then
  echo "usage: $0 <gamedir>"
  exit 3
fi
gamedir="$1"

# Does directory exist?
for i in "" "/in" "/out" "/data" "/backup"; do
  if test -d "$gamedir$i"; then
    :
  else
    echo "Error: '$gamedir$i' does not exist"
    exit 1
  fi
done

# Log file
logfile="$gamedir/runmaster.log"
echo "Log file for '$0 $gamedir' on `date`" >"$logfile"

# Read Configuration
. "$gamedir/c2host.ini"
spec_mandatory="beamspec.dat engspec.dat hullspec.dat pconfig.src planet.nm race.nm storm.nm torpspec.dat truehull.dat"
spec_optional="amaster.src pmaster.cfg shiplist.txt hullfunc.txt xyplan.dat"

# Check configuration
if test -x "$game_master_path/$game_master_program"; then
  :
else
  echo "Error: master program '$game_master_path/$game_master_program' does not exist."
  exit 1
fi

# Find tools
toolpaths=""
if test -n "$game_tools"; then
  for i in $game_tools; do
    eval "toolpaths=\"\$toolpaths \$game_tool_${i}_path\""
  done
fi

# Install all game files. Priority order is:
# - game
# - shiplist
# - master
# - host
# - default
# Mandatory files first:
echo "Copying spec files..." >>$logfile
for f in $spec_mandatory; do
  if test -e "$gamedir/data/$f"; then
    : # ok
  else
    ok=false
    for src in $toolpaths "$game_sl_path" "$game_master_path" "$game_host_path" "defaults"; do
      if cp "$src/$f" "$gamedir/data/$f" 2>/dev/null; then
        ok=true
        echo "  Using $src/$f" >>"$logfile"
        break
      fi
    done
    if $ok; then
      : # ok
    else
      echo "Error: unable to find file '$f'"
      exit 1
    fi
  fi
done
for f in $spec_optional; do
  if test -e "$gamedir/data/$f"; then
    : # ok
  else
    for src in $toolpaths "$game_sl_path" "$game_master_path" "$game_host_path" "defaults"; do
      if cp "$src/$f" "$gamedir/data/$f" 2>/dev/null; then
        echo "  Using $src/$f" >>"$logfile"
        break
      fi
    done
  fi
done

# Install all configuration fragments
echo "Updating configuration..." >>$logfile

# Update configuration files
for file in pconfig.src shiplist.txt amaster.src pmaster.cfg; do
  if test -r "$gamedir/data/$file"; then
    # Priority order is tools, shiplist, master, host, default, so we must build options
    # for updateconfig in reverse order!
    # FIXME: this comment does not match the code; think about it.
    frags=""
    for src in "$gamedir/data" $game_sl_path $game_master_path $game_host_path defaults $toolpaths; do
      f="$src/$file.frag"
      if test -r "$f"; then
        echo "  Using $f for $file" >>"$logfile"
        frags="--file=$f $frags"
      fi
    done

    # Update file
    case $file in
      pconfig.src)
        # Update PHost configuration regularily, but include game name
        perl bin/updateconfig.pl "$gamedir/data/$file" $frags "PHOST.GameName=$game_name" || exit 1
        ;;
      amaster.src)
        # Some options are mirrored from pconfig.src into amaster.src
        tmpfrag="$gamedir/tmpamaster.frag"
        (
          echo '%AMASTER'
          if test -f "$gamedir/data/pconfig.src"; then
            grep -wiE 'PlayerRace|MapTruehullByPlayerRace|AllowWraparoundMap|WraparoundRectangle|CrystalsPreferDeserts|CrystalSinTempBehavior|ClimateLimitsPopulation|MaxColTempSlope' <"$gamedir/data/pconfig.src"
          fi
        ) > "$tmpfrag"
        nlines=$(wc -l <"$tmpfrag")
        echo "  Updating $file with" $nlines "lines from host configuration" >>"$logfile"
        perl bin/updateconfig.pl "$gamedir/data/$file" $frags --file="$tmpfrag" "AMASTER.RaceIsPlaying=$game_slots" || exit 1
        rm -f "$tmpfrag"
        ;;
      pmaster.cfg)
        # Some options are mirrored from pconfig.src into pmaster.cfg
        tmpfrag="$gamedir/tmppmaster.frag"
        (
          echo '%PMASTER'
          if test -f "$gamedir/data/pconfig.src"; then
            perl -ne 'print if s/CrystalsPreferDeserts/DesertCrystalHomeworld/i' <"$gamedir/data/pconfig.src"
          fi
        ) > "$tmpfrag"
        nlines=$(wc -l <"$tmpfrag")
        echo "  Updating $file with" $nlines "lines from host configuration" >>"$logfile"
        perl bin/updateconfig.pl "$gamedir/data/$file" $frags --file="$tmpfrag" "PMASTER.RaceIsPlaying=$game_slots" || exit 1
        rm -f "$tmpfrag"
        ;;
      *)
        # Standard behaviour
        perl bin/updateconfig.pl "$gamedir/data/$file" $frags || exit 1
        ;;
    esac
  fi
done

# Initialize tools
for src in $toolpaths; do
  f="$src/c2master.sh"
  if test -r "$f"; then
    echo "  Executing $f..." >>"$logfile"
    set -- "$gamedir/data" "$src"
    . "$f" >>$logfile || exit 1
  fi
done

# Pre-master backup
echo "Pre-Master backup..." >>$logfile
( cd "$gamedir/data" && tar czf "../backup/premaster.tgz" . ) || exit 1

# Prepare
if test "$game_host_kind" = "phost4"; then
  PHOST_VERSION=4
else
  PHOST_VERSION=3
fi
export PHOST_VERSION

# Run master
if test "$game_settings_masterHasRun" = "1"; then
  echo "Not running master because masterHasRun is set." >>$logfile
else
  echo "Running master ($game_master_path/$game_master_program)..." >>$logfile
  "$game_master_path/$game_master_program" $game_master_options "$gamedir/data" "$game_master_path" || exit 1
fi

# No post-master backup; that would be the the same as the next pre-host backup
exit 0
