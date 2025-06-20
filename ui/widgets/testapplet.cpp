/**
  *  \file ui/widgets/testapplet.cpp
  *  \brief Class ui::widgets::TestApplet
  */

#include "ui/widgets/testapplet.hpp"

#include "afl/base/observable.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"
#include "gfx/point.hpp"
#include "gfx/rectangle.hpp"
#include "gfx/windowparameters.hpp"
#include "ui/defaultresourceprovider.hpp"
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
#include "ui/res/manager.hpp"
#include "ui/rich/document.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/root.hpp"
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
#include "ui/widgets/treelistbox.hpp"
#include "ui/window.hpp"
#include "util/rich/colorattribute.hpp"
#include "util/rich/linkattribute.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/rich/text.hpp"
#include "util/syntax/keywordtable.hpp"
#include "util/syntax/scripthighlighter.hpp"
#include "afl/sys/standardcommandlineparser.hpp"

using afl::base::Deleter;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::LogListener;
using gfx::ColorQuad_t;
using gfx::Color_t;
using gfx::Point;
using util::SkinColor;
using util::rich::ColorAttribute;
using util::rich::LinkAttribute;
using util::rich::StyleAttribute;
using util::rich::Text;

// Test widget
class ui::widgets::TestApplet::MyListbox : public AbstractListbox {
 public:
    MyListbox()
        : AbstractListbox()
        { }
    virtual ui::layout::Info getLayoutInfo() const
        { return Point(200, 110); }
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
            ColorQuad_t cq[1] = {COLORQUAD_FROM_RGBA(128,0,0,0)};
            Color_t c[1];
            can.encodeColors(cq, c);
            can.drawBar(area, c[0], gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
        }
    virtual void drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
        { }
    virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
        {
            ColorQuad_t cq[2] = {COLORQUAD_FROM_RGBA(0,16*item+20,0,0), COLORQUAD_FROM_RGBA(255,255,255,0)};
            Color_t c[2];
            can.encodeColors(cq, c);
            can.drawBar(area, c[0], gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);

            if (state == FocusedItem || state == ActiveItem) {
                gfx::Context<SkinColor::Color> ctx(can, getColorScheme());
                ctx.setRawColor(c[1]);
                drawRectangle(ctx, area);
            }
        }
};

// Test widget for clip test
class ui::widgets::TestApplet::MyWidget : public SimpleWidget {
 public:
    MyWidget(Root& root, bool& stop, int id)
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
    virtual bool handleMouse(Point /*pt*/, MouseButtons_t /*pressedButtons*/)
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
            r.moveBy(Point(dx, dy));
            setExtent(r);
        }
 private:
    Color_t m_color;
    Root& m_root;
    bool& m_stop;
    bool m_blinkState;
    afl::base::Ref<gfx::Timer> m_timer;
    int m_id;
};

// Test case for UI clipping.
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeClip()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                bool stop = false;
                root.addChild(*new MyWidget(root, stop, 0), 0);
                while (!stop) {
                    root.handleEvent();
                }
            }
    };
    return new Class();
}

// Buttons
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeButton()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                Window window("Test Window", root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                Button btn1("One", '1', root);
                Button btn2("Two", '2', root);
                Button btn3("Three", '3', root);
                InputLine input(100, 40, root);
                MyListbox box;
                window.add(btn1);
                window.add(btn2);
                window.add(btn3);
                window.add(input);
                window.add(box);
                window.pack();
                root.centerWidget(window);
                root.addChild(window, 0);

                EventLoop loop(root);
                btn1.sig_fire.addNewClosure(loop.makeStop(1));
                btn2.sig_fire.addNewClosure(loop.makeStop(2));
                btn3.sig_fire.addNewClosure(loop.makeStop(3));
                loop.run();
            }
    };
    return new Class();
}

// Input widget
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeInput()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& tx)
            {
                InputLine(10, root).
                    setFont(gfx::FontRequest().addSize(1)).
                    setText("hello").
                    doStandardDialog("Input", "Type here:", tx);
            }
    };
    return new Class();
}

// IconGrid widget
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeIconGrid()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                Deleter del;
                Point size(24, 24);
                IconGrid g(root.engine(), size, 10, 10);
                for (int i = 0; i < 256; ++i) {
                    g.addIcon(&del.addNew(new ui::icons::ColorTile(root, size, uint8_t(i))));
                }
                g.setPadding(1);
                testWidget(root, g);
            }
    };
    return new Class();
}

// Rich text document view
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeRichDocumentView()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                 ui::rich::DocumentView view(Point(200, 200), ui::rich::DocumentView::fl_Help, root.provider());
                 ui::rich::Document& doc = view.getDocument();

                 Window window("Test Window", root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                 window.add(view);

                 Button btn("OK", util::Key_Return, root);
                 window.add(btn);
                 window.pack();

                 doc.add("Hello, rich text world");
                 doc.addParagraph();
                 doc.add("This is some rich text. ");
                 doc.add(Text("It can use bold. ").withNewAttribute(new StyleAttribute(StyleAttribute::Bold)));
                 doc.add(Text("Or underline.").withNewAttribute(new StyleAttribute(StyleAttribute::Underline)));
                 doc.add(Text(" Or fixed width. ").withNewAttribute(new StyleAttribute(StyleAttribute::Fixed)));
                 doc.add(Text("Or all of it.").
                         withNewAttribute(new StyleAttribute(StyleAttribute::Fixed)).
                         withNewAttribute(new StyleAttribute(StyleAttribute::Underline)).
                         withNewAttribute(new StyleAttribute(StyleAttribute::Bold)));
                 doc.add(Text(" Even a bigger font.").withNewAttribute(new StyleAttribute(StyleAttribute::Big)));
                 doc.add(Text(" Did I say I can use color?").withNewAttribute(new ColorAttribute(SkinColor::Red)));
                 doc.addParagraph();
                 doc.add(Text("This is text with "));
                 doc.add(Text("a link").withNewAttribute(new LinkAttribute("hu")));
                 doc.add(Text(" and another "));
                 doc.add(Text("link").withNewAttribute(new LinkAttribute("hu")));
                 doc.add(Text("."));
                 doc.addParagraph();
                 doc.add("Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula. Phasellus at purus sed purus cursus iaculis. Suspendisse fermentum. Pellentesque et arcu.");
                 doc.addParagraph();
                 doc.add("Maecenas viverra. In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros, sit amet sagittis nunc mi ac neque.");
                 doc.finish();

                 root.centerWidget(window);
                 root.addChild(window, 0);

                 EventLoop loop(root);
                 btn.sig_fire.addNewClosure(loop.makeStop(1));
                 loop.run();
             }
    };
    return new Class();
}

// Rich text listbox
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeRichListBox()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                RichListbox box(root.provider(), root.colorScheme());
                box.addItem("Plain text", 0, true);
                box.addItem(Text("Bold text").withNewAttribute(new StyleAttribute(StyleAttribute::Bold)), 0, true);
                box.addItem(Text("Bold text").withNewAttribute(new StyleAttribute(StyleAttribute::Bold))
                            + " followed by "
                            + Text("fixed text").withNewAttribute(new StyleAttribute(StyleAttribute::Fixed)), 0, true);
                box.addItem("Maecenas viverra. In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros, sit amet sagittis nunc mi ac neque.", 0, true);
                box.addItem(Text("Large text").withNewAttribute(new StyleAttribute(StyleAttribute::Big)), 0, true);

                Window window("Test Window", root.provider(), root.colorScheme(), ui::BLUE_BLACK_WINDOW, ui::layout::VBox::instance5);
                window.add(box);

                Button btn("OK", util::Key_Return, root);
                window.add(btn);
                window.pack();

                root.centerWidget(window);
                root.addChild(window, 0);

                EventLoop loop(root);
                btn.sig_fire.addNewClosure(loop.makeStop(1));
                loop.run();
            }
    };
    return new Class();
}

// StringListbox
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeStringListBox()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                StringListbox box(root.provider(), root.colorScheme());
                box.addItem(1, "foo");
                box.addItem(2, "bar");
                box.addItem(5, "Maecenas viverra. In consectetuer, lorem eu lobortis egestas, velit odio imperdiet eros, sit amet sagittis nunc mi ac neque.");
                box.addItem(6, "Öhm. nö?");
                box.addItem(3, "baz");
                box.addItem(4, "qux");
                box.setPreferredWidth(30, false);

                Window window("Test Window", root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                window.add(box);

                Button btn("OK", util::Key_Return, root);
                window.add(btn);
                window.pack();

                root.centerWidget(window);
                root.addChild(window, 0);

                EventLoop loop(root);
                btn.sig_fire.addNewClosure(loop.makeStop(1));
                loop.run();
            }
    };
    return new Class();
}

// TreeListbox
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeTreeListBox()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                ui::icons::ColorTile blackTile(root, Point(20, 30), ui::Color_Black);
                ui::icons::ColorTile whiteTile(root, Point(20, 30), ui::Color_White);
                TreeListbox tree(root, 6, 100);
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
                    Handler(TreeListbox& tree)
                        : m_tree(tree)
                        { }
                    virtual void call(int32_t i)
                        { m_tree.setIcon(m_tree.findNodeById(i), 0); }
                 private:
                    TreeListbox& m_tree;
                };
                tree.sig_iconClick.addNewClosure(new Handler(tree));

                ScrollbarContainer cont(tree, root);

                testWidget(root, cont);
            }
    };
    return new Class();
}

// SimpleTable widget
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeSimpleTable()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                SimpleTable t(root, 3, 4);
                t.column(0).subrange(0, 3).setExtraColumns(1);
                t.cell(0, 0).setText("Amount:");
                t.cell(0, 1).setText("Auto-B. Goal:");
                t.cell(0, 2).setText("Maximum:");

                t.column(2).subrange(0, 3).setColor(SkinColor::Green).setTextAlign(gfx::RightAlign, gfx::TopAlign);
                t.cell(2, 0).setText("12");
                t.cell(2, 1).setText("[max]");
                t.cell(2, 2).setText("213");

                t.cell(0, 3).setText("Cost:");
                t.cell(1, 3).setExtraColumns(1).setColor(SkinColor::Green).setText("4 mc + 1 supply").setTextAlign(gfx::RightAlign, gfx::TopAlign);

                testWidget(root, t);
            }
    };
    return new Class();
}

// CardGroup / CardTabBar
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeCards()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                 Deleter del;
                 ui::Group g(ui::layout::VBox::instance5);
                 ui::CardGroup cc;
                 CardTabBar bar(root, cc);
                 for (int i = 0; i < 5; ++i) {
                     Button& btn = del.addNew(new Button(Format("Button %d", i), 'x', root));
                     cc.add(btn);
                     bar.addPage(Format("Page %d", i), 'a' + i, btn);
                 }
                 g.add(bar);
                 g.add(cc);
                 testWidget(root, g);
             }
    };
    return new Class();
}

// Layouts
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeLayout(Layout lay)
{
    class Class : public TestApplet {
     public:
        Class(Layout lay)
            : m_layout(lay)
            { }
        void runTest(Root& root, Translator& /*tx*/)
            {
                Deleter del;
                ui::layout::Manager* layout = 0;
                switch (m_layout) {
                 case RightFlow:
                    layout = &del.addNew(new ui::layout::Flow(3, true));
                    break;
                 case LeftFlow:
                    layout = &del.addNew(new ui::layout::Flow(3, false));
                    break;
                 case ForcedGrid: {
                    ui::layout::Grid& g = del.addNew(new ui::layout::Grid(3));
                    g.setForcedCellSize(100, afl::base::Nothing);
                    layout = &g;
                    break;
                 }
                 case NormalGrid:
                    layout = &del.addNew(new ui::layout::Grid(3));
                    break;
                }
                EventLoop loop(root);
                Window window("Test window", root.provider(), root.colorScheme(), ui::BLUE_WINDOW, *layout);

                window.add(del.addNew(new Button("one", '1', root)));
                window.add(del.addNew(new Button("two", '2', root)));
                window.add(del.addNew(new Button("three", '3', root)));
                window.add(del.addNew(new Button("four", '4', root)));
                window.add(del.addNew(new Button("five", '5', root)));
                window.add(del.addNew(new Button("six", '6', root)));
                window.add(del.addNew(new Button("seeeeeeeven", '7', root)));
                window.add(del.addNew(new Button("eight", '8', root)));
                window.add(del.addNew(new Button("nine", '9', root)));
                window.add(del.addNew(new Button("ten", '0', root)));

                Button& btn = del.addNew(new Button("OK", util::Key_Return, root));
                btn.sig_fire.addNewClosure(loop.makeStop(0));
                window.add(btn);
                window.pack();
                root.centerWidget(window);
                root.addChild(window, 0);

                btn.sig_fire.addNewClosure(loop.makeStop(1));
                loop.run();
            }

     private:
        Layout m_layout;
    };
    return new Class(lay);
}

// Editor widget incl. syntax coloring
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeEditor()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                class Filter : public Editor::CharacterFilter_t {
                 public:
                    bool call(afl::charset::Unichar_t ch)
                        { return (ch >= 32 && ch < 127); }
                };

                Filter f;
                util::editor::Editor ed;
                util::syntax::KeywordTable tab;
                util::syntax::ScriptHighlighter sh(tab);
                Editor edWidget(ed, root);
                ed.setLengthLimit(40);
                edWidget.setPreferredSizeInCells(40, 20);
                edWidget.setFlag(util::editor::AllowCursorAfterEnd, true);
                edWidget.setHighlighter(&sh);
                edWidget.setCharacterFilter(&f);
                testWidget(root, edWidget);
            }
    };
    return new Class();
}

// CheckboxListbox
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeCheckboxListbox(bool multiLine)
{
    class Class : public TestApplet {
     public:
        Class(bool multiLine)
            : m_multiLine(multiLine)
            { }
        void runTest(Root& root, Translator& /*tx*/)
            {
                CheckboxListbox box(root, m_multiLine ? CheckboxListbox::MultiLine : CheckboxListbox::SingleLine);
                box.setItemImageName(box.setItemInfo(box.addItem(1, "label one"), "info one"), "ui.cb0");
                box.setItemImageName(box.setItemInfo(box.addItem(2, "label two"), "info two"), "ui.cb1");
                box.setItemAccessible(box.setItemImageName(box.setItemInfo(box.addItem(3, "label three"), "info three"), "ui.cb0"), false);
                box.setItemImageName(box.setItemInfo(box.addItem(4, "label four"), "info four"), "ui.cb0");

                testWidget(root, box);
            }
     private:
        bool m_multiLine;
    };
    return new Class(multiLine);
}

// Checkbox / RadioButton
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeCheckbox()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                Deleter del;
                EventLoop loop(root);
                Window window("Test window", root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);

                afl::base::Observable<int> value;
                AbstractCheckbox cb1(root, 'a', "an item", Point(20, 20));
                AbstractCheckbox cb2(root, 'b', "better item", Point(20, 20));
                AbstractCheckbox cb3(root, 'c', "crazy item", Point(20, 20));
                AbstractCheckbox cb4(root, 'd', "damned item", Point(20, 20));
                Checkbox cb5(root, 'e', "extra item", value);
                RadioButton rb6(root, 'f', "f?", value, 0);
                RadioButton rb7(root, 'g', "good.", value, 1);
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

                Button btn("OK", util::Key_Return, root);
                window.add(btn);
                window.pack();
                root.centerWidget(window);
                root.addChild(window, 0);

                btn.sig_fire.addNewClosure(loop.makeStop(1));
                loop.run();
            }
    };
    return new Class();
}

// Framed widgets
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeFrames()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                Deleter del;
                EventLoop loop(root);
                Window window("Test window", root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                addFrames(window, del, loop, root, ui::NoFrame);
                addFrames(window, del, loop, root, ui::RedFrame);
                addFrames(window, del, loop, root, ui::YellowFrame);
                addFrames(window, del, loop, root, ui::GreenFrame);
                addFrames(window, del, loop, root, ui::RaisedFrame);
                addFrames(window, del, loop, root, ui::LoweredFrame);
                window.pack();
                root.centerWidget(window);
                root.add(window);
                loop.run();
            }

        static void addFrames(Window& win, Deleter& del, EventLoop& loop, Root& root, ui::FrameType type)
            {
                static const int widths[] = { 0, 1, 1, 2, 2, 3, 5, 10 };
                static const int pads[]   = { 0, 0, 3, 0, 3, 1, 1, 1 };

                ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                for (size_t i = 0; i < sizeof(widths)/sizeof(widths[0]); ++i) {
                    FrameGroup& fg = del.addNew(new FrameGroup(ui::layout::VBox::instance5, root.colorScheme(), type));
                    fg.setFrameWidth(widths[i]);
                    fg.setPadding(pads[i]);
                    Button& btn = del.addNew(new Button("X", ' ', root));
                    btn.sig_fire.addNewClosure(loop.makeStop(1));
                    fg.add(btn);
                    g.add(fg);
                }
                win.add(g);
            }
    };
    return new Class();
}

// OptionGrid
ui::widgets::TestApplet*
ui::widgets::TestApplet::makeOptionGrid()
{
    class Class : public TestApplet {
     public:
        void runTest(Root& root, Translator& /*tx*/)
            {
                EventLoop loop(root);
                Window window("Test window", root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                OptionGrid g(100, 100, root);
                g.addItem(1, 'x', "First item");
                g.addItem(2, 'y', "Second item");
                g.addItem(3, 'z', "Third item");
                g.findItem(2).setEnabled(false);
                g.findItem(3).setFont(gfx::FontRequest().addWeight(1));
                g.findItem(1).setValue("one");
                g.findItem(2).setValue("two");
                g.findItem(3).setValue("three");
                window.add(g);

                Button btn("OK", util::Key_Return, root);
                window.add(btn);
                btn.sig_fire.addNewClosure(loop.makeStop(1));

                window.pack();
                root.centerWidget(window);
                root.addChild(window, 0);

                loop.run();
            }
    };
    return new Class();
}

// Utility: test a single widget. Creates a test window and shows the widget.
void
ui::widgets::TestApplet::testWidget(ui::Root& root, ui::Widget& w)
{
    Deleter del;
    EventLoop loop(root);
    Window window("Test window", root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);

    window.add(w);

    Button& btn = del.addNew(new Button("OK", util::Key_Return, root));
    btn.sig_fire.addNewClosure(loop.makeStop(0));
    window.add(btn);
    window.pack();
    root.centerWidget(window);
    root.addChild(window, 0);
    loop.run();
}

// Main entry point
int
ui::widgets::TestApplet::run(gfx::Application& app, gfx::Engine& engine, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::sys::Environment::CommandLine_t& cmdl)
{
    Translator& tx = app.translator();
    LogListener& log = app.log();

    // Configure manager
    ui::res::Manager mgr;
    mgr.addNewImageLoader(new ui::res::EngineImageLoader(engine));
    mgr.addNewImageLoader(new ui::res::CCImageLoader());

    afl::base::Ref<afl::io::Directory> f = fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "resource"));
    mgr.addNewProvider(new ui::res::DirectoryProvider(f, fs, log, tx), "key");

    DefaultResourceProvider provider(mgr, f, engine.dispatcher(), tx, log);

    gfx::WindowParameters param;
    afl::sys::StandardCommandLineParser cmdlParser(cmdl);
    bool option;
    String_t text;
    while (cmdlParser.getNext(option, text)) {
        if (option && handleWindowParameterOption(param, text, cmdlParser, tx)) {
            // ok
        } else {
            app.dialog().showError(Format("parameter not understood: %s", text), env.getInvocationName());
            return 1;
        }
    }

    Root root(engine, provider, param);
    mgr.setScreenSize(root.getExtent().getSize());

    runTest(root, tx);
    return 0;
}
