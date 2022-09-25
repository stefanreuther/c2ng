/**
  *  \file u/t_client_widgets_pluginlist.cpp
  *  \brief Test for client::widgets::PluginList
  */

#include "client/widgets/pluginlist.hpp"

#include "t_client_widgets.hpp"
#include "afl/string/nulltranslator.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"

using util::plugin::Manager;

/** Test formatSubtitle(). */
void
TestClientWidgetsPluginList::testFormat()
{
    afl::string::NullTranslator tx;
    String_t out;
    TS_ASSERT_EQUALS(client::widgets::formatSubtitle(out, Manager::Info("I", "Name", Manager::Loaded), tx), util::SkinColor::Faded);
    TS_ASSERT_EQUALS(out, "(I, loaded)");

    TS_ASSERT_EQUALS(client::widgets::formatSubtitle(out, Manager::Info("OT", "Other", Manager::NotLoaded), tx), util::SkinColor::Red);
    TS_ASSERT_EQUALS(out, "(OT, not loaded)");
}

/** Test content handling. */
void
TestClientWidgetsPluginList::testContent()
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
    TS_ASSERT_EQUALS(list.getNumItems(), 4U);
    TS_ASSERT_EQUALS(list.getCurrentItem(), 0U);

    // Place on THREE
    list.setCurrentItem(2);
    TS_ASSERT(list.getCurrentPlugin() != 0);
    TS_ASSERT_EQUALS(list.getCurrentPlugin()->id, "THREE");
    TS_ASSERT_DIFFERS(list.getCurrentPlugin(), &info1[2]);

    // Update
    std::vector<Manager::Info> info2;
    info2.push_back(Manager::Info("TWO",   "Second", Manager::Loaded));
    info2.push_back(Manager::Info("THREE", "Third",  Manager::Loaded));
    list.setContent(info2);

    // Verify
    TS_ASSERT_EQUALS(list.getNumItems(), 2U);
    TS_ASSERT_EQUALS(list.getCurrentItem(), 1U);
}

