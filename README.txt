
                       Planets Command Center II
                      ---------------------------
                         Next Generation (c2ng)


WHAT IS PCC2?
==============

  PCC2 is a re-write of the semi-popular VGA Planets 3 client PCC 1.x.

  VGA Planets is a play-by-email space-combat strategy game for DOS,
  written by Tim Wisseman. Planets Command Center (PCC) 1.x is a
  closed-source DOS program since 1995. PCC2 is an open-source version
  for 32-bit operating systems in work since 2001.

  PCC2 Next Generation (c2ng) is the next step in the evolution of
  PCC2. It is intended to be 100% compatible to PCC2. However, it does
  away with many self-imposed limitations of PCC2.

  Milestone Five "Let's Get Dangerous" adds the ability to save data -
  for now, for local games only. However, this means you now need to
  use it with care: DO NOT RUN THIS ON YOUR ORIGINAL GAME DATA. With
  this ability come ports of the Maketurn and Unpack utilities
  (c2mkturn, c2unpack).

  That aside, this version improves functionality a little pretty much
  everywhere, but adds no major breakthroughs.


Project Goals
--------------

  We do no longer expect to ever have a DOS version. Multithreading /
  multicore is no longer rocket science. We assume every operating
  system we ever run on has threads, and every library we use can deal
  with them. Computers are fast to allow us some internal abstractions
  that finally yield better reliability.

  This also allows us to implement features that cannot be done in the
  original 2001 PCC2 core:
  - network play. Talk to your host directly. No more messing with
    folders and zip files.
  - targeting a GUI library other than SDL 1.2. In particular, this
    means we can target SDL 2, allowing us to run on Android. Maybe we
    can even run on a direct OS interface with no library between.
  - history. The single most often requested feature.
  - more scriptability.


Milestone Three
================

  This version implements some key parts to prove feasibility. It does
  not have a fancy GUI and can not be used for playing.

  Browsing and loading local and network games ................. done
    Browse your hard disk as well as network.
    Support servers that do not have VGAP binary formats (Nu).
    Milestone four adds long-needed configuration/setup menus.

  Manage multiple turns ........................................ done
    Browse history while playing.

  Manage multiple games ........................................ done
    PCC2 used global variables. This one does not, so it can have
    multiple instances.

  Model/View/Presenter separation with multiple threads ........ done
    More fluent GUI. No more "Application not responding".

  Scripting engine ............................................. done
    Controls most of the GUI and data displays.

  Meaningful test coverage ..................................... done
    PCC2 has a meager test coverage of around 10%. c2ng currently
    achieves >50% in the application, ~90% in the foundation
    libraries. This, together with some newer coding styles, gives it
    a greater stability than PCC2, and some confidence that changing a
    feature does not break another.

  PlanetsCentral / PCC2Web ..................................... done
    Milestone Three adds all servers except for PCC2 Web, in better
    quality than before.

  SDL2 ......................................................... done
    Milestone four can be built using SDL2.


What can you do with it?
========================

  c2ng(.exe) is the future player client (pcc-v2).

  When you start the program, you will be shown a browser starting
  with "My Computer". Note that the GUI is still incomplete.

  (a) Use the browser to browse to a game directory on your hard disk.

  (b) Press [Ins] to add a network account. c2ng will ask for
  - user name
  - server type (PlanetsCentral or planets.nu)
  - server address (you can leave this blank)
  A new entry will appear representing that account. You can enter it
  like a normal folder to browse your games; you will be asked for
  your password the first time you do that in a session. Also, c2ng
  will ask you for a game directory where to place local files. If you
  choose the default, c2ng will store the files in your user profile.

  Use arrow keys and [Enter] to browse. After entering a game, you
  will see a screen with F1/F2/F3 buttons. Use these to select a ship,
  planet, or base, and view its control screen.

  If you have configured PCC2 to make backups of your result files,
  [Alt]+[Up] will show the previous turn. (If you have not configured
  PCC2 to make backups, do so now.)


Program List
-------------

  - Main Utilities
    . c2ng: graphical player client

  - Utilities
    . c2check: turn checker
    . c2configtool: configuration handler
    . c2export: game data export
    . c2gfxgen: procedural graphics generation
    . c2mgrep: message search
    . c2mkturn: maketurn utility
    . c2plugin: plugin manager
    . c2pluginw: plugin manager (simple GUI)
    . c2rater: game rating computer
    . c2script: scripting engine
    . c2sweep: game directory cleaner
    . c2unpack: unpack utility
    . c2untrn: turn file decompiler

  - PlanetsCentral
    . c2console: console
    . c2dbexport: database exporter
    . c2fileclient: file client
    . c2file-server: file server
    . c2format-server: format (binary I/O) server
    . c2host-server: host server
    . c2mailin: incoming mail processor
    . c2mailout-server: mail queue server
    . c2monitor-server: server status monitor
    . c2nntp-server: NNTP server
    . c2talk-server: PM/forum server


Near Milestones
----------------

  - model the complete data and make it available for scripting
    . most v3 objects done now
    . Nu advantages (map to racial abilities and configuration)
  - more server integration
    . saving for server-based games
  - fancier GUI
  - more graphics resolutions!


Future Milestones
------------------

  - implement i18n
  - implement help
  - can we integrate forums/activities?
  - more operating systems (Android!)
  - C++11, maybe
