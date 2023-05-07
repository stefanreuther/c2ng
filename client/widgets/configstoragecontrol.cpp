/**
  *  \file client/widgets/configstoragecontrol.cpp
  *  \brief Class client::widgets::ConfigStorageControl
  */

#include "client/widgets/configstoragecontrol.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/functional/stringtable.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "util/translation.hpp"

using game::config::ConfigurationEditor;
using game::config::ConfigurationOption;

namespace {
    static const char* const SOURCE_NAMES[] = {
        "",
        N_("(Multiple files)"),
        N_("Default value"),
        N_("System configuration file"),
        N_("User configuration file"),
        N_("Game configuration file"),
    };
    static_assert(ConfigurationEditor::NotStored == 0, "NotStored");
    static_assert(ConfigurationEditor::Mixed == 1,     "Mixed");
    static_assert(ConfigurationEditor::Default == 2,   "Default");
    static_assert(ConfigurationEditor::System == 3,    "System");
    static_assert(ConfigurationEditor::User == 4,      "User");
    static_assert(ConfigurationEditor::Game == 5,      "Game");
}

client::widgets::ConfigStorageControl::ConfigStorageControl(ui::Root& root, afl::string::Translator& tx)
    : Group(ui::layout::HBox::instance5),
      m_root(root),
      m_translator(tx),
      m_button("S", 's', root),
      m_text("", util::SkinColor::Static, gfx::FontRequest(), root.provider()),
      m_spacer(),
      m_source(ConfigurationEditor::NotStored)
{
    // ex WConfigStorageControl::WConfigStorageControl
    init(root);
}

client::widgets::ConfigStorageControl::~ConfigStorageControl()
{ }

void
client::widgets::ConfigStorageControl::setSource(game::config::ConfigurationEditor::Source source)
{
    m_source = source;
    render();
}

void
client::widgets::ConfigStorageControl::init(ui::Root& root)
{
    // In c2ng, this is just an aggregation of widgets using standard behaviour.
    // We therefore need a spacer to eat up excess horizontal space (instead of a custom getLayoutInfo()).
    add(m_button);
    add(m_text);
    add(m_spacer);

    m_button.sig_fire.add(this, &ConfigStorageControl::onButtonClick);
    m_text.setForcedWidth(root.provider().getFont(gfx::FontRequest())->getMaxTextWidth(afl::functional::createStringTable(SOURCE_NAMES).map(m_translator)));

    render();
}

void
client::widgets::ConfigStorageControl::render()
{
    size_t index = m_source;
    if (index < countof(SOURCE_NAMES)) {
        m_text.setText(m_translator(SOURCE_NAMES[index]));
    }
    m_button.setState(DisabledState, m_source == ConfigurationEditor::NotStored);
}

void
client::widgets::ConfigStorageControl::onButtonClick()
{
    // ex WConfigStorageControl::onSelect()
    ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
    list.addItem(ConfigurationOption::User, m_translator(SOURCE_NAMES[ConfigurationEditor::User]));
    list.addItem(ConfigurationOption::Game, m_translator(SOURCE_NAMES[ConfigurationEditor::Game]));
    list.setCurrentKey(m_source == ConfigurationEditor::User ? ConfigurationOption::User : ConfigurationOption::Game);
    list.setPreferredHeight(int(list.getNumItems()));

    ui::EventLoop loop(m_root);
    if (ui::widgets::MenuFrame(ui::layout::HBox::instance0, m_root, loop).doMenu(list, m_button.getExtent().getBottomLeft())) {
        int32_t result;
        if (list.getCurrentKey().get(result)) {
            sig_change.raise(static_cast<ConfigurationOption::Source>(result));
        }
    }
}
