/**
  *  \file test/ui/widgets/charttest.cpp
  *  \brief Test for ui::widgets::Chart
  */

#include "ui/widgets/chart.hpp"

#include "afl/string/string.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "gfx/rgbapixmap.hpp"
#include "ui/draw.hpp"
#include "ui/skincolorscheme.hpp"
#include "util/numberformatter.hpp"

namespace {
    String_t getPixmapRow(const gfx::RGBAPixmap& pix, int y)
    {
        afl::base::Memory<const gfx::ColorQuad_t> row = pix.row(y);
        String_t result;
        while (const gfx::ColorQuad_t* q = row.eat()) {
            switch (*q) {
             case COLORQUAD_FROM_RGB(0,0,0):
                result += '#';
                break;
             case COLORQUAD_FROM_RGB(194,194,194):     // gray / background
                result += '.';
                break;
             case COLORQUAD_FROM_RGB(255,0,0):         // red
                result += 'R';
                break;
             case COLORQUAD_FROM_RGB(97,242,97):       // green
                result += 'G';
                break;
             case COLORQUAD_FROM_RGB(97,97,194):       // blue
                result += 'B';
                break;

             /* The following are anti-aliased colors, mostly derived from red */
             case COLORQUAD_FROM_RGB(204,162,162):
                result += 'r';
                break;
             case COLORQUAD_FROM_RGB(221,109,109):
                result += 's';
                break;
             case COLORQUAD_FROM_RGB(227,86,86):
                result += 't';
                break;
             case COLORQUAD_FROM_RGB(244,33,33):
                result += 'u';
                break;
             case COLORQUAD_FROM_RGB(217,119,119):
                result += 'v';
                break;
             case COLORQUAD_FROM_RGB(231,76,76):
                result += 'w';
                break;
             case COLORQUAD_FROM_RGB(251,12,12):
                result += 'x';
                break;
             case COLORQUAD_FROM_RGB(197,183,183):
                result += 'y';
                break;
             case COLORQUAD_FROM_RGB(224,98,98):
                result += 'z';
                break;
             case COLORQUAD_FROM_RGB(224,97,97):
                result += 'a';
                break;
             case COLORQUAD_FROM_RGB(231,77,77):
                result += 'b';
                break;
             case COLORQUAD_FROM_RGB(217,118,118):
                result += 'c';
                break;
             default:
                result += '?';
                break;
            }
        }
        return result;
    }

    void addDefaultChart(ui::widgets::Chart& w)
        {
            std::auto_ptr<util::DataTable> tab(new util::DataTable());
            util::DataTable::Row& r1 = tab->addRow(10);
            r1.set(0, 0);
            r1.set(1, 10);
            r1.set(2, 5);

            util::DataTable::Row& r2 = tab->addRow(20);
            r2.set(0, 5);
            r2.set(1, 4);
            r2.set(2, 6);

            tab->setColumnName(0, "a");
            tab->setColumnName(2, "b");

            w.setContent(tab);
            w.style(10).setColor(ui::Color_Red).setLineMode(ui::widgets::Chart::Line_NoAntiAliasing);
            w.style(20).setColor(ui::Color_Green).setLineMode(ui::widgets::Chart::Line_NoAntiAliasing);
        }
}

/** Regression test for rendering. */
AFL_TEST("ui.widgets.Chart:basics", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());
    util::NumberFormatter fmt(false, false);
    ui::SkinColorScheme colors(ui::GRAY_COLOR_SET, root.colorScheme());

    // Testee
    ui::widgets::Chart testee(root, gfx::Point(50, 30), fmt);
    testee.setColorScheme(colors);
    testee.setExtent(gfx::Rectangle(0, 0, 50, 30));
    addDefaultChart(testee);

    // Draw
    afl::base::Ref<gfx::RGBAPixmap> pix(gfx::RGBAPixmap::create(50, 30));
    testee.draw(*pix->makeCanvas());

    // Verify
    // printf("\n");
    // for (int i = 0; i < 30; ++i) {
    //     printf("\t%s\n", getPixmapRow(*pix, i).c_str());
    // }
    a.checkEqual("01", getPixmapRow(*pix,  0), "...##....#####....................................");
    a.checkEqual("02", getPixmapRow(*pix,  1), "..###...##..###......#.............R..............");
    a.checkEqual("03", getPixmapRow(*pix,  2), "...##...##.####.....###...........RRR.............");
    a.checkEqual("04", getPixmapRow(*pix,  3), "...##...####.##....#####..........RRR.............");
    a.checkEqual("05", getPixmapRow(*pix,  4), "...##...###..##......#............R..R............");
    a.checkEqual("06", getPixmapRow(*pix,  5), "...##...##...##......#...........R....R...........");
    a.checkEqual("07", getPixmapRow(*pix,  6), ".######..#####.......#...........R.....R..........");
    a.checkEqual("08", getPixmapRow(*pix,  7), ".....................#..........R.......R.........");
    a.checkEqual("09", getPixmapRow(*pix,  8), ".....................#..........R........R........");
    a.checkEqual("10", getPixmapRow(*pix,  9), ".....................#.........R..........R.G.....");
    a.checkEqual("11", getPixmapRow(*pix, 10), ".....................#...G.....R...........GGG....");
    a.checkEqual("12", getPixmapRow(*pix, 11), ".....................#..GGGG..R.........GGGRGR....");
    a.checkEqual("13", getPixmapRow(*pix, 12), ".....................#...G..GGGGG..G.GGG....R.....");
    a.checkEqual("14", getPixmapRow(*pix, 13), ".....................#.......R...GGGG.............");
    a.checkEqual("15", getPixmapRow(*pix, 14), ".........#####.......#......R......G..............");
    a.checkEqual("16", getPixmapRow(*pix, 15), "........##..###......#......R.....................");
    a.checkEqual("17", getPixmapRow(*pix, 16), "........##.####......#.....R......................");
    a.checkEqual("18", getPixmapRow(*pix, 17), "........####.##......#.....R......................");
    a.checkEqual("19", getPixmapRow(*pix, 18), "........###..##......#....R...................#...");
    a.checkEqual("20", getPixmapRow(*pix, 19), "........##...##......#...RR...................##..");
    a.checkEqual("21", getPixmapRow(*pix, 20), ".........#####.......###RRR######################.");
    a.checkEqual("22", getPixmapRow(*pix, 21), ".........................R....................##..");
    a.checkEqual("23", getPixmapRow(*pix, 22), "..........................................##..#...");
    a.checkEqual("24", getPixmapRow(*pix, 23), "..........................................##......");
    a.checkEqual("25", getPixmapRow(*pix, 24), ".#####....................................##......");
    a.checkEqual("26", getPixmapRow(*pix, 25), ".....##...................................######..");
    a.checkEqual("27", getPixmapRow(*pix, 26), ".######...................................##...##.");
    a.checkEqual("28", getPixmapRow(*pix, 27), "##...##...................................##...##.");
    a.checkEqual("29", getPixmapRow(*pix, 28), ".######...................................######..");
    a.checkEqual("30", getPixmapRow(*pix, 29), "..................................................");
}

/** Regression test for rendering: ExtendRight mode. */
AFL_TEST("ui.widgets.Chart:Line_ExtendRight", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());
    util::NumberFormatter fmt(false, false);
    ui::SkinColorScheme colors(ui::GRAY_COLOR_SET, root.colorScheme());

    // Testee
    ui::widgets::Chart testee(root, gfx::Point(50, 30), fmt);
    testee.setColorScheme(colors);
    testee.setExtent(gfx::Rectangle(0, 0, 50, 30));
    addDefaultChart(testee);

    // Configure "extend right"
    testee.style(10).setLineMode(ui::widgets::Chart::Line_ExtendRight | ui::widgets::Chart::Line_NoAntiAliasing);

    // Draw
    afl::base::Ref<gfx::RGBAPixmap> pix(gfx::RGBAPixmap::create(50, 30));
    testee.draw(*pix->makeCanvas());

    // Verify
    a.checkEqual("01", getPixmapRow(*pix,  8), ".....................#..........R........R........");
    a.checkEqual("02", getPixmapRow(*pix,  9), ".....................#.........R..........R.G.....");
    a.checkEqual("03", getPixmapRow(*pix, 10), ".....................#...G.....R...........GGG....");
    a.checkEqual("04", getPixmapRow(*pix, 11), ".....................#..GGGG..R.........GGGRGRRRR.");
    a.checkEqual("05", getPixmapRow(*pix, 12), ".....................#...G..GGGGG..G.GGG....R.....");
    a.checkEqual("06", getPixmapRow(*pix, 13), ".....................#.......R...GGGG.............");
}

/** Regression test for rendering: Skip mode and aux data. */
AFL_TEST("ui.widgets.Chart:Line_SkipGaps", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());
    util::NumberFormatter fmt(false, false);
    ui::SkinColorScheme colors(ui::GRAY_COLOR_SET, root.colorScheme());

    // Testee
    ui::widgets::Chart testee(root, gfx::Point(50, 30), fmt);
    testee.setColorScheme(colors);
    testee.setExtent(gfx::Rectangle(0, 0, 50, 30));
    addDefaultChart(testee);

    // Add an aux chart
    std::auto_ptr<util::DataTable> tab(new util::DataTable());
    util::DataTable::Row& r1 = tab->addRow(5);
    r1.set(0, 10);
    // No point at 1
    r1.set(2, 0);
    testee.setAuxContent(tab);
    testee.style(5).setColor(ui::Color_Blue).setLineMode(ui::widgets::Chart::Line_SkipGaps);

    // Draw
    afl::base::Ref<gfx::RGBAPixmap> pix(gfx::RGBAPixmap::create(50, 30));
    testee.draw(*pix->makeCanvas());

    // Verify
    a.checkEqual("01", getPixmapRow(*pix,  0), "...##....#####....................................");
    a.checkEqual("02", getPixmapRow(*pix,  1), "..###...##..###......#...B.........R..............");
    a.checkEqual("03", getPixmapRow(*pix,  2), "...##...##.####.....###.BBB.......RRR.............");
    a.checkEqual("04", getPixmapRow(*pix,  3), "...##...####.##....#####.B........RRR.............");
    a.checkEqual("05", getPixmapRow(*pix,  4), "...##...###..##......#............R..R............");
    a.checkEqual("06", getPixmapRow(*pix,  5), "...##...##...##......#...........R....R...........");
    a.checkEqual("07", getPixmapRow(*pix,  6), ".######..#####.......#...........R.....R..........");
    a.checkEqual("08", getPixmapRow(*pix,  7), ".....................#..........R.......R.........");
    a.checkEqual("09", getPixmapRow(*pix,  8), ".....................#..........R........R........");
    a.checkEqual("10", getPixmapRow(*pix,  9), ".....................#.........R..........R.G.....");
    a.checkEqual("11", getPixmapRow(*pix, 10), ".....................#...G.....R...........GGG....");
    a.checkEqual("12", getPixmapRow(*pix, 11), ".....................#..GGGG..R.........GGGRGR....");
    a.checkEqual("13", getPixmapRow(*pix, 12), ".....................#...G..GGGGG..G.GGG....R.....");
    a.checkEqual("14", getPixmapRow(*pix, 13), ".....................#.......R...GGGG.............");
    a.checkEqual("15", getPixmapRow(*pix, 14), ".........#####.......#......R......G..............");
    a.checkEqual("16", getPixmapRow(*pix, 15), "........##..###......#......R.....................");
    a.checkEqual("17", getPixmapRow(*pix, 16), "........##.####......#.....R......................");
    a.checkEqual("18", getPixmapRow(*pix, 17), "........####.##......#.....R......................");
    a.checkEqual("19", getPixmapRow(*pix, 18), "........###..##......#....R...................#...");
    a.checkEqual("20", getPixmapRow(*pix, 19), "........##...##......#...RR.................B.##..");
    a.checkEqual("21", getPixmapRow(*pix, 20), ".........#####.......###RRR################BBB###.");
    a.checkEqual("22", getPixmapRow(*pix, 21), ".........................R..................B.##..");
    a.checkEqual("23", getPixmapRow(*pix, 22), "..........................................##..#...");
    a.checkEqual("24", getPixmapRow(*pix, 23), "..........................................##......");
    a.checkEqual("25", getPixmapRow(*pix, 24), ".#####....................................##......");
    a.checkEqual("26", getPixmapRow(*pix, 25), ".....##...................................######..");
    a.checkEqual("27", getPixmapRow(*pix, 26), ".######...................................##...##.");
    a.checkEqual("28", getPixmapRow(*pix, 27), "##...##...................................##...##.");
    a.checkEqual("29", getPixmapRow(*pix, 28), ".######...................................######..");
    a.checkEqual("30", getPixmapRow(*pix, 29), "..................................................");
}

/** Regression test for rendering: antialiasing. Otherwise same as ExtendRight mode. */
AFL_TEST("ui.widgets.Chart:anti-alias", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());
    util::NumberFormatter fmt(false, false);
    ui::SkinColorScheme colors(ui::GRAY_COLOR_SET, root.colorScheme());

    // Testee
    ui::widgets::Chart testee(root, gfx::Point(50, 30), fmt);
    testee.setColorScheme(colors);
    testee.setExtent(gfx::Rectangle(0, 0, 50, 30));
    addDefaultChart(testee);

    // Configure "extend right", default (=enabled) anti-aliasing
    testee.style(10).setLineMode(ui::widgets::Chart::Line_ExtendRight);

    // Draw
    afl::base::Ref<gfx::RGBAPixmap> pix(gfx::RGBAPixmap::create(50, 30));
    testee.draw(*pix->makeCanvas());

    // Verify
    a.checkEqual("01", getPixmapRow(*pix,  8), ".....................#.........ur.......st........");
    a.checkEqual("02", getPixmapRow(*pix,  9), ".....................#........vw.........st.G.....");
    a.checkEqual("03", getPixmapRow(*pix, 10), ".....................#...G....xy..........sGGG....");
    a.checkEqual("04", getPixmapRow(*pix, 11), ".....................#..GGGG.za.........GGGRGRRRR.");
    a.checkEqual("05", getPixmapRow(*pix, 12), ".....................#...G..GGGGG..G.GGG....R.....");
    a.checkEqual("06", getPixmapRow(*pix, 13), ".....................#......bc...GGGG.............");
}
