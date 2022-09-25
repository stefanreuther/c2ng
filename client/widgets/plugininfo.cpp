/**
  *  \file client/widgets/plugininfo.cpp
  *  \brief Class client::widgets::PluginInfo
  */

#include "client/widgets/plugininfo.hpp"
#include "client/widgets/pluginlist.hpp"

namespace {
    /* Interval after which widget is cleared, ms */
    const uint32_t DEBOUNCE_INTERVAL = 500;

    void renderListItem(ui::rich::Document& doc, bool& did, const String_t& title, util::rich::Text text)
    {
        if (!did) {
            doc.add(util::rich::Text(title).withStyle(util::rich::StyleAttribute::Bold));
            did = true;
        } else {
            doc.add(", ");
        }
        doc.add(text);
    }

    void renderList(ui::rich::Document& doc, String_t title, const afl::data::StringList_t& good, const afl::data::StringList_t& bad)
    {
        bool did = false;
        for (size_t i = 0; i < good.size(); ++i) {
            renderListItem(doc, did, title, util::rich::Text(good[i]));
        }
        for (size_t i = 0; i < bad.size(); ++i) {
            renderListItem(doc, did, title, util::rich::Text(bad[i]).withColor(util::SkinColor::Red).withStyle(util::rich::StyleAttribute::Underline));
        }
        if (did) {
            doc.addParagraph();
        }
    }
}

client::widgets::PluginInfo::PluginInfo(ui::Root& root, afl::string::Translator& tx)
    : DocumentView(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, 18), 0, root.provider()),
      m_translator(tx),
      m_content("", "", util::plugin::Manager::NotLoaded),
      m_timer(root.engine().createTimer()),
      m_timerRunning(false)
{
    m_timer->sig_fire.add(this, &PluginInfo::onTimer);
}

client::widgets::PluginInfo::~PluginInfo()
{ }

void
client::widgets::PluginInfo::setContent(const util::plugin::Manager::Details& d)
{
    m_content = d;
    if (m_timerRunning) {
        m_timer->setInterval(afl::sys::INFINITE_TIMEOUT);
        m_timerRunning = false;
    }
    render();
}

void
client::widgets::PluginInfo::setLoading()
{
    if (!m_timerRunning) {
        m_timer->setInterval(DEBOUNCE_INTERVAL);
        m_timerRunning = true;
    }
}

void
client::widgets::PluginInfo::render()
{
    // WPluginDialog::onMove()
    ui::rich::Document& doc = getDocument();
    doc.clear();
    if (getExtent().getWidth() != 0 && !m_content.id.empty()) {
        // Header
        doc.add(util::rich::Text(m_content.name).withStyle(util::rich::StyleAttribute::Big));
        doc.addNewline();

        String_t line;
        util::SkinColor::Color color = formatSubtitle(line, m_content, m_translator);
        doc.add(util::rich::Text(line).withColor(color));
        doc.addParagraph();

        if (m_content.description.empty()) {
            doc.add(m_translator("(no description given)"));
        } else {
            String_t desc = m_content.description;
            String_t::size_type n;
            while ((n = desc.find('\n')) != String_t::npos) {
                doc.add(desc.substr(0, n));
                doc.addParagraph();
                desc.erase(0, n+1);
            }
            doc.add(desc);
        }
        doc.addParagraph();

        // Lists
        const afl::data::StringList_t empty;
        renderList(doc, m_translator("Files: "), m_content.files, empty);
        renderList(doc, m_translator("Requires: "), m_content.usedFeatures, m_content.missingFeatures);
        renderList(doc, m_translator("Provides: "), m_content.providedFeatures, empty);
    }
    doc.finish();
    requestRedraw();
}

void
client::widgets::PluginInfo::onTimer()
{
    m_timerRunning = false;
    setContent(util::plugin::Manager::Details("", "", util::plugin::Manager::NotLoaded));
}
