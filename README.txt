
                       Planets Command Center II
                      ---------------------------
                         Next Generation (c2ng)


WHAT IS PCC2 (c2ng)?
=====================

  Planets Command Center (PCC) is a game client for VGA Planets 3.

  VGA Planets is a play-by-email space-combat strategy game for DOS,
  written by Tim Wisseman. PCC 1.x is a closed-source DOS program
  since 1995. PCC2 is an open-source version for 32-bit operating
  systems in work since 2001.

  PCC2 Next Generation (c2ng) is the next step in the evolution of
  PCC2. It is intended to be 100% compatible to PCC2. However, it does
  away with many self-imposed limitations of PCC2.


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

  Basically, this is the VGA Planets client for the 21st century.


Status
=======

  This is a complete, playable Planets client at a feature level
  comparable to PCC1/PCC2. It is pretty solid and mature and can be
  used by everyone.

  The user interface is still rough at times; I hope to clean that up
  in the future. In particular, it flickers a bit more than I like to
  admit.


User Features
--------------

  Browsing and loading local and network games ................. done
    Browse your hard disk as well as network.
    Support servers that do not have VGAP binary formats (Nu).

  Manage multiple turns ........................................ done
    Browse history while playing.

  Multithreaded combat simulation .............................. done
    We can use multiple threads for simulation and thus (again)
    offer the fastest and most feature-rich combat simulation ever.

  Lots of utilities ............................................ done
    All the PCC2 command-line utilities, plus some more, for all your
    scripting needs, e.g. c2unpack, c2check, c2export, etc.

  SDL2 ......................................................... done
    The SDL2 graphics library is more up-to-date than SDL1 that PCC2
    is using. In particular, it allows painless full-screen mode on
    current computers.

  All the features of PCC1/PCC2 .......................... 99.9% done
    Scripting. Battle simulator. History. Zoomable map. And so on.
    New features nowadays often get born in PCC2ng.


Technical Features
-------------------

  (Not directly relevant for players, but I like them.)

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
    PlanetsCentral now runs exclusively on servers provided by c2ng,
    in better quality than before. c2ng serves as an architectural
    blueprint for PCC2 Web.


Still Missing
--------------

  Installer (for Windows), Package (for Linux).

  Full network play, in particular, for Nu.

  Standalone simulator, maybe?


Installation Instructions
=========================

  As of 2.41, the Windows version is distributed in a *.zip file. To
  install, just unzip into an empty directory (e.g. C:\Planets\PCC2).

  Install an artwork plugin from <http://phost.de/~stefan/plugins/> by
  downloading the *.c2z file, and dropping it on c2pluginw.exe.

  Linux users need to install from source; see doc/HackingGuide.txt.


How to use it
=============

  The main program is c2ng(.exe); it replaces the former pcc-v2(.exe).

  When you start the program, you will be shown a browser starting
  with "My Computer".

  (a) To play a local game, use the browser to browse to a game
  directory on your hard disk.

  (b) To play a network game, press [Ins] to add a network account.
  c2ng will ask for
  - user name
  - server type (PlanetsCentral or planets.nu)
  - server address (you can leave this blank)
  A new entry will appear representing that account. You can enter it
  like a normal folder to browse your games; you will be asked for
  your password the first time you do that in a session. Also, c2ng
  will ask you for a game directory where to place local files. If you
  choose the default, c2ng will store the files in your user profile.

  Use arrow keys and [Enter] to browse. After entering a game, you
  will see the regular PCC2 main screen with F1/F2/F3 buttons and all
  the usual playing options. Play normally. When you exit, a turn file
  will be written.

  If you have configured PCC2 to make backups of your result files,
  [Alt]+[Up] will show the previous turn. (If you have not configured
  PCC2 to make backups, do so now.)


Program List
-------------

  - Main Utilities
    . c2ng: graphical player client

  - Utilities
    . c2check: turn checker
    . c2compiler: script compiler
    . c2configtool: configuration handler
    . c2export: game data export
    . c2gfxcodec: graphics codecs
    . c2gfxgen: procedural graphics generation
    . c2mgrep: message search
    . c2mkturn: maketurn utility
    . c2plugin: plugin manager
    . c2pluginw: plugin manager (simple GUI)
    . c2rater: game rating computer
    . c2restool: resource file manager
    . c2script: scripting engine
    . c2simtool: battle simulator command-line tool
    . c2sweep: game directory cleaner
    . c2unpack: unpack utility
    . c2untrn: turn file decompiler

  - PlanetsCentral
    . c2console: console
    . c2dbexport: database exporter
    . c2docmanager: manage documentation for c2doc-server
    . c2doc-server: documentation server
    . c2fileclient: file client
    . c2file-server: file server
    . c2format-server: format (binary I/O) server
    . c2host-server: host server
    . c2logger: server control and logging utility
    . c2mailin: incoming mail processor
    . c2mailout-server: mail queue server
    . c2monitor-server: server status monitor
    . c2nntp-server: NNTP server
    . c2play-server: web-based play backend server
    . c2router-server: web-based play session manager
    . c2talk-server: PM/forum server
    . c2user-server: user manager


Future Milestones
------------------

  - complete the Nu integration
  - more server integration
  - fancier GUI and more graphics resolutions!
  - implement internationalisation and help
  - can we integrate forums/activities?
  - more operating systems (Android!)
  - C++11/17
  - OpenGL


Contributing
=============

  This package is licensed under a BSD license; see COPYING.txt. You
  can get sourcecode, build it yourself, and modify if you wish. See
  <doc/HackingGuide.txt> for instructions.

  Since 2.40.10 (May 2021), c2ng is maintained in a public git
  repository. You are welcome to submit bug reports, patches, or pull
  requests.

  Most PCC1/PCC2 code has now been ported so the code should now be
  fairly stable, structure-wise.


Contact
========

  Web site: <https://phost.de/~stefan/pcc2ng.html>

  Github: <https://github.com/stefanreuther/c2ng>

  PlanetsCentral: <https://planetscentral.com/>

  Email: <streu@gmx.de>
