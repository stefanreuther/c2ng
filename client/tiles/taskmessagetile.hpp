/**
  *  \file client/tiles/taskmessagetile.hpp
  *  \brief Class client::tiles::TaskMessageTile
  */
#ifndef C2NG_CLIENT_TILES_TASKMESSAGETILE_HPP
#define C2NG_CLIENT_TILES_TASKMESSAGETILE_HPP

#include "afl/base/deleter.hpp"
#include "afl/string/translator.hpp"
#include "game/proxy/taskeditorproxy.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/cardgroup.hpp"
#include "ui/group.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/root.hpp"
#include "ui/widgets/button.hpp"

namespace client { namespace tiles {

    /** Task Editor Message/Status Tile.
        This is a base class for a compound-widget tile that displays
        - a list of buttons to manipulate the auto-task (command-part);
        - either some task status (status-part), or a notification message.

        To use, derive a class:
        - populate commandPart()
        - populate statusPart()
        - connect TaskEditorProxy::sig_messageChange to setMessageStatus

        To populate a part, just add widgets.
        You can use deleter() to control their lifetime.
        Use addCommandButton() as short-cut. */
    class TaskMessageTile : public ui::Group {
     public:
        /** Constructor.
            @param root        UI root
            @param keyHandler  Key handler to process button events
            @param tx          Translator */
        TaskMessageTile(ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx);

        /** Destructor. */
        ~TaskMessageTile();

        /** Access command-part of the widget.
            @return group */
        ui::Group& commandPart();

        /** Access status-part of the widget.
            @return group */
        ui::Group& statusPart();

        /** Set notification message status.
            This reconfigures the widget to show the message if needed.
            @param st Status */
        void setMessageStatus(const game::proxy::TaskEditorProxy::MessageStatus& st);

        /** Add command button.
            This is a short-cut to adding a button to commandPart().
            The button will produce the given key event as callback to the keyHandler.
            @param key   Key
            @param label Label */
        void addCommandButton(util::Key_t key, String_t label);

        /** Access UI root.
            @return root */
        ui::Root& root();

        /** Access deleter.
            This deleter can be used to control lifetime of child widgets.
            @return deleter */
        afl::base::Deleter& deleter();

        /** Access translator.
            @return translator */
        afl::string::Translator& translator();

     private:
        afl::base::Deleter m_deleter;
        afl::string::Translator& m_translator;
        gfx::KeyEventConsumer& m_keyHandler;
        ui::Root& m_root;
        ui::Group m_commandPart;
        ui::Group m_statusPart;
        ui::Group m_messagePart;
        ui::CardGroup m_cards;
        ui::rich::DocumentView m_messageView;
        ui::widgets::Button m_confirmButton;
    };

} }

#endif
