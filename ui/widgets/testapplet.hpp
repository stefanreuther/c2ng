/**
  *  \file ui/widgets/testapplet.hpp
  *  \brief Class ui::widgets::TestApplet
  */
#ifndef C2NG_UI_WIDGETS_TESTAPPLET_HPP
#define C2NG_UI_WIDGETS_TESTAPPLET_HPP

#include "gfx/applet.hpp"
#include "ui/root.hpp"

namespace ui { namespace widgets {

    /** User interface widget test applet.

        This is the (base class) for a series of applets to test user interface widgets.
        To use, derive a class and implement runTest() with the test code (create widget, add to root, event loop).
        The main entry point contains the UI Root boilerplate.

        This class implements a few test cases, with static methods to create them. */
    class TestApplet : public gfx::Applet {
     public:
        // Applet:
        virtual int run(gfx::Application& app, gfx::Engine& engine, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::sys::Environment::CommandLine_t& cmdl);

        /** Entry point.
            @param root   UI root
            @param tx     Translator */
        virtual void runTest(ui::Root& root, afl::string::Translator& tx) = 0;

        // For makeLayout
        enum Layout {
            LeftFlow,
            RightFlow,
            ForcedGrid,
            NormalGrid
        };

        // Applet implementations
        static TestApplet* makeClip();
        static TestApplet* makeButton();
        static TestApplet* makeInput();
        static TestApplet* makeIconGrid();
        static TestApplet* makeRichDocumentView();
        static TestApplet* makeRichListBox();
        static TestApplet* makeStringListBox();
        static TestApplet* makeTreeListBox();
        static TestApplet* makeSimpleTable();
        static TestApplet* makeCards();
        static TestApplet* makeLayout(Layout lay);
        static TestApplet* makeEditor();
        static TestApplet* makeCheckboxListbox(bool multiLine);
        static TestApplet* makeCheckbox();
        static TestApplet* makeFrames();
        static TestApplet* makeOptionGrid();

        static void testWidget(ui::Root& root, ui::Widget& w);

     private:
        class MyListbox;
        class MyWidget;
    };

} }

#endif
