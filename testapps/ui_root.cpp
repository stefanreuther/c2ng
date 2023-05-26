/**
  *  \file test_apps/ui_root.cpp
  */

#include <cstdlib>
#include <iostream>
#include "config.h"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directory.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/environment.hpp"
#include "client/widgets/alliancestatuslist.hpp"
#include "client/widgets/filelistbox.hpp"
#include "client/widgets/playerlist.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "client/widgets/standarddataview.hpp"
#include "gfx/complex.hpp"
#include "ui/defaultresourceprovider.hpp"
#include "ui/draw.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/icons/colortile.hpp"
#include "ui/layout/flow.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/res/ccimageloader.hpp"
#include "ui/res/directoryprovider.hpp"
#include "ui/res/engineimageloader.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "ui/widgets/abstractcheckbox.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/cardtabbar.hpp"
#include "ui/widgets/checkbox.hpp"
#include "ui/widgets/checkboxlistbox.hpp"
#include "ui/widgets/editor.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/icongrid.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/radiobutton.hpp"
#include "ui/widgets/richlistbox.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/simpletable.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/widgets/tabbar.hpp"
#include "ui/widgets/treelistbox.hpp"
#include "ui/window.hpp"
#include "util/consolelogger.hpp"
#include "util/editor/editor.hpp"
#include "util/rich/colorattribute.hpp"
#include "util/rich/linkattribute.hpp"
#include "util/rich/parser.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/syntax/keywordtable.hpp"
#include "util/syntax/scripthighlighter.hpp"
#ifdef HAVE_SDL
# include "gfx/sdl/engine.hpp"
typedef gfx::sdl::Engine Engine_t;
#elif defined(HAVE_SDL2)
# include "gfx/sdl2/engine.hpp"
typedef gfx::sdl2::Engine Engine_t;
#else
# error "foo"
#endif

namespace {
    void addFrames(ui::Window& win, afl::base::Deleter& del, ui::EventLoop& loop, ui::Root& root, ui::FrameType type)
    {
        static const int widths[] = { 0, 1, 1, 2, 2, 3, 5, 10 };
        static const int pads[]   = { 0, 0, 3, 0, 3, 1, 1, 1 };

        ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
        for (size_t i = 0; i < sizeof(widths)/sizeof(widths[0]); ++i) {
            ui::widgets::FrameGroup& fg = del.addNew(new ui::widgets::FrameGroup(ui::layout::VBox::instance5, root.colorScheme(), type));
            fg.setFrameWidth(widths[i]);
            fg.setPadding(pads[i]);
            ui::widgets::Button& btn = del.addNew(new ui::widgets::Button("X", ' ', root));
            btn.sig_fire.addNewClosure(loop.makeStop(1));
            fg.add(btn);
            g.add(fg);
        }
        win.add(g);
    }

    void printInt(int i)
    {
        std::cout << "Result = " << i << "\n";
    }

    class MyListbox : public ui::widgets::AbstractListbox {
     public:
        MyListbox()
            : AbstractListbox()
            { }
        virtual ui::layout::Info getLayoutInfo() const
            { return gfx::Point(200, 110); }
        virtual void handlePositionChange()
            { }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }
        virtual size_t getNumItems() const
            { return 10; }
        virtual bool isItemAccessible(size_t /*n*/) const
            { return true; }
        virtual int getItemHeight(size_t /*n*/) const
            { return 16; }
        virtual int getHeaderHeight() const
            { return 5; }
        virtual int getFooterHeight() const
            { return 0; }
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area)
            {
                gfx::ColorQuad_t cq[1] = {COLORQUAD_FROM_RGBA(128,0,0,0)};
                gfx::Color_t c[1];
                can.encodeColors(cq, c);
                can.drawBar(area, c[0], gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
            }
        virtual void drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
            {
                gfx::ColorQuad_t cq[2] = {COLORQUAD_FROM_RGBA(0,16*item+20,0,0), COLORQUAD_FROM_RGBA(255,255,255,0)};
                gfx::Color_t c[2];
                can.encodeColors(cq, c);
                can.drawBar(area, c[0], gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);

                if (state == FocusedItem || state == ActiveItem) {
                    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
                    ctx.setRawColor(c[1]);
                    drawRectangle(ctx, area);
                }
            }
    };

    class MyWidget : public ui::SimpleWidget {
     public:
        MyWidget(ui::Root& root, bool& stop, int id)
            : SimpleWidget(),
              m_color(COLORQUAD_FROM_RGBA(std::rand()&255, std::rand()&255, std::rand()&255, 255)),
              m_root(root),
              m_stop(stop),
              m_blinkState(false),
              m_timer(root.engine().createTimer()),
              m_id(id)
            {
                setExtent(gfx::Rectangle(std::rand()%540, std::rand()%380, 100, 100));
                m_timer->setInterval(500);
                m_timer->sig_fire.add(this, &MyWidget::tick);
            }

        void testWidget(ui::Widget& w)
            {
                afl::base::Deleter del;
                ui::EventLoop loop(m_root);
                ui::Window window("Test window", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);

                window.add(w);

                ui::widgets::Button& btn = del.addNew(new ui::widgets::Button("OK", util::Key_Return, m_root));
                btn.sig_fire.addNewClosure(loop.makeStop(0));
                window.add(btn);
                window.pack();
                m_root.centerWidget(window);
                m_root.addChild(window, 0);
                loop.run();
            }

        void testPlayerList(client::widgets::PlayerList::Layout lay, int preferredWidth)
            {
                client::widgets::PlayerList pl(m_root, lay, pl.ShowNames, pl.PlayerColors, preferredWidth, game::PlayerSet_t::allUpTo(12));
                pl.setName(1, "Feds");
                pl.setName(2, "Lizard");
                pl.setName(3, "Bird Men");
                pl.setName(4, "Klingon");
                pl.setName(5, "Privateer");
                pl.setName(6, "Cyborg");
                pl.setName(7, "Tholian");
                pl.setName(8, "Imperial");
                pl.setName(9, "Robot");
                pl.setName(10, "Rebel");
                pl.setName(11, "Colonial");
                pl.setName(12, "Alien");

                testWidget(pl);
            }

        void testFileList()
            {
                using client::widgets::FileListbox;
                FileListbox box(2, 7, m_root);

                FileListbox::Items_t items;
                items.push_back(FileListbox::Item("up", 0, true, FileListbox::iUp));
                for (int i = 0; i < 10; ++i) {
                    items.push_back(FileListbox::Item("directory", 1, true, FileListbox::iFolder));
                }
                for (int i = 0; i < 20; ++i) {
                    items.push_back(FileListbox::Item("file", 1, false, FileListbox::iFile));
                }
                box.swapItems(items);

                testWidget(box);
            }

        void testCheckboxList(ui::widgets::CheckboxListbox::Layout lay)
            {
                ui::widgets::CheckboxListbox box(m_root, lay);
                box.setItemImageName(box.setItemInfo(box.addItem(1, "label one"), "info one"), "ui.cb0");
                box.setItemImageName(box.setItemInfo(box.addItem(2, "label two"), "info two"), "ui.cb1");
                box.setItemAccessible(box.setItemImageName(box.setItemInfo(box.addItem(3, "label three"), "info three"), "ui.cb0"), false);
                box.setItemImageName(box.setItemInfo(box.addItem(4, "label four"), "info four"), "ui.cb0");

                testWidget(box);
            }

        virtual void draw(gfx::Canvas& can)
            {
                can.drawBar(getExtent(), m_blinkState ? m_color : ~m_color, m_color, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
                gfx::Rectangle r = getExtent();
                r.grow(-5, -5);
                can.drawBar(r, m_root.colorScheme().getColor(uint8_t(m_id+1)), m_color, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
            }
        virtual void handleStateChange(State /*st*/, bool /*enable*/)
            { }
        virtual void handlePositionChange()
            { }
        virtual ui::layout::Info getLayoutInfo() const
            { return ui::layout::Info(); }
        virtual bool handleKey(util::Key_t key, int /*prefix*/)
            {
                switch (key) {
                 case 'q':
                 case util::Key_Quit:
                    m_stop = true;
                    return true;

                 case 'n':
                    m_root.addChild(*new MyWidget(m_root, m_stop, m_id + 1), 0);
                    return true;

                 case 'b':
                 {
                     // Build a button window
                     ui::Window window("Test Window", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                     ui::widgets::Button btn1("One", '1', m_root);
                     ui::widgets::Button btn2("Two", '2', m_root);
                     ui::widgets::Button btn3("Three", '3', m_root);
                     ui::widgets::InputLine input(100, 40, m_root);
                     MyListbox box;
                     window.add(btn1);
                     window.add(btn2);
                     window.add(btn3);
                     window.add(input);
                     window.add(box);
                     window.pack();
                     m_root.centerWidget(window);
                     m_root.addChild(window, 0);

                     ui::EventLoop loop(m_root);
                     btn1.sig_fire.addNewClosure(loop.makeStop(1));
                     btn2.sig_fire.addNewClosure(loop.makeStop(2));
                     btn3.sig_fire.addNewClosure(loop.makeStop(3));
                     int i = loop.run();
                     std::cout << "Closed using button " << i << "\n";
                     return true;
                 }

                 case 'i':
                 {
                     afl::string::NullTranslator tx;
                     ui::widgets::InputLine(10, m_root).
                         setFont(gfx::FontRequest().addSize(1)).
                         setText("hello").
                         doStandardDialog("Input", "Type here:", tx);
                     return true;
                 }

                 case 'I':
                 {
                     afl::base::Deleter del;
                     gfx::Point size(24, 24);
                     ui::widgets::IconGrid g(m_root.engine(), size, 10, 10);
                     for (int i = 0; i < 256; ++i) {
                         g.addIcon(&del.addNew(new ui::icons::ColorTile(m_root, size, uint8_t(i))));
                     }
                     g.setPadding(1);
                     testWidget(g);
                     return true;
                 }

                 case 'r':
                 {
                     ui::rich::DocumentView view(gfx::Point(200, 200),
                                                 ui::rich::DocumentView::fl_Help,
                                                 m_root.provider());
                     ui::rich::Document& doc = view.getDocument();

                     ui::Window window("Test Window", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                     window.add(view);

                     ui::widgets::Button btn("OK", util::Key_Return, m_root);
                     window.add(btn);
                     window.pack();

                     doc.add("Hello, rich text world");
                     doc.addParagraph();
                     doc.add("This is some rich text. ");
                     doc.add(util::rich::Text("It can use bold. ").withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Bold)));
                     doc.add(util::rich::Text("Or underline.").withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Underline)));
                     doc.add(util::rich::Text(" Or fixed width. ").withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Fixed)));
                     doc.add(util::rich::Text("Or all of it.").
                             withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Fixed)).
                             withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Underline)).
                             withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Bold)));
                     doc.add(util::rich::Text(" Even a bigger font.").withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Big)));
                     doc.add(util::rich::Text(" Did I say I can use color?").withNewAttribute(new util::rich::ColorAttribute(util::SkinColor::Red)));
                     doc.addParagraph();
                     doc.add(util::rich::Text("This is text with "));
                     doc.add(util::rich::Text("a link").withNewAttribute(new util::rich::LinkAttribute("hu")));
                     doc.add(util::rich::Text(" and another "));
                     doc.add(util::rich::Text("link").withNewAttribute(new util::rich::LinkAttribute("hu")));
                     doc.add(util::rich::Text("."));
                     doc.addParagraph();
                     doc.add("Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula. Phasellus at purus sed purus cursus iaculis. Suspendisse fermentum. Pellentesque et arcu.");
                     doc.addParagraph();
                     doc.add("Maecenas viverra. In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros, sit amet sagittis nunc mi ac neque.");
                     doc.finish();

                     m_root.centerWidget(window);
                     m_root.addChild(window, 0);

                     ui::EventLoop loop(m_root);
                     btn.sig_fire.addNewClosure(loop.makeStop(1));
                     loop.run();
                     return true;
                 }

                 case 'R':
                 {
                     ui::widgets::RichListbox box(m_root.provider(), m_root.colorScheme());
                     box.addItem("Plain text", 0, true);
                     box.addItem(util::rich::Text("Bold text").withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Bold)), 0, true);
                     box.addItem(util::rich::Text("Bold text").withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Bold))
                                 + " followed by "
                                 + util::rich::Text("fixed text").withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Fixed)), 0, true);
                     box.addItem("Maecenas viverra. In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros, sit amet sagittis nunc mi ac neque.", 0, true);
                     box.addItem(util::rich::Text("Large text").withNewAttribute(new util::rich::StyleAttribute(util::rich::StyleAttribute::Big)), 0, true);

                     ui::Window window("Test Window", m_root.provider(), m_root.colorScheme(), ui::BLUE_BLACK_WINDOW, ui::layout::VBox::instance5);
                     window.add(box);

                     ui::widgets::Button btn("OK", util::Key_Return, m_root);
                     window.add(btn);
                     window.pack();

                     m_root.centerWidget(window);
                     m_root.addChild(window, 0);

                     ui::EventLoop loop(m_root);
                     btn.sig_fire.addNewClosure(loop.makeStop(1));
                     loop.run();
                     return true;
                 }

                 case 'p':
                    testPlayerList(client::widgets::PlayerList::FlowLayout, 300);
                    return true;

                 case 'P':
                    testPlayerList(client::widgets::PlayerList::VerticalLayout, 0);
                    return true;

                 case 'l':
                 {
                     ui::widgets::StringListbox box(m_root.provider(), m_root.colorScheme());
                     box.addItem(1, "foo");
                     box.addItem(2, "bar");
                     box.addItem(5, "Maecenas viverra. In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros, sit amet sagittis nunc mi ac neque.");
                     box.addItem(6, "Öhm. nö?");
                     box.addItem(3, "baz");
                     box.addItem(4, "qux");
                     box.setPreferredWidth(30, false);

                     ui::Window window("Test Window", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                     window.add(box);

                     ui::widgets::Button btn("OK", util::Key_Return, m_root);
                     window.add(btn);
                     window.pack();

                     m_root.centerWidget(window);
                     m_root.addChild(window, 0);

                     ui::EventLoop loop(m_root);
                     btn.sig_fire.addNewClosure(loop.makeStop(1));
                     loop.run();
                     return true;
                 }

                 case 'L':
                 {
                     ui::icons::ColorTile blackTile(m_root, gfx::Point(20, 30), ui::Color_Black);
                     ui::icons::ColorTile whiteTile(m_root, gfx::Point(20, 30), ui::Color_White);
                     ui::widgets::TreeListbox tree(m_root, 6, 100);
                     tree.addNode(0, 0, "Configuration", !false);
                     tree.addNode(1, 1, "Size", false);
                     tree.addNode(2, 1, "Color", false);
                     tree.addNode(3, 2, "Black", false);
                     tree.addNode(4, 2, "White", false);
                     tree.addNode(5, 1, "Weight", false);
                     tree.addNode(6, 1, "Speed", false);
                     tree.addNode(7, 0, "Action", false);
                     tree.addNode(8, 1, "Eat", false);
                     tree.addNode(9, 1, "Drink", false);
                     tree.addNode(10, 1, "Sleep", false);
                     tree.addNode(10, 1, "Repeat", false);
                     tree.setIcon(tree.findNodeById(3), &blackTile);
                     tree.setIcon(tree.findNodeById(4), &whiteTile);

                     class Handler : public afl::base::Closure<void(int32_t)> {
                      public:
                         Handler(ui::widgets::TreeListbox& tree)
                             : m_tree(tree)
                             { }
                         virtual void call(int32_t i)
                             { m_tree.setIcon(m_tree.findNodeById(i), 0); }
                      private:
                         ui::widgets::TreeListbox& m_tree;
                     };
                     tree.sig_iconClick.addNewClosure(new Handler(tree));

                     ui::widgets::ScrollbarContainer cont(tree, m_root);

                     testWidget(cont);
                     return true;
                 }

                 case 't':
                 {
                     ui::widgets::SimpleTable t(m_root, 3, 4);
                     t.column(0).subrange(0, 3).setExtraColumns(1);
                     t.cell(0, 0).setText("Amount:");
                     t.cell(0, 1).setText("Auto-B. Goal:");
                     t.cell(0, 2).setText("Maximum:");

                     t.column(2).subrange(0, 3).setColor(ui::Color_Green).setTextAlign(gfx::RightAlign, gfx::TopAlign);
                     t.cell(2, 0).setText("12");
                     t.cell(2, 1).setText("[max]");
                     t.cell(2, 2).setText("213");

                     t.cell(0, 3).setText("Cost:");
                     t.cell(1, 3).setExtraColumns(1).setColor(ui::Color_Green).setText("4 mc + 1 supply").setTextAlign(gfx::RightAlign, gfx::TopAlign);

                     testWidget(t);
                     return true;
                 }

                 case 'T':
                 {
                     afl::base::Deleter del;
                     ui::Group g(ui::layout::VBox::instance5);
                     ui::CardGroup cc;
                     ui::widgets::CardTabBar bar(m_root, cc);
                     for (int i = 0; i < 5; ++i) {
                         ui::widgets::Button& btn = del.addNew(new ui::widgets::Button(afl::string::Format("Button %d", i), 'x', m_root));
                         cc.add(btn);
                         bar.addPage(afl::string::Format("Page %d", i), 'a' + i, btn);
                     }
                     g.add(bar);
                     g.add(cc);
                     testWidget(g);
                     return true;
                 }

                 // case 'v':
                 // {
                 //     afl::base::Deleter del;
                 //     ui::EventLoop loop(m_root);
                 //     ui::Window window("Test window", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);

                 //     client::widgets::StandardDataView sdv1(m_root, gfx::Point(20, 10));
                 //     client::widgets::StandardDataView sdv2(m_root, gfx::Point(20, 10));
                 //     client::widgets::StandardDataView sdv3(m_root, gfx::Point(20, 10));
                 //     sdv1.setTitle("Title 1");
                 //     sdv2.setTitle("Title 2");
                 //     sdv3.setTitle("Title 3");
                 //     sdv2.setViewState(sdv2.HeadingOnly);
                 //     sdv3.setViewState(sdv2.DataOnly);
                 //     const char text[] = "Some text here with <b>bold</b> and <u>underlined</u> and <font color=\"green\">green</font>.";
                 //     sdv1.setText(util::rich::Parser::parseXml(String_t("<big>1</big>\n") + text));
                 //     sdv2.setText(util::rich::Parser::parseXml(String_t("<big>2</big>\n") + text));
                 //     sdv3.setText(util::rich::Parser::parseXml(String_t("<big>3</big>\n") + text));
                 //     window.add(sdv1);
                 //     window.add(sdv2);
                 //     window.add(sdv3);

                 //     ui::widgets::Button& btn = del.addNew(new ui::widgets::Button("OK", util::Key_Return, m_root));
                 //     btn.sig_fire.addNewClosure(loop.makeStop(0));
                 //     window.add(btn);
                 //     window.pack();
                 //     m_root.centerWidget(window);
                 //     m_root.addChild(window, 0);

                 //     btn.sig_fire.addNewClosure(loop.makeStop(1));
                 //     loop.run();
                 //     return true;
                 // }

                 case 'f':
                 case 'F':
                 case 'g':
                 case 'G':
                 {
                     afl::base::Deleter del;
                     ui::layout::Manager* layout;
                     if (key == 'F') {
                         layout = &del.addNew(new ui::layout::Flow(3, true));
                     } else if (key == 'f') {
                         layout = &del.addNew(new ui::layout::Flow(3, false));
                     } else if (key == 'G') {
                         ui::layout::Grid& g = del.addNew(new ui::layout::Grid(3));
                         g.setForcedCellSize(100, afl::base::Nothing);
                         layout = &g;
                     } else {
                         layout = &del.addNew(new ui::layout::Grid(3));
                     }
                     ui::EventLoop loop(m_root);
                     ui::Window window("Test window", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, *layout);

                     window.add(del.addNew(new ui::widgets::Button("one", '1', m_root)));
                     window.add(del.addNew(new ui::widgets::Button("two", '2', m_root)));
                     window.add(del.addNew(new ui::widgets::Button("three", '3', m_root)));
                     window.add(del.addNew(new ui::widgets::Button("four", '4', m_root)));
                     window.add(del.addNew(new ui::widgets::Button("five", '5', m_root)));
                     window.add(del.addNew(new ui::widgets::Button("six", '6', m_root)));
                     window.add(del.addNew(new ui::widgets::Button("seeeeeeeven", '7', m_root)));
                     window.add(del.addNew(new ui::widgets::Button("eight", '8', m_root)));
                     window.add(del.addNew(new ui::widgets::Button("nine", '9', m_root)));
                     window.add(del.addNew(new ui::widgets::Button("ten", '0', m_root)));

                     ui::widgets::Button& btn = del.addNew(new ui::widgets::Button("OK", util::Key_Return, m_root));
                     btn.sig_fire.addNewClosure(loop.makeStop(0));
                     window.add(btn);
                     window.pack();
                     m_root.centerWidget(window);
                     m_root.addChild(window, 0);

                     btn.sig_fire.addNewClosure(loop.makeStop(1));
                     loop.run();
                     return true;
                 }

                 case 'a':
                 {
                     afl::string::NullTranslator tx;
                     client::widgets::AllianceStatusList asl(m_root, tx);
                     asl.add(1, "Federation", client::widgets::AllianceStatusList::ItemFlags_t(client::widgets::AllianceStatusList::Self));
                     asl.add(2, "Lizard", client::widgets::AllianceStatusList::ItemFlags_t(client::widgets::AllianceStatusList::WeOffer));
                     asl.add(3, "Bird", client::widgets::AllianceStatusList::ItemFlags_t(client::widgets::AllianceStatusList::TheyOffer));
                     asl.add(4, "Klingon", client::widgets::AllianceStatusList::ItemFlags_t(client::widgets::AllianceStatusList::TheyOffer) + client::widgets::AllianceStatusList::Enemy);
                     asl.add(5, "Orion", client::widgets::AllianceStatusList::ItemFlags_t());

                     testWidget(asl);

                     return true;
                 }

                 case 'e':
                 {
                     game::ref::UserList ul;
                     ul.add(ul.DividerItem, "SMALL DEEP SPACE FREIGHTER", game::Reference(), false, game::map::Object::Playable, util::SkinColor::Static);
                     ul.add(ul.SubdividerItem, "The Lizards", game::Reference(), false, game::map::Object::Playable, util::SkinColor::Static);
                     ul.add(ul.ReferenceItem, "Listiger Lurch", game::Reference(), false, game::map::Object::Playable, util::SkinColor::Green);
                     ul.add(ul.ReferenceItem, "Crocodile Dundee", game::Reference(), true, game::map::Object::Playable, util::SkinColor::Green);
                     ul.add(ul.SubdividerItem, "The Bird Men", game::Reference(), false, game::map::Object::Playable, util::SkinColor::Static);
                     ul.add(ul.ReferenceItem, "Starling", game::Reference(), false, game::map::Object::Playable, util::SkinColor::Red);
                     ul.add(ul.ReferenceItem, "Eagle", game::Reference(), false, game::map::Object::Playable, util::SkinColor::Red);
                     ul.add(ul.OtherItem, "Some Link", game::Reference(), false, game::map::Object::Playable, util::SkinColor::Static);

                     client::widgets::ReferenceListbox list(m_root);
                     list.setContent(ul);
                     testWidget(list);
                     return true;
                 }

                 case 'E':
                 {
                     class Filter : public ui::widgets::Editor::CharacterFilter_t {
                      public:
                         bool call(afl::charset::Unichar_t ch)
                             { return (ch >= 32 && ch < 127); }
                     };

                     Filter f;
                     util::editor::Editor ed;
                     util::syntax::KeywordTable tab;
                     util::syntax::ScriptHighlighter sh(tab);
                     ui::widgets::Editor edWidget(ed, m_root);
                     ed.setLengthLimit(40);
                     edWidget.setPreferredSizeInCells(40, 20);
                     edWidget.setFlag(util::editor::AllowCursorAfterEnd, true);
                     edWidget.setHighlighter(&sh);
                     edWidget.setCharacterFilter(&f);
                     testWidget(edWidget);
                     return true;
                 }

                 case 'k':
                    testCheckboxList(ui::widgets::CheckboxListbox::SingleLine);
                    return true;

                 case 'K':
                    testCheckboxList(ui::widgets::CheckboxListbox::MultiLine);
                    return true;

                 case 'x':
                 {
                     afl::base::Deleter del;
                     ui::EventLoop loop(m_root);
                     ui::Window window("Test window", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);

                     afl::base::Observable<int> value;
                     ui::widgets::AbstractCheckbox cb1(m_root, 'a', "an item", gfx::Point(20, 20));
                     ui::widgets::AbstractCheckbox cb2(m_root, 'b', "better item", gfx::Point(20, 20));
                     ui::widgets::AbstractCheckbox cb3(m_root, 'c', "crazy item", gfx::Point(20, 20));
                     ui::widgets::AbstractCheckbox cb4(m_root, 'd', "damned item", gfx::Point(20, 20));
                     ui::widgets::Checkbox cb5(m_root, 'e', "extra item", value);
                     ui::widgets::RadioButton rb6(m_root, 'f', "f?", value, 0);
                     ui::widgets::RadioButton rb7(m_root, 'g', "good.", value, 1);
                     window.add(cb1);
                     window.add(cb2);
                     window.add(cb3);
                     window.add(cb4);
                     window.add(cb5);
                     window.add(rb6);
                     window.add(rb7);
                     cb1.setImage("ui.cb0");
                     cb2.setImage("ui.cb1");
                     cb3.setImage("ui.cbc");
                     cb4.setImage("ui.cb0");
                     cb5.addDefaultImages();

                     ui::widgets::Button btn("OK", util::Key_Return, m_root);
                     window.add(btn);
                     window.pack();
                     m_root.centerWidget(window);
                     m_root.addChild(window, 0);

                     btn.sig_fire.addNewClosure(loop.makeStop(1));
                     loop.run();
                     return true;
                 }

                 case 'c':
                 {
                     afl::base::Deleter del;
                     ui::EventLoop loop(m_root);
                     ui::Window window("Test window", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                     addFrames(window, del, loop, m_root, ui::NoFrame);
                     addFrames(window, del, loop, m_root, ui::RedFrame);
                     addFrames(window, del, loop, m_root, ui::YellowFrame);
                     addFrames(window, del, loop, m_root, ui::GreenFrame);
                     addFrames(window, del, loop, m_root, ui::RaisedFrame);
                     addFrames(window, del, loop, m_root, ui::LoweredFrame);
                     window.pack();
                     m_root.centerWidget(window);
                     m_root.add(window);
                     loop.run();
                     return true;
                 }

                 case 'o':
                 {
                     ui::EventLoop loop(m_root);
                     ui::Window window("Test window", m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                     ui::widgets::OptionGrid g(100, 100, m_root);
                     g.addItem(1, 'x', "First item");
                     g.addItem(2, 'y', "Second item");
                     g.addItem(3, 'z', "Third item");
                     g.findItem(2).setEnabled(false);
                     g.findItem(3).setFont(gfx::FontRequest().addWeight(1));
                     g.findItem(1).setValue("one");
                     g.findItem(2).setValue("two");
                     g.findItem(3).setValue("three");
                     g.sig_click.add(printInt);
                     window.add(g);

                     ui::widgets::Button btn("OK", util::Key_Return, m_root);
                     window.add(btn);
                     btn.sig_fire.addNewClosure(loop.makeStop(1));

                     window.pack();
                     m_root.centerWidget(window);
                     m_root.addChild(window, 0);

                     loop.run();
                     return true;
                 }

                 case 'd':
                    testFileList();
                    return true;

                 case util::Key_Delete:
                    if (m_id == 0) {
                        m_stop = true;
                    }
                    delete this;
                    return true;

                 case util::Key_Left:
                    move(-10, 0);
                    return true;

                 case util::Key_Up:
                    move(0, -10);
                    return true;

                 case util::Key_Right:
                    move(10, 0);
                    return true;

                 case util::Key_Down:
                    move(0, 10);
                    return true;
                }
                if (key == util::Key_F1 + m_id) {
                    m_color = COLORQUAD_FROM_RGBA(std::rand()&255, std::rand()&255, std::rand()&255, 255);
                    requestRedraw();
                }
                return false;
            }
        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
            { return false; }
        void tick()
            {
                m_timer->setInterval(500);
                m_blinkState = !m_blinkState;
                requestRedraw();
            }

        void move(int dx, int dy)
            {
                gfx::Rectangle r = getExtent();
                r.moveBy(gfx::Point(dx, dy));
                setExtent(r);
            }
     private:
        gfx::Color_t m_color;
        ui::Root& m_root;
        bool& m_stop;
        bool m_blinkState;
        afl::base::Ref<gfx::Timer> m_timer;
        int m_id;
    };
}

int main(int, char** argv)
{
    try {
        afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
        afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();

        bool stop = false;
        afl::string::NullTranslator tx;
        util::ConsoleLogger log;
        Engine_t engine(log, tx);

        // Configure manager
        ui::res::Manager mgr;
        mgr.addNewImageLoader(new ui::res::EngineImageLoader(engine));
        mgr.addNewImageLoader(new ui::res::CCImageLoader());

        afl::base::Ref<afl::io::Directory> f = fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "resource"));
        mgr.addNewProvider(new ui::res::DirectoryProvider(f, fs, log, tx), "key");

        ui::DefaultResourceProvider provider(mgr, f, engine.dispatcher(), tx, log);

        gfx::WindowParameters param;
        ui::Root root(engine, provider, param);
        mgr.setScreenSize(root.getExtent().getSize());
        root.addChild(*new MyWidget(root, stop, 0), 0);

        ui::widgets::Button btn("Hi there", 'h', root);
        btn.setExtent(gfx::Rectangle(gfx::Point(20, 20), btn.getLayoutInfo().getPreferredSize()));
        root.addChild(btn, 0);
        while (!stop) {
            root.handleEvent();
        }
    }
    catch (afl::except::FileProblemException& e) {
        std::cout << "exception: " << e.getFileName() << ": " << e.what() << "\n";
    }
    catch (std::exception& e) {
        std::cout << "exception: " << e.what() << "\n";
    }
}
