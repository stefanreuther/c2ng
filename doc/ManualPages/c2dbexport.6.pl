start('c2dbexport', 'PCC2 Database Export');

section('Synopsis',
        paragraph_synopsis('<b>c2dbexport --help</b>'),
        paragraph_synopsis('<b>c2dbexport</b> [<b>--config=</b><i>INFO</i>] [<b>-D</b><i>KEY</i><b>=</b><i>VALUE</i>...] <i>COMMAND</i> [<i>ARGS</i>...]'));

section('Description',
        paragraph_text('<i>c2dbexport</i> is part of the PlanetsCentral suite.',
                       'It can create dumps of (ports of) PlanetsCentral data store.'),
        paragraph_text('<i>c2dbexport</i> writes a script for <link>c2console(6)</link> to standard output.',
                       'You can redirect that into a <fn>*.con</fn> file, and later execute it to restore the respective status.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_default_server_config(';'),
        paragraph_default_log());

section('Commands',
        paragraph_detail('<b>db</b> [<b>--delete</b>] <i>WILDCARD</i>...',
                         paragraph_text('Export database keys matching the given wildcards.',
                                        'The resulting script will add these keys to the target database instance.',
                                        'With <b>--delete</b>, the script will start with a command to delete all keys matching the given wildcards,',
                                        'allowing import into a nonempty database.')));

section('Environment',
        paragraph_default_environment(';'),
        paragraph_default_server_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)', 'c2console(6)'));
