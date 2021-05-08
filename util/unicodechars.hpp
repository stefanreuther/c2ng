/**
  *  \file util/unicodechars.h
  *  \brief Unicode Character Codes
  *
  *  This module contains character codes for Unicode characters used
  *  in the user interface. Those can be used instead of hard-coded
  *  escapes for easier readability. Note that they should not be used
  *  in strings intended for translation, because xgettext will not
  *  handle that.
  */
#ifndef C2NG_UTIL_UNICODECHARS_HPP
#define C2NG_UTIL_UNICODECHARS_HPP

#define UTF_UP_ARROW         "\xE2\x86\x91"       /* U+2191 */
#define UTF_DOWN_ARROW       "\xE2\x86\x93"       /* U+2193 */
#define UTF_RIGHT_ARROW      "\xE2\x86\x92"       /* U+2192 */
#define UTF_LEFT_ARROW       "\xE2\x86\x90"       /* U+2190 */
#define UTF_LEFT_RIGHT_ARROW "\xE2\x86\x94"       /* U+2194 */
#define UTF_TOP_DOWN_ARROW   "\xE2\x86\x95"       /* U+2195 */
#define UTF_PAGE_UP_ARROW    "\xE2\x86\x9F"       /* U+219F */
#define UTF_PAGE_DOWN_ARROW  "\xE2\x86\xA1"       /* U+21A1 */
#define UTF_TAB_ARROW        "\xEE\x85\x80"       /* U+E140 (private) */
#define UTF_LEFT_TRIANGLE    "\xE2\x97\x80"       /* U+25C0 */
#define UTF_RIGHT_TRIANGLE   "\xE2\x96\xB6"       /* U+25B6 */
#define UTF_RIGHT_POINTER    "\xE2\x96\xBA"       /* U+25BA */
#define UTF_THIN_BAR         "\xEE\x85\x84"       /* U+E144 (private) */
#define UTF_BULLET           "\xE2\x80\xA2"       /* U+2022, round bullet */
#define UTF_SQUARE_BULLET    "\xE2\x96\xA0"       /* U+25A0, black square */
#define UTF_HYPHEN           "\xE2\x80\x90"       /* U+2010 */
#define UTF_FIGURE_DASH      "\xE2\x80\x92"       /* U+2012 */
#define UTF_EN_DASH          "\xE2\x80\x93"       /* U+2013 */
#define UTF_EM_DASH          "\xE2\x80\x94"       /* U+2014 */
#define UTF_CORRESPONDS      "\xE2\x89\x99"       /* U+2259, =^ */
#define UTF_TIMES            "\xC3\x97"           /* U+00D7 */
#define UTF_GEQ              "\xE2\x89\xA5"       /* U+2265 */
#define UTF_LEQ              "\xE2\x89\xA4"       /* U+2264 */
#define UTF_REGISTERED       "\xC2\xAE"           /* U+00AE */
#define UTF_TRADEMARK        "\xE2\x84\xA2"       /* U+2122 */
#define UTF_ORNAMENT_LEFT    "\xEE\x85\x82"       /* U+E142 (private) */
#define UTF_ORNAMENT_RIGHT   "\xEE\x85\x83"       /* U+E143 (private) */
#define UTF_MIDDLE_DOT       "\xC2\xB7"           /* U+00B7 */
#define UTF_CHECK_MARK       "\xE2\x9C\x93"       /* U+2713 */
#define UTF_BALLOT_CROSS     "\xE2\x9C\x97"       /* U+2717 */
#define UTF_ALMOST_EQUAL     "\xE2\x89\x88"       /* U+2248, ~~ */
#define UTF_STOPWATCH        "\xE2\x8F\xB1"       /* U+23F1, used in place of a proper clock icon (U+1F550 is outside BMP) */

#endif
