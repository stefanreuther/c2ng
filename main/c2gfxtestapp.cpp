/**
  *  \file main/c2gfxtestapp.cpp
  *  \brief c2testapp - Graphical test applets
  */

#include "afl/io/filesystem.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/environment.hpp"
#include "client/widgets/testapplet.hpp"
#include "gfx/applet.hpp"
#include "gfx/threed/modelapplet.hpp"
#include "gfx/threed/renderapplet.hpp"
#include "ui/widgets/testapplet.hpp"

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::sys::Dialog& dlg = afl::sys::Dialog::getSystemInstance();

    return gfx::Applet::Runner(dlg, env, fs, "PCC2 Test Applets")
        .addNew("client-alliance-list",    "Client widget test: alliance status list",      client::widgets::TestApplet::makeAllianceStatusList())
        .addNew("client-file-list",        "Client widget test: file list",                 client::widgets::TestApplet::makeFileList())
        .addNew("client-player-list",      "Client widget test: player list, normal",       client::widgets::TestApplet::makePlayerList(false))
        .addNew("client-player-list-flow", "Client widget test: player list, flow",         client::widgets::TestApplet::makePlayerList(true))
        .addNew("client-reference-list",   "Client widget test: reference list",            client::widgets::TestApplet::makeReferenceList())
        .addNew("ui-button",               "UI widget test: button",                        ui::widgets::TestApplet::makeButton())
        .addNew("ui-cards",                "UI widget test: cards",                         ui::widgets::TestApplet::makeCards())
        .addNew("ui-checkbox",             "UI widget test: checkbox",                      ui::widgets::TestApplet::makeCheckbox())
        .addNew("ui-checkboxlist-multi",   "UI widget test: checkbox listbox, multi-line",  ui::widgets::TestApplet::makeCheckboxListbox(true))
        .addNew("ui-checkboxlist-single",  "UI widget test: checkbox listbox, single-line", ui::widgets::TestApplet::makeCheckboxListbox(false))
        .addNew("ui-clip",                 "UI clipping test",                              ui::widgets::TestApplet::makeClip())
        .addNew("ui-editor",               "UI widget test: editor",                        ui::widgets::TestApplet::makeEditor())
        .addNew("ui-frames",               "UI widget test: frames",                        ui::widgets::TestApplet::makeFrames())
        .addNew("ui-icongrid",             "UI widget test: icon grid",                     ui::widgets::TestApplet::makeIconGrid())
        .addNew("ui-input",                "UI widget test: input",                         ui::widgets::TestApplet::makeInput())
        .addNew("ui-layout-forcedgrid",    "UI layout test: forced grid",                   ui::widgets::TestApplet::makeLayout(ui::widgets::TestApplet::ForcedGrid))
        .addNew("ui-layout-grid",          "UI layout test: grid",                          ui::widgets::TestApplet::makeLayout(ui::widgets::TestApplet::NormalGrid))
        .addNew("ui-layout-leftflow",      "UI layout test: left-aligned flow",             ui::widgets::TestApplet::makeLayout(ui::widgets::TestApplet::LeftFlow))
        .addNew("ui-layout-rightflow",     "UI layout test: right-aligned flow",            ui::widgets::TestApplet::makeLayout(ui::widgets::TestApplet::RightFlow))
        .addNew("ui-optiongrid",           "UI widget test: option grid",                   ui::widgets::TestApplet::makeOptionGrid())
        .addNew("ui-rich-docview",         "UI widget test: rich document",                 ui::widgets::TestApplet::makeRichDocumentView())
        .addNew("ui-rich-list",            "UI widget test: rich list box",                 ui::widgets::TestApplet::makeRichListBox())
        .addNew("ui-simpletable",          "UI widget test: simple table",                  ui::widgets::TestApplet::makeSimpleTable())
        .addNew("ui-string-list",          "UI widget test: string list box",               ui::widgets::TestApplet::makeStringListBox())
        .addNew("ui-tree-list",            "UI widget test: tree list box",                 ui::widgets::TestApplet::makeTreeListBox())
        .addNew("render-model",            "3-D rendering: model file",                     new gfx::threed::ModelApplet())
        .addNew("render-lines",            "3-D rendering: lines",                          new gfx::threed::RenderApplet(gfx::threed::RenderApplet::Lines))
        .addNew("render-triangles",        "3-D rendering: triangles",                      new gfx::threed::RenderApplet(gfx::threed::RenderApplet::Triangles))
        .run();
}
