start('pcc2', 'Planets Command Center II (c2ng)');

section('Synopsis',
        paragraph_text('<i>Planets Command Center II</i> (PCC2) is a game client for VGA Planets&trade; 3.x.'),
        paragraph_text('VGA Planets is a play-by-email space-combat strategy game for DOS, written by Tim Wisseman.',
                       'PCC 1.x is a closed-source client for DOS since 1995.',
                       'PCC2 is an open-source version for 32-bit operating systems in work since 2001.'),
        paragraph_text('PCC2 Next Generation (c2ng) is the next step in the evolution of PCC2.',
                       'It is intended to be 100% compatible to PCC2.',
                       'However, it does away with many self-imposed limitations of PCC2,',
                       'and has evolved into an entire suite/stack of programs.'),
        paragraph_text('In addition to the main client program <i>c2ng</i>, a graphical application,',
                       'this package includes a number of additional command-line utilities whose names usually start with <i>c2</i>.',
                       'Part of these are the servers (microservices) that drive the hosting site, <link>https://planetscentral.com/</link>.'));

section('Description',
        paragraph_text('Each utility is described on its own manual page.'));

subsect('Graphical Player Utilities',
        paragraph_detail('<link>c2ng(6)</link>',             paragraph_text('Graphical player client;')),
        paragraph_detail('<link>c2pluginw(6)</link>',        paragraph_text('Graphical plugin installer.')),
    );

subsect('Player Command-Line Utilities',
        paragraph_detail('<link>c2check(6)</link>',          paragraph_text('Turn file checker;')),
        paragraph_detail('<link>c2export(6)</link>',         paragraph_text('Export game data in various formats;')),
        paragraph_detail('<link>c2mgrep(6)</link>',          paragraph_text('Search for messages in various game files;')),
        paragraph_detail('<link>c2mkturn(6)</link>',         paragraph_text('Convert unpacked game data into a turn file to send to host;')),
        paragraph_detail('<link>c2plugin(6)</link>',         paragraph_text('Plugin installer/manager;')),
        paragraph_detail('<link>c2script(6)</link>',         paragraph_text('Command-line script interpreter;')),
        paragraph_detail('<link>c2simtool(6)</link>',        paragraph_text('Command-line battle simulator;')),
        paragraph_detail('<link>c2sweep(6)</link>',          paragraph_text('Game directory cleanup utility;')),
        paragraph_detail('<link>c2unpack(6)</link>',         paragraph_text('Unpack result files.'))
    );

subsect('Developer Utilities',
        paragraph_detail('<link>c2compiler(6)</link>',       paragraph_text('Compiler for PCC2 scripts (<fn>*.q</fn>);')),
        paragraph_detail('<link>c2gfxcodec(6)</link>',       paragraph_text('PCC2 graphics codecs;')),
        paragraph_detail('<link>c2gfxgen(6)</link>',         paragraph_text('Procedural generation of graphics;')),
        paragraph_detail('<link>c2restool(6)</link>',        paragraph_text('Manipulate resource files;')),
        paragraph_detail('<link>c2untrn(6)</link>',          paragraph_text('Decode and manipulate turn files.'))
    );

subsect('Host/Server Utilities',
        paragraph_detail('<link>c2configtool(6)</link>',     paragraph_text('Manipulate and query configuration files;')),
        paragraph_detail('<link>c2console(6)</link>',        paragraph_text('Command-line interface to microservices;')),
        paragraph_detail('<link>c2dbexport(6)</link>',       paragraph_text('Export slices of the database;')),
        paragraph_detail('<link>c2docmanager(6)</link>',     paragraph_text('Manage documentation data sets for <link>c2doc-server(6)</link>;')),
        paragraph_detail('<link>c2doc-server(6)</link>',     paragraph_text('Server publishing a set of documentation files;')),
        paragraph_detail('<link>c2fileclient(6)</link>',     paragraph_text('Client for <link>c2file-server(6)</link>;')),
        paragraph_detail('<link>c2file-server(6)</link>',    paragraph_text('Server publishing a file store;')),
        paragraph_detail('<link>c2format-server(6)</link>',  paragraph_text('Server providing parsing of binary files;')),
        paragraph_detail('<link>c2host-server(6)</link>',    paragraph_text('Server providing game management and host scheduler;')),
        paragraph_detail('<link>c2logger(6)</link>',         paragraph_text('Wrapper for servers to provide logging and lifecycle;')),
        paragraph_detail('<link>c2mailin(6)</link>',         paragraph_text('Incoming mail processor for host;')),
        paragraph_detail('<link>c2mailout-server(6)</link>', paragraph_text('Server providing a queue for outgoing mail;')),
        paragraph_detail('<link>c2monitor-server(6)</link>', paragraph_text('Server that monitors other servers, and publishes the result;')),
        paragraph_detail('<link>c2nntp-server(6)</link>',    paragraph_text('Server publishing forums via NNTP;')),
        paragraph_detail('<link>c2play-server(6)</link>',    paragraph_text('Server providing machine-access to game data for playing;')),
        paragraph_detail('<link>c2rater(6)</link>',          paragraph_text('Game difficulty rating;')),
        paragraph_detail('<link>c2router-server(6)</link>',  paragraph_text('Server providing access to multiple <link>c2play-server</link> instances;')),
        paragraph_detail('<link>c2talk-server(6)</link>',    paragraph_text('Server providing forums and private messages;')),
        paragraph_detail('<link>c2user-server(6)</link>',    paragraph_text('Server providing user management.'))
    );

section('Documentation',
        paragraph_text('In addition to their own manual page,',
                       'all utilities accept a <b>-h</b> or <b>--help</b> option.',
                       '<link>c2ng(6)</link> also comes with built-in hypertext help.'),
        paragraph_text('All documentation is also available in the PlanetsCentral Documentation Center at <link>https://planetscentral.com/doc.cgi/</link>.'));

section('Localisation');
subsect('Unix/Linux',
        paragraph_text('Under Unix/Linux, PCC2 and all utilities evaluate environment variables and adapt accordingly.',
                       'This behaviour matches that of the C library and most programs,',
                       "so you usually don't have to change anything for PCC2 to work the way it should."),
        paragraph_detail('<b>LC_MESSAGES</b>',
                         paragraph_text('This environment variable determines the language to use.',
                                        'It must contain a two-character country code (optionally followed by additional qualifiers).',
                                        'When an appropriate language file (e.g., <fn>de.lang</fn>) exists in the resource directory,',
                                        'it is used to produce messages.',
                                        'Otherwise, messages will be English;')),
        paragraph_detail('<b>LC_CTYPE</b>',
                         paragraph_text('This environment variable determines the character encoding to use.',
                                        'It is evaluated by the C library, which determines the exact rules.',
                                        'For example, on my system, "de_DE" produces Latin-1, "de_DE.utf8" produces UTF-8.',
                                        'All input, output, and file names will be produced in this encoding.',
                                        'In particular, everything written to standard output by command-line tools will have this encoding;')),
        paragraph_detail('<b>LC_ALL</b>',
                         paragraph_text('This environment variable, if set, overrides <b>LC_MESSAGES</b> and <b>LC_CTYPE</b>;')),
        paragraph_detail('<b>LANG</b>',
                         paragraph_text('This variable provides a default if none of the LC_xxx variables is set.',
                                        'If this variable is not set, either, PCC2 will use English.')));
subsect('Windows',
        paragraph_text('PCC2 uses the language reported by Windows.',
                       'When an appropriate language file (e.g., <fn>de.lang</fn>) exists in the resource directory,',
                       'it is used to produce messages.',
                       'Otherwise, messages will be English.'),
        paragraph_text('Console output will use your console codepage ("OEM").',
                       'When output is redirected, output will use UTF-8.',
                       'This has the advantage of working with most current editors and the Cygwin console.',
                       'However, "c2untrn ... | more" on a Windows prompt will produce garbage.',
                       'There is no reliable way to satisfy both usecases.'));

subsect('Game Character Set',
        paragraph_text('Utilities that deal with game files support a <i>game character set</i>.',
                       'Planets is often played in DOS environments,',
                       'and programs like PHost produce in-game messages in DOS encodings.',
                       'By specifying a game character set, the utilities can decode those in-game encodings to display them properly.'),
        paragraph_text('Usually, programs take an option <b>--charset=</b><i>CHARSET</i> or <b>-C</b> <i>CHARSET</i>,',
                       'where <i>CHARSET</i> is the character set name.',
                       'The following character sets are supported:',
                       'cp1250,',
                       'cp1251,',
                       'cp1252,',
                       'cp437,',
                       'cp850,',
                       'cp852,',
                       'cp866,',
                       'koi8r,',
                       'latin1,',
                       'latin2.'));

section('Files',
        paragraph_text('Under Unix, PCC2 utilities store their configuration in a directory <fn>.pcc2</fn>',
                       'within your home directory ($HOME).'),
        paragraph_text('Under Windows, the configuration is stored in a directory <fn>PCC2</fn> within your',
                       "profile's <i>Application Data</i> folder, e.g. \"C:\\Users\\You\\AppData\\Roaming\\PCC2\"."),
        paragraph_text('The configuration can consist of multiple files:'),
        paragraph_detail('<fn>pcc2.ini</fn>',          paragraph_text('User settings file;')),
        paragraph_detail('<fn>expr.ini</fn>',          paragraph_text('Predefined expressions for drop-down lists;')),
        paragraph_detail('<fn>lru.ini</fn>',           paragraph_text('Previous expressions used in drop-down lists;')),
        paragraph_detail('<fn>network.ini</fn>',       paragraph_text('Configuration data for network games;')),
        paragraph_detail('<fn>pccinit.q</fn>',         paragraph_text('User initialisation script;')),
        paragraph_detail('<fn>cc-res.cfg</fn>',        paragraph_text('User resource configuration;')),
        paragraph_detail('<fn>games/</fn>',            paragraph_text('Automatically managed game directories;')),
        paragraph_detail('<fn>plugins/</fn>',          paragraph_text('Plugins (see <link>c2plugin(6)</link>). One <fn>*.c2p</fn> file and one directory per installed plugin.'))
    );

section('Author',
        paragraph_text('Most of the code has been written by Stefan Reuther &lt;Streu@gmx.de&gt; with a little help',
                       "from people all over the 'net.",
                       'See the <fn>AUTHORS.txt</fn> file included with the source code package for details.'));

section('License',
        paragraph_text('<i>Planets Command Center II</i> is distributed under a permissive BSD-like open-source license.',
                       'See the <fn>COPYING.txt</fn> file included with the source code package for details.'),
        paragraph_text('You are invited to compile and modify PCC2 yourself if you need.',
                       'I would be thankful if you send me your modifications for inclusion in my "official" version.'));
