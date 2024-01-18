/**
  *  \file test/client/widgets/pluginlisttest.cpp
  *  \brief Test for client::widgets::PluginList
  */

#include "client/widgets/pluginlist.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"

using util::plugin::Manager;

/** Test formatSubtitle(). */
AFL_TEST("client.widgets.PluginList:formatSubtitle", a)
{
    afl::string::NullTranslator tx;
    String_t out;
    a.checkEqual("01. formatSubtitle", client::widgets::formatSubtitle(out, Manager::Info("I", "Name", Manager::Loaded), tx), util::SkinColor::Faded);
    a.checkEqual("02. out", out, "(I, loaded)");

    a.checkEqual("11. formatSubtitle", client::widgets::formatSubtitle(out, Manager::Info("OT", "Other", Manager::NotLoaded), tx), util::SkinColor::Red);
    a.checkEqual("12. out", out, "(OT, not loaded)");
}

/** Test content handling. */
AFL_TEST("client.widgets.PluginList:content", a)
{
    afl::string::NullTranslator tx;
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    client::widgets::PluginList list(root, tx);

    // Set first content
    std::vector<Manager::Info> info1;
    info1.push_back(Manager::Info("ONE",   "First",  Manager::Loaded));
    info1.push_back(Manager::Info("TWO",   "Second", Manager::Loaded));
    info1.push_back(Manager::Info("THREE", "Third",  Manager::Loaded));
    info1.push_back(Manager::Info("FOUR",  "Fourth", Manager::Loaded));
    list.setContent(info1);

    // Verify
    a.checkEqual("01. getNumItems", list.getNumItems(), 4U);
    a.checkEqual("02. getCurrentItem", list.getCurrentItem(), 0U);

    // Place on THREE
    list.setCurrentItem(2);
    a.checkNonNull("11. getCurrentPlugin", list.getCurrentPlugin());
    a.checkEqual("12. id", list.getCurrentPlugin()->id, "THREE");
    a.checkDifferent("13. has been copied", list.getCurrentPlugin(), &info1[2]);

    // Update
    std::vector<Manager::Info> info2;
    info2.push_back(Manager::Info("TWO",   "Second", Manager::Loaded));
    info2.push_back(Manager::Info("THREE", "Third",  Manager::Loaded));
    list.setContent(info2);

    // Verify
    a.checkEqual("21. getNumItems", list.getNumItems(), 2U);
    a.checkEqual("22. getCurrentItem", list.getCurrentItem(), 1U);
}
