/**
  *  \file client/dialogs/folderconfigdialog.cpp
  */

#include "client/dialogs/folderconfigdialog.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/functional/stringtable.hpp"
#include "client/downlink.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "util/charsetfactory.hpp"

namespace {
    typedef game::proxy::BrowserProxy::Configuration State_t;

    class Dialog {
     public:
        Dialog(ui::Root& root, State_t& state, afl::string::Translator& tx);

        bool run();

        void updateData();
        void onOptionClick(int id);

     private:
        ui::Root& m_root;
        State_t& m_state;
        ui::widgets::OptionGrid m_grid;
        afl::string::Translator& m_translator;
    };

    enum {
        ID_CHARSET,
        ID_FINISHED,
        ID_READONLY
    };

    // ex client/widgets/expformat.cc:CharsetNames
    // FIXME: move to a generic place?
    class CharsetNames : public afl::functional::StringTable_t {
     public:
        CharsetNames(bool longStyle, afl::string::Translator& tx)
            : m_longStyle(longStyle),
              m_translator(tx)
            { }
        virtual String_t get(int32_t a) const
            {
                return m_longStyle
                    ? util::CharsetFactory().getCharsetDescription(a, m_translator)
                    : util::CharsetFactory().getCharsetName(a, m_translator);
            }
        virtual bool getFirstKey(int32_t& a) const
            {
                a = 0;
                return a < int32_t(util::CharsetFactory().getNumCharsets());
            }
        virtual bool getNextKey(int32_t& a) const
            {
                ++a;
                return a < int32_t(util::CharsetFactory().getNumCharsets());
            }
     private:
        bool m_longStyle;
        afl::string::Translator& m_translator;
    };

    bool doList(ui::Root& root, afl::string::Translator& tx, const String_t title, int32_t& current, const afl::functional::StringTable_t& tab)
    {
        // ex client/widgets/expformat.cc:doList
        ui::widgets::StringListbox box(root.provider(), root.colorScheme());
        box.addItems(tab);
        box.setCurrentKey(current);

        if (doStandardDialog(title, String_t(), box, true, root, tx)) {
            return box.getCurrentKey().get(current);
        } else {
            return false;
        }
    }
}

Dialog::Dialog(ui::Root& root, State_t& state, afl::string::Translator& tx)
    : m_root(root),
      m_state(state),
      m_grid(0, 0, root),
      m_translator(tx)
{
    // Populate the OptionGrid
    if (m_state.charsetId.isValid()) {
        m_grid.addItem(ID_CHARSET, 'c', tx("Character set"))
            .addPossibleValue(tx("yes"))
            .addPossibleValue(tx("no"));
    }
    if (m_state.finished.isValid()) {
        m_grid.addItem(ID_FINISHED, 'f', tx("Game is finished"))
            .addPossibleValue(tx("yes"))
            .addPossibleValue(tx("no"));
    }
    if (m_state.readOnly.isValid()) {
        m_grid.addItem(ID_READONLY, 'r', tx("Open game read-only"))
            .addPossibleValues(CharsetNames(false, tx));
    }
}

bool
Dialog::run()
{
    afl::base::SignalConnection conn(m_grid.sig_click.add(this, &Dialog::onOptionClick));
    updateData();
    return doStandardDialog(m_translator("Folder Configuration"), String_t(), m_grid, false, m_root, m_translator);
}

void
Dialog::updateData()
{
    afl::string::Translator& tx = m_translator;
    if (const String_t* p = m_state.charsetId.get()) {
        String_t name;
        if (p->empty()) {
            name = tx("default");
        } else {
            util::CharsetFactory::Index_t index;
            util::CharsetFactory f;
            if (f.findIndexByKey(*p).get(index)) {
                name = f.getCharsetName(index, m_translator);
            } else {
                name = *p;
            }
        }
        m_grid.findItem(ID_CHARSET).setValue(name);
    }
    if (const bool* p = m_state.readOnly.get()) {
        m_grid.findItem(ID_READONLY).setValue(*p ? tx("yes") : tx("no"));
    }
    if (const bool* p = m_state.finished.get()) {
        m_grid.findItem(ID_FINISHED).setValue(*p ? tx("yes") : tx("no"));
    }
}

void
Dialog::onOptionClick(int id)
{
    switch (id) {
     case ID_CHARSET: {
        // Convert name to proper index
        using util::CharsetFactory;
        const CharsetFactory::Index_t index = CharsetFactory().findIndexByKey(m_state.charsetId.orElse("")).orElse(CharsetFactory::LATIN1_INDEX);

        // List dialog uses int32_t
        int32_t i = int32_t(index);
        if (doList(m_root, m_translator, m_translator("Character Set"), i, CharsetNames(true, m_translator))) {
            m_state.charsetId = CharsetFactory().getCharsetKey(CharsetFactory::Index_t(i));
            updateData();
        }
        break;
     }

     case ID_READONLY:
        m_state.readOnly = !m_state.readOnly.orElse(false);
        updateData();
        break;

     case ID_FINISHED:
        m_state.finished = !m_state.finished.orElse(false);
        updateData();
        break;
    }
}

void
client::dialogs::doFolderConfigDialog(ui::Root& root, game::proxy::BrowserProxy& proxy, afl::string::Translator& tx)
{
    // Initialize
    Downlink link(root, tx);
    State_t state;
    proxy.getConfiguration(link, state);

    // Build dialog
    Dialog dlg(root, state, tx);

    // Run and evaluate
    bool ok = dlg.run();
    if (ok) {
        proxy.setConfiguration(link, state);
    }
}
