#!/bin/sh
#
#  PlanetsCentral - Run Host
#
#  First parameter is game directory, "games/$gid"
#  Second parameter is turn number (min 3 digits)
#
#  Exit status:
#    1 - host preparation or run failed
#    2 - host postprocessing failed
#    3 - general error
#

if test -z "$1" || test -z "$2"; then
  echo "usage: $0 <gamedir> <turn>"
  exit 3
fi
gamedir="$1"
turn="$2"
players="1 2 3 4 5 6 7 8 9 10 11"

# Log file
logfile="$gamedir/runhost.log"
echo "Log file for '$0 $gamedir $turn' on `date`" >"$logfile"

# Read Configuration
. "$gamedir/c2host.ini"
spec_mandatory="beamspec.dat engspec.dat hullspec.dat pconfig.src planet.nm race.nm torpspec.dat truehull.dat"
spec_optional="amaster.src pmaster.cfg shiplist.txt hullfunc.txt xtrfcode.txt xyplan.dat mission.ini map.ini"

# Check configuration
if test -x "$game_host_path/$game_host_program"; then
  :
else
  echo "Error: host program '$game_host_path/$game_host_program' does not exist."
  exit 1
fi

# Locate add-ons
echo "Updating auxhost scripts..." >>$logfile
toolpaths=""
if test -n "$game_tools"; then
  for i in $game_tools; do
    eval "toolpaths=\"\$toolpaths \$game_tool_${i}_path\""
  done
fi

# Generate auxhost files
rm -f $gamedir/data/auxhost1.ini $gamedir/data/auxhost2.ini $gamedir/data/auxbc.ini
for src in $toolpaths; do
  s="$src/c2aux1.sh"
  if test -r "$s"; then
    echo "  Using $s..." >> "$logfile"
    echo "/bin/sh $s $gamedir/data $src" >> "$gamedir/data/auxhost1.ini" || exit 1
  fi
  s="$src/c2aux2.sh"
  if test -r "$s"; then
    echo "  Using $s..." >> "$logfile"
    echo "/bin/sh $s $gamedir/data $src" >> "$gamedir/data/auxhost2.ini" || exit 1
  fi
  s="$src/c2auxbc.sh"
  if test -r "$s"; then
    echo "  Using $s..." >> "$logfile"
    echo "/bin/sh $s $gamedir/data $src" >> "$gamedir/data/auxbc.ini" || exit 1
  fi
done

# Generate player files
misfrags=""
mapfrags=""
echo "Updating player config files..." >>$logfile
for src in $game_sl_path $game_master_path $game_host_path defaults $toolpaths; do
  s="$src/mission.ini"
  if test -r "$s"; then
    echo "  Using $s for mission.ini..." >>$logfile
    misfrags="$misfrags $s"
  fi
  s="$src/map.ini.frag"
  if test -r "$s"; then
    echo "  Using $s for map.ini..." >>$logfile
    mapfrags="$mapfrags --file=$s"
  fi
done

mis="$gamedir/data/mission.ini"
if test -z "$misfrags"; then
  echo "  $mis is empty." >>$logfile
  rm -f "$mis"
else
  for i in $misfrags; do
    cat "$i"
    echo
  done > "$mis"
fi

map="$gamedir/data/map.ini"
if test -z "$mapfrags"; then
  echo "  $map is empty." >>$logfile
  rm -f "$map"
else
  perl bin/updateconfig.pl "$map" $mapfrags || exit 1
fi

# Delete signature files
rm -f $gamedir/data/c2score.txt $gamedir/data/c2ref.txt

# Pre-host backup
echo "Pre-host backup..." >>$logfile
( cd "$gamedir/data" && tar czf "../backup/pre-$turn.tgz" . ) || exit 1
( cd "$gamedir/in"   && tar czf "../backup/trn-$turn.tgz" . ) || exit 1

# Copy turn files into game directory
# (delete any that may be left in the game directory by accident)
echo "Copying turns..." >>$logfile
rm -f "$gamedir/data"/*.trn
for i in $players; do
  trn="$gamedir/in/player$i.trn"
  if test -r "$trn"; then
    echo "  $trn..." >>$logfile
    cp "$trn" "$gamedir/data/player$i.trn" || exit 1
  fi
done

# Pre-host
if test "$game_settings_hostHasRun" = "1"; then
  echo "Not running pre-host scripts because hostHasRun is set." >>$logfile
else
  echo "Running pre-host scripts..." >>$logfile
  for src in $toolpaths; do
    s="$src/c2pre.sh"
    if test -r "$s"; then
      echo "  $s..." >>$logfile
      set -- "$gamedir/data" "$src"
      . "$s" >>$logfile || exit 1
    fi
  done
fi

# Run host
if test "$game_settings_hostHasRun" = "1"; then
  echo "Not running host because hostHasRun is set." >>$logfile
else
  echo "Running host..." >>$logfile
  "$game_host_path/$game_host_program" $game_host_options "$gamedir/data" "$game_host_path" || exit 1
fi

# Post-host
echo "Running post-host scripts..." >>$logfile
for src in $toolpaths; do
  s="$src/c2post.sh"
  if test -r "$s"; then
    echo "  $s..." >>$logfile
    set -- "$gamedir/data" "$src"
    . "$s" >>$logfile || exit 1
  fi
done

# Post-host backup
echo "Post-host backup..." >>$logfile
( cd "$gamedir/data" && tar czf "../backup/post-$turn.tgz" . ) || exit 1

# Remove old transfer data
# (using "*" means that .c2file stays)
echo "Erasing old transfer data..." >>$logfile
for i in $players all; do
  rm -f "$gamedir/out/$i"/*
done
rm -f "$gamedir/in"/*

# Create new outgoing packages
echo "Creating player packages..." >>$logfile
for i in $players; do
  files=""
  doit=false
  for f in player$i.rst util$i.dat xyplan$i.dat flak$i.dat; do
    if test -r "$gamedir/data/$f"; then
      files="$files $f"
      doit=true
      cp "$gamedir/data/$f" "$gamedir/out/$i/$f" || exit 2
    fi
  done
  if $doit; then
    echo "  player$i.zip will be created with$files" >>$logfile
    ( cd "$gamedir/out/$i" && zip "player$i.zip" $files ) || exit 2
  else
    echo "  player$i.zip will not be created" >>$logfile
  fi
done

# Create playerfiles package
echo "Creating player files package..." >>$logfile
files=""
for i in $spec_mandatory $spec_optional; do
  if test -r "$gamedir/data/$i"; then
    use=true
    case $i in
      xyplan.dat)
        if test -n "$game_tool_explmap"; then use=false; fi
        ;;
    esac
    if $use; then
      files="$files $i"
      case "$i" in
        *.src|*.ini|*.txt|explmap.cfg|pmaster.cfg)
           echo "  Using $i (text)" >>$logfile
           perl -pe 's/\r*$/\r/' <"$gamedir/data/$i" >"$gamedir/out/all/$i" || exit 2
           ;;
        *)
           echo "  Using $i" >>$logfile
           cp "$gamedir/data/$i" "$gamedir/out/all/$i" || exit 2
           ;;
      esac
    fi
  fi
done
( cd "$gamedir/out/all" && zip playerfiles.zip $files ) || exit 2

# Done.
exit 0
