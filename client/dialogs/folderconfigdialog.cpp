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
#include "util/translation.hpp"

namespace {
    struct State {
        bool charsetAvailable;
        String_t charsetId;

        bool finishedAvailable;
        bool finished;

        bool readOnlyAvailable;
        bool readOnly;

        State()
            : charsetAvailable(false), charsetId(),
              finishedAvailable(false), finished(false),
              readOnlyAvailable(false), readOnly(false)
            { }
    };

    /*
     *  InitializeRequest: initialize a State from the current configuration
     */
    class InitializeRequest : public util::Request<game::browser::Session> {
     public:
        InitializeRequest(State& st)
            : m_state(st)
            { }
        virtual void handle(game::browser::Session& session)
            {
                using game::Root;
                using game::config::UserConfiguration;
                if (const game::browser::Browser* p = session.browser().get()) {
                    const Root* root = p->getSelectedRoot().get();
                    const UserConfiguration* config = p->getSelectedConfiguration();
                    if (root != 0 && config != 0) {
                        Root::Actions_t as = root->getPossibleActions();
                        if (as.contains(Root::aConfigureCharset)) {
                            m_state.charsetAvailable = true;
                            m_state.charsetId = (*config)[UserConfiguration::Game_Charset]();
                        }
                        if (as.contains(Root::aConfigureFinished)) {
                            m_state.finishedAvailable = true;
                            m_state.finished = (*config)[UserConfiguration::Game_Finished]();
                        }
                        if (as.contains(Root::aConfigureReadOnly)) {
                            m_state.readOnlyAvailable = true;
                            m_state.readOnly = (*config)[UserConfiguration::Game_ReadOnly]();
                        }
                    }
                }
            }
     private:
        State& m_state;
    };

    /*
     *  UpdateRequest: update configuration from State and write back to file
     */
    class UpdateRequest : public util::Request<game::browser::Session> {
     public:
        explicit UpdateRequest(const State& st)
            : m_state(st)
            { }
        virtual void handle(game::browser::Session& session)
            {
                using game::Root;
                using game::config::UserConfiguration;
                using game::config::StringOption;
                using game::config::IntegerOption;
                using game::config::ConfigurationOption;
                if (game::browser::Browser* p = session.browser().get()) {
                    if (UserConfiguration* config = p->getSelectedConfiguration()) {
                        if (m_state.charsetAvailable) {
                            StringOption& opt = (*config)[UserConfiguration::Game_Charset];
                            opt.set(m_state.charsetId);
                            opt.setSource(ConfigurationOption::Game);
                        }
                        if (m_state.finishedAvailable) {
                            IntegerOption& opt = (*config)[UserConfiguration::Game_Finished];
                            opt.set(m_state.finished);
                            opt.setSource(ConfigurationOption::Game);
                        }
                        if (m_state.readOnlyAvailable) {
                            IntegerOption& opt = (*config)[UserConfiguration::Game_ReadOnly];
                            opt.set(m_state.readOnly);
                            opt.setSource(ConfigurationOption::Game);
                        }
                        p->updateConfiguration();
                    }
                }
            }
     private:
        const State& m_state;
    };
    

    class Dialog {
     public:
        Dialog(ui::Root& root, State& state, afl::string::Translator& tx);

        bool run();

        void updateData();
        void onOptionClick(int id);

     private:
        ui::Root& m_root;
        State& m_state;
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

    // /** Simple list. Displays a StringTable as a list and lets the user select an item.
    //     \param title [in] Dialog title
    //     \param current [in/out] Current value
    //     \param tab [in] String table
    //     \retval true User selected a new item; current was updated
    //     \retval false User canceled */
    bool doList(ui::Root& root, const String_t title, int32_t& current, const afl::functional::StringTable_t& tab)
    {
        // ex client/widgets/expformat.cc:doList
        ui::widgets::StringListbox box(root.provider(), root.colorScheme());
        box.addItems(tab);
        box.setCurrentKey(current);

        if (doStandardDialog(title, String_t(), box, true, root)) {
            return box.getCurrentKey(current);
        } else {
            return false;
        }
    }
}

Dialog::Dialog(ui::Root& root, State& state, afl::string::Translator& tx)
    : m_root(root),
      m_state(state),
      m_grid(0, 0, root),
      m_translator(tx)
{
    // Populate the OptionGrid
    if (m_state.charsetAvailable) {
        m_grid.addItem(ID_CHARSET, 'c', m_translator.translateString("Character set"))
            .addPossibleValue(_("yes"))
            .addPossibleValue(_("no"));
    }
    if (m_state.finishedAvailable) {
        m_grid.addItem(ID_FINISHED, 'f', m_translator.translateString("Game is finished"))
            .addPossibleValue(_("yes"))
            .addPossibleValue(_("no"));
    }
    if (m_state.readOnlyAvailable) {
        m_grid.addItem(ID_READONLY, 'r', m_translator.translateString("Open game read-only"))
            .addPossibleValues(CharsetNames(false, m_translator));
    }
}

bool
Dialog::run()
{
    afl::base::SignalConnection conn(m_grid.sig_click.add(this, &Dialog::onOptionClick));
    updateData();
    return doStandardDialog(m_translator.translateString("Folder Configuration"), String_t(), m_grid, false, m_root);
}

void
Dialog::updateData()
{
    if (m_state.charsetAvailable) {
        String_t name;
        if (m_state.charsetId.empty()) {
            name = _("default");
        } else {
            util::CharsetFactory::Index_t index;
            util::CharsetFactory f;
            if (f.findIndexByKey(m_state.charsetId, index)) {
                name = f.getCharsetName(index, m_translator);
            } else {
                name = m_state.charsetId;
            }
        }
        m_grid.findItem(ID_CHARSET).setValue(name);
    }
    if (m_state.readOnlyAvailable) {
        m_grid.findItem(ID_READONLY).setValue(m_state.readOnly ? _("yes") : _("no"));
    }
    if (m_state.finishedAvailable) {
        m_grid.findItem(ID_FINISHED).setValue(m_state.finished ? _("yes") : _("no"));
    }
}

void
Dialog::onOptionClick(int id)
{
    switch (id) {
     case ID_CHARSET: {
        // Convert name to proper index
        using util::CharsetFactory;
        CharsetFactory::Index_t index = CharsetFactory::LATIN1_INDEX;
        CharsetFactory().findIndexByKey(m_state.charsetId, index);

        // List dialog uses int32_t
        int32_t i = int32_t(index);
        if (doList(m_root, _("Character Set"), i, CharsetNames(true, m_translator))) {
            m_state.charsetId = CharsetFactory().getCharsetKey(CharsetFactory::Index_t(i));
            updateData();
        }
        break;
     }

     case ID_READONLY:
        m_state.readOnly = !m_state.readOnly;
        updateData();
        break;

     case ID_FINISHED:
        m_state.finished = !m_state.finished;
        updateData();
        break;
    }
}

void
client::dialogs::doFolderConfigDialog(ui::Root& root,
                                      util::RequestSender<game::browser::Session> sender,
                                      afl::string::Translator& tx)
{
    // Initialize
    Downlink link(root);
    State state;
    {
        InitializeRequest rq(state);
        link.call(sender, rq);
    }

    // Build dialog
    Dialog dlg(root, state, tx);

    // Run and evaluate
    bool ok = dlg.run();
    if (ok) {
        UpdateRequest rq(state);
        link.call(sender, rq);
    }
}
