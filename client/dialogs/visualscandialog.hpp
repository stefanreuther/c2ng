/**
  *  \file client/dialogs/visualscandialog.hpp
  *  \brief Class client::dialogs::VisualScanDialog
  */
#ifndef C2NG_CLIENT_DIALOGS_VISUALSCANDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_VISUALSCANDIALOG_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"
#include "client/downlink.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "game/map/point.hpp"
#include "game/ref/list.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Visual Scan Dialog.
        Displays a ship or list of ships.

        To use,
        - build object
        - configure (setTitle etc.)
        - load content (loadCurrent(), loadNext())
        - run(). This produces the dialog result (selected unit)
        - deal with outputState(). The dialog may produce a process and/or context change. */
    class VisualScanDialog {
     public:
        /** Construct.
            \param iface      UserSide
            \param root       UI root
            \param tx         Translator */
        VisualScanDialog(client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx);
        ~VisualScanDialog();

        /** Set dialog title.
            \param title Title */
        void setTitle(String_t title);

        /** Set name of "OK" button.
            \param okName Name */
        void setOkName(String_t okName);

        /** Set acceptance of foreign ships.
            When set to false (default), users can confirm the dialog only for played ships, even if foreign ones are visible.
            \param flag true to accept foreign ships */
        void setAllowForeignShips(bool flag);

        /** Set early-exit flag.
            When set to true, and there is only one choice, accept that directly. Default is false.
            \param flag true to exit early */
        void setEarlyExit(bool flag);

        /** Load current ships.
            \param link        User-interface synchronisation
            \param pos         Map position to look at
            \param options     Options determining what units to list
            \param excludeShip If non-zero, exclude the ship with that Id from the list
            \retval true  List has been prepared; call run() next.
            \retval false No applicable units; user advice has been displayed, do not call run() */
        bool loadCurrent(Downlink& link, game::map::Point pos, game::ref::List::Options_t options, game::Id_t excludeShip);

        /** Load next-turn ships.
            \param link        User-interface synchronisation
            \param pos         Map position to look at
            \param fromShip    If nonzero, use that ship's next position (ignore pos)
            \param options     Options determining what units to list
            \retval true  List has been prepared; call run() next.
            \retval false No applicable units; user advice has been displayed, do not call run() */
        bool loadNext(Downlink& link, game::map::Point pos, game::Id_t fromShip, game::ref::List::Options_t options);

        /** Display dialog.
            Note that you need to process outputState() in any case.
            \return If user confirmed the dialog, reference to the chosen unit (a ship).
            If user cancelled, a null reference (!isSet()). */
        game::Reference run();

        /** Access output state.
            This dialog can produce a context change, e.g. via a ship's messages.
            \return OutputState */
        client::si::OutputState& outputState();

     private:
        struct ShipData;

        class CargoSummaryBuilder;
        class CurrentSummaryBuilder;
        class NextSummaryBuilder;

        class Listener;
        class KeyHandler;
        class ListPeer;
        class SpecPeer;
        class Window;

        client::si::UserSide& m_userSide;
        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        afl::string::Translator& m_translator;
        client::si::OutputState m_outputState;
        String_t m_title;
        String_t m_okName;
        bool m_allowForeignShips;
        bool m_earlyExit;
        bool m_allowRemoteControl;
        bool m_canEarlyExit;
        game::Id_t m_initialShipId;
        game::ref::List m_list;
        std::auto_ptr<CargoSummaryBuilder> m_cargoSummaryBuilder;
    };

} }

#endif
