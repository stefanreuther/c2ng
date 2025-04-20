start('c2rater', 'PCC2 Game Difficulty Rater');

section('Synopsis',
        paragraph_synopsis('<b>c2rater</b> [<b>-</b><i>OPTIONS</i>...] [<i>GAMEDIR</i>]'));

section('Description',
        paragraph_text('<i>c2rater</i> computes a difficulty rating for a game setup.',
                       'To do that, it reads the specification and host/master configuration files from the given game directory (default: current directory).'),
        paragraph_text('The difficulty rating is computed from four individual ratings',
                       '(ship list, minerals, natives, production).',
                       'For each rating, 100 represents "normal" difficulty; higher values are harder, lower values are easier.',
                       'By default, <i>c2rater</i> outputs a human-readable report containing all ratings.',
                       'With <b>--total</b> or <b>--value</b>, <i>c2rater</i> outputs only one value as an integer.'));

section('Options',
        paragraph_default_help(';'),
        paragraph_detail('<b>-D</b><i>SECTION</i><b>.</b><i>OPTION</i><b>=</b><i>VALUE</i>',
                         paragraph_text('Override a configuration option;')),
        paragraph_detail(text_option('--file=','FILE'),
                         paragraph_text('Override configuration options given by file;')),
        paragraph_detail(text_option('--total'),
                         paragraph_text('Report only total difficulty;')),
        paragraph_detail(text_option('--value=', 'WHICH'),
                         paragraph_text('Report only given rating (shiplist, minerals, natives, production).')));

section('Environment',
        paragraph_default_environment());

section('Author',
        paragraph_default_author());

section('See Also',
        paragraph_link_list('pcc2(6)'));
