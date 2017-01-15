/**
  *  \file ui/colorscheme.cpp
  */

#include "ui/colorscheme.hpp"
#include "gfx/context.hpp"

/** Standard Palette.
    This defines the standard colors used for the program. */
const gfx::ColorQuad_t ui::STANDARD_COLORS[] = {
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 0
    COLORQUAD_FROM_RGB( 97,  97,  97),    // 1
    COLORQUAD_FROM_RGB(194, 194, 194),    // 2
    COLORQUAD_FROM_RGB( 97, 242,  97),    // 3
    COLORQUAD_FROM_RGB(255,   0,   0),    // 4
    COLORQUAD_FROM_RGB( 64, 129,  64),    // 5
    COLORQUAD_FROM_RGB( 97,  97, 194),    // 6
    COLORQUAD_FROM_RGB(129, 129, 194),    // 7
    COLORQUAD_FROM_RGB( 97,  97, 129),    // 8
    COLORQUAD_FROM_RGB(255, 255,   0),    // 9
    COLORQUAD_FROM_RGB(121,  97, 121),    // 10       30 24 30
    COLORQUAD_FROM_RGB(129, 129, 129),    // 11       32 32 32
    COLORQUAD_FROM_RGB(178, 178, 137),    // 12       44 44 34
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 13
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 14
    COLORQUAD_FROM_RGB(255, 255, 255),    // 15
    COLORQUAD_FROM_RGB( 12,  12,  12),    // 16
    COLORQUAD_FROM_RGB( 28,  28,  28),    // 17
    COLORQUAD_FROM_RGB( 44,  44,  44),    // 18
    COLORQUAD_FROM_RGB( 60,  60,  60),    // 19
    COLORQUAD_FROM_RGB( 76,  76,  76),    // 20
    COLORQUAD_FROM_RGB( 93,  93,  93),    // 21
    COLORQUAD_FROM_RGB(109, 109, 109),    // 22
    COLORQUAD_FROM_RGB(125, 125, 125),    // 23
    COLORQUAD_FROM_RGB(141, 141, 141),    // 24
    COLORQUAD_FROM_RGB(157, 157, 157),    // 25
    COLORQUAD_FROM_RGB(174, 174, 174),    // 26
    COLORQUAD_FROM_RGB(190, 190, 190),    // 27
    COLORQUAD_FROM_RGB(206, 206, 206),    // 28
    COLORQUAD_FROM_RGB(222, 222, 222),    // 29
    COLORQUAD_FROM_RGB(238, 238, 238),    // 30
    COLORQUAD_FROM_RGB(255, 255, 255),    // 31
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 32
    COLORQUAD_FROM_RGB( 24,   8,   0),    // 33
    COLORQUAD_FROM_RGB( 48,  16,   0),    // 34
    COLORQUAD_FROM_RGB( 72,  24,   0),    // 35
    COLORQUAD_FROM_RGB( 97,  32,   0),    // 36
    COLORQUAD_FROM_RGB(121,  40,   0),    // 37
    COLORQUAD_FROM_RGB(145,  48,   0),    // 38
    COLORQUAD_FROM_RGB(170,  56,   0),    // 39
    COLORQUAD_FROM_RGB(194,  64,   0),    // 40
    COLORQUAD_FROM_RGB(218,  72,   0),    // 41
    COLORQUAD_FROM_RGB(242,  80,   0),    // 42
    COLORQUAD_FROM_RGB(255,  85,   0),    // 43
    COLORQUAD_FROM_RGB(255,  97,   0),    // 44
    COLORQUAD_FROM_RGB(255, 109,   0),    // 45
    COLORQUAD_FROM_RGB(255, 121,   0),    // 46
    COLORQUAD_FROM_RGB(255, 133,   0),    // 47
    COLORQUAD_FROM_RGB(255, 145,   0),    // 48
    COLORQUAD_FROM_RGB(255, 157,   0),    // 49
    COLORQUAD_FROM_RGB(255, 170,   0),    // 50
    COLORQUAD_FROM_RGB(255, 182,   0),    // 51
    COLORQUAD_FROM_RGB(255, 194,   0),    // 52
    COLORQUAD_FROM_RGB(255, 206,   0),    // 53
    COLORQUAD_FROM_RGB(255, 218,   0),    // 54
    COLORQUAD_FROM_RGB(255, 230,   0),    // 55
    COLORQUAD_FROM_RGB(255, 242,   0),    // 56
    COLORQUAD_FROM_RGB(255, 255,   0),    // 57
    COLORQUAD_FROM_RGB(255, 255,  32),    // 58
    COLORQUAD_FROM_RGB(255, 255,  72),    // 59
    COLORQUAD_FROM_RGB(255, 255, 113),    // 60
    COLORQUAD_FROM_RGB(255, 255, 153),    // 61
    COLORQUAD_FROM_RGB(255, 255, 194),    // 62
    COLORQUAD_FROM_RGB(255, 255, 234),    // 63
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 64
    COLORQUAD_FROM_RGB(  0,   0,  28),    // 65
    COLORQUAD_FROM_RGB(  0,   0,  56),    // 66
    COLORQUAD_FROM_RGB(  0,   0,  85),    // 67
    COLORQUAD_FROM_RGB( 32,  32, 121),    // 68
    COLORQUAD_FROM_RGB( 52,  52, 133),    // 69
    COLORQUAD_FROM_RGB( 72,  72, 145),    // 70
    COLORQUAD_FROM_RGB( 93,  93, 157),    // 71
    COLORQUAD_FROM_RGB(113, 113, 170),    // 72
    COLORQUAD_FROM_RGB(133, 133, 182),    // 73
    COLORQUAD_FROM_RGB(153, 153, 194),    // 74
    COLORQUAD_FROM_RGB(174, 174, 206),    // 75
    COLORQUAD_FROM_RGB(194, 194, 218),    // 76
    COLORQUAD_FROM_RGB(214, 214, 230),    // 77
    COLORQUAD_FROM_RGB(234, 234, 242),    // 78
    COLORQUAD_FROM_RGB(255, 255, 255),    // 79
    COLORQUAD_FROM_RGB(255,  12,   0),    // 80
    COLORQUAD_FROM_RGB(238,  28,   0),    // 81
    COLORQUAD_FROM_RGB(222,  44,   0),    // 82
    COLORQUAD_FROM_RGB(206,  60,   0),    // 83
    COLORQUAD_FROM_RGB(190,  76,   0),    // 84
    COLORQUAD_FROM_RGB(174,  93,   0),    // 85
    COLORQUAD_FROM_RGB(157, 109,   0),    // 86
    COLORQUAD_FROM_RGB(141, 125,   0),    // 87
    COLORQUAD_FROM_RGB(125, 141,   0),    // 88
    COLORQUAD_FROM_RGB(109, 157,   0),    // 89
    COLORQUAD_FROM_RGB( 93, 174,   0),    // 90
    COLORQUAD_FROM_RGB( 76, 190,   0),    // 91
    COLORQUAD_FROM_RGB( 60, 206,   0),    // 92
    COLORQUAD_FROM_RGB( 44, 222,   0),    // 93
    COLORQUAD_FROM_RGB( 28, 238,   0),    // 94
    COLORQUAD_FROM_RGB( 12, 255,   0),    // 95
    COLORQUAD_FROM_RGB(149, 149, 202),    // 96
    COLORQUAD_FROM_RGB(  0,   0, 170),    // 97
    COLORQUAD_FROM_RGB( 85,  85, 255),    // 98
    COLORQUAD_FROM_RGB(  0, 170,   0),    // 99
    COLORQUAD_FROM_RGB( 85, 255,  85),    // 100
    COLORQUAD_FROM_RGB(  0, 170, 170),    // 101
    COLORQUAD_FROM_RGB( 85, 255, 255),    // 102
    COLORQUAD_FROM_RGB(170,   0,   0),    // 103
    COLORQUAD_FROM_RGB(255,  85,  85),    // 104
    COLORQUAD_FROM_RGB(170,   0, 170),    // 105
    COLORQUAD_FROM_RGB(255,  85, 255),    // 106
    COLORQUAD_FROM_RGB(170, 170,   0),    // 107
    COLORQUAD_FROM_RGB(255, 255,  85),    // 108
    COLORQUAD_FROM_RGB(125,  97,   0),    // 109
    COLORQUAD_FROM_RGB(194,  97,   0),    // 110
    COLORQUAD_FROM_RGB(194,  97, 121),    // 111
    COLORQUAD_FROM_RGB(230, 137, 137),    // 112
    COLORQUAD_FROM_RGB(255, 121,   0),    // 113
    COLORQUAD_FROM_RGB(255, 194,   0),    // 114
    COLORQUAD_FROM_RGB(129,  64,  97),    // 115
    COLORQUAD_FROM_RGB(194,  97, 255),    // 116
    COLORQUAD_FROM_RGB( 68, 141,  68),    // 117
    COLORQUAD_FROM_RGB( 72, 157,  72),    // 118
    COLORQUAD_FROM_RGB( 76, 170,  76),    // 119
    COLORQUAD_FROM_RGB( 80, 186,  80),    // 120
    COLORQUAD_FROM_RGB( 85, 202,  85),    // 121
    COLORQUAD_FROM_RGB( 89, 214,  89),    // 122
    COLORQUAD_FROM_RGB( 93,   0,  93),    // 123
    COLORQUAD_FROM_RGB( 72,  72,   0),    // 124
    COLORQUAD_FROM_RGB( 48,  48,   0),    // 125
    COLORQUAD_FROM_RGB( 28,  28,   0),    // 126
    COLORQUAD_FROM_RGB(109,  72,  72),    // 127
    COLORQUAD_FROM_RGB(  0,  12,   0),    // 128
    COLORQUAD_FROM_RGB(  0,  28,   0),    // 129
    COLORQUAD_FROM_RGB(  0,  44,   0),    // 130
    COLORQUAD_FROM_RGB(  0,  60,   0),    // 131
    COLORQUAD_FROM_RGB(  0,  76,   0),    // 132
    COLORQUAD_FROM_RGB(  0,  93,   0),    // 133
    COLORQUAD_FROM_RGB(  0, 109,   0),    // 134
    COLORQUAD_FROM_RGB(  0, 125,   0),    // 135
    COLORQUAD_FROM_RGB(  0, 141,   0),    // 136
    COLORQUAD_FROM_RGB(  0, 157,   0),    // 137
    COLORQUAD_FROM_RGB(  0, 174,   0),    // 138
    COLORQUAD_FROM_RGB(  0, 190,   0),    // 139
    COLORQUAD_FROM_RGB(  0, 206,   0),    // 140
    COLORQUAD_FROM_RGB(  0, 222,   0),    // 141
    COLORQUAD_FROM_RGB(  0, 238,   0),    // 142
    COLORQUAD_FROM_RGB(  0, 255,   0),    // 143
    COLORQUAD_FROM_RGB( 12,  12,   0),    // 144
    COLORQUAD_FROM_RGB( 28,  28,   0),    // 145
    COLORQUAD_FROM_RGB( 44,  44,   0),    // 146
    COLORQUAD_FROM_RGB( 60,  60,   0),    // 147
    COLORQUAD_FROM_RGB( 76,  76,   0),    // 148
    COLORQUAD_FROM_RGB( 93,  93,   0),    // 149
    COLORQUAD_FROM_RGB(109, 109,   0),    // 150
    COLORQUAD_FROM_RGB(125, 125,   0),    // 151
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 152
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 153
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 154
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 155
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 156
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 157
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 158
    COLORQUAD_FROM_RGB(  0,   0,   0),    // 159
};

ui::ColorScheme::ColorScheme()
{
    afl::base::Memory<gfx::Color_t>(m_colors).fill(0);
}

ui::ColorScheme::~ColorScheme()
{ }

gfx::Color_t
ui::ColorScheme::getColor(uint8_t index)
{
    // ex GfxStandardColorScheme::getColor
    return index < Color_Avail ? m_colors[index] : m_colors[0];
}

void
ui::ColorScheme::drawBackground(gfx::Canvas& can, const gfx::Rectangle& area)
{
    // ex GfxStandardColorScheme::drawBackground
    can.drawBar(area, m_colors[0], gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
}

void
ui::ColorScheme::init(gfx::Canvas& can)
{
    can.setPalette(0, STANDARD_COLORS, m_colors);
}
