ResHack Resource Editor
=======================

This directory contains c2reshack, a resource editor.
It can edit pictures and fonts in PCC2's custom formats.

It is aimed at developers, not end-users.


Main Menu
---------

The main menu displays a list of open files.

* "Edit" opens the file-specific editor.
* "Load" loads a new file and adds it to the list.
* "Save" shows the file-specific dialog to save a file.
* "New" allows creating a new file.


Picture Editor
--------------

Left mouse button draws in foreground color, right mouse button draws
in background color. Function keys pick a tool:

* F1 - Pencil tool (free drawing, thin lines)
* F2 - Brush tool (free drawing, thick lines)
* F3 - Line tool (straight lines)
* F4 - Rectangle tool
* F5 - Solid bar tool
* F6 - Circle tool
* F7 - Copy tool
* F8 - Paste tool (place one copy of internal clipboard)
* F9 - Stamp tool (repeatedly place copies)
* F10 - Pipette tool (pick up a color)

Keys 0-3 set zoom level.

Pick foreground/background color by clicking with left/right mouse
button on color selector on the right. Use [<] and [>] buttons to
browse through colors. Some colors show a "Pal" option to modify the
palette.

The "Modify" button allows changing the size of the picture, and
adding/removing auxiliary lines.

When loading, the following file formats are supported:

* "*.cc" - 4-bpp custom picture file format with custom palette.
* "*.cd" - 8-bpp custom picture file format with custom palette.
* "*.gfx" - 8-bpp custom picture file format without palette.
* "*.bmp" - standard bitmap picture file format.
* "*.png" etc. - standard picture formats.

Pictures that do not use our custom palette will be converted. For
saving, only the first four formats are supported.

Unlike the PCC2 version of this program, this version does not try to
support and preserve pictures with foreign palette or truecolor
pictures. There are better programs for editing those.


Font Editor
-----------

Mouse buttons, function keys and digits work identical to Picture
Editor. Colors are limited to black, white, and half-intensity.

Additional keys:

* PgUp/PgDn - browse through characters
* Ctrl+PgUp/Ctrl+PgDn - browse, only stopping at defined characters
* +/- - change width of character
* Del - delete character
* A - attempt to synthesize character. This feature can synthesize
  "LETTER X WITH ACCENT" from "LETTER X" and "COMBINING ACCENT", and
  can synthesize various box drawing characters from "FULL BLOCK".
* C, Shift-C - copy from character. Select character from list of
  names (without shift), or glyphs (with shift).
* E - show font's coverage of character sets
* G, Shift-G - go to character.
* M - change font alignment by inserting/removing blank lines at top
  and bottom
* O, Shift-O - overlay character.
* P - preview this font.
* R - change font encoding; e.g., if this font contains characters for
  a cyrillic codepage at positions 128..255, moves those to their
  place in the Unicode encoding.
* S - show current character in system fonts.
* U - undo last editing.

On loading, the following font file formats are supported:

* "*.fnt" - custom bitmap font file format.
* "*.bdf" - a standard bitmap font file format.

When saving, only *.fnt can be created. c2reshack will ask for a font
encoding. PCC2's fonts use Unicode encoding. PCC1 supports "System"
and "Cyrillic" encoding with 256 characters only.


Unicode Support
---------------

The font editor uses the Unicode encoding. It can utilize auxiliary
files to show character names.

Command-line option "--names=NamesList.txt" will load character names.
The NamesList.txt file is part of Unicode. The names will then be
shown in various places, in particular, on the character name list (C,
G, O keys). The name list will also enable synthesis of characters by
name ("LETTER X WITH ACCENT" from "LETTER X" and "COMBINING ACCENT").

Command-line option "--alias=alias.txt" will load an additional alias
file for character synthesis. An alias.txt file has been provided with
the source code. This will allow synthesis of characters from
equally-looking characters, e.g. "CYRILLIC LETTER A" from "LATIN
LETTER A".
