/**
  *  \file client/screens/controlscreen.hpp
  */
#ifndef C2NG_CLIENT_SCREENS_CONTROLSCREEN_HPP
#define C2NG_CLIENT_SCREENS_CONTROLSCREEN_HPP

#include "afl/base/deleter.hpp"
#include "client/map/movementoverlay.hpp"
#include "client/map/scanneroverlay.hpp"
#include "client/map/widget.hpp"
#include "client/session.hpp"
#include "client/si/control.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/outputstate.hpp"
#include "client/widgets/scanresult.hpp"
#include "game/map/object.hpp"
#include "game/map/objectcursor.hpp"
#include "ui/eventloop.hpp"
#include "client/screenhistory.hpp"
#include "client/map/minefieldmissionoverlay.hpp"
#include "interpreter/process.hpp"
#include "ui/widgets/panel.hpp"

namespace client { namespace screens {

    class ControlScreen : private client::si::Control {
     public:
        struct Definition {
            client::si::OutputState::Target target;
            ScreenHistory::Type historyType;
            interpreter::Process::ProcessKind taskType;
            const char* layoutName;
            const char* keymapName;
        };
        static const Definition ShipScreen;
        static const Definition PlanetScreen;
        static const Definition BaseScreen;
        static const Definition ShipTaskScreen;
        static const Definition PlanetTaskScreen;
        static const Definition BaseTaskScreen;

        ControlScreen(Session& session, int nr, const Definition& def);

        void run(client::si::InputState& in, client::si::OutputState& out);

     public:
        /* FIXME: as of 20180827, this object is shared between threads but not modified.
           Can we replace it by a by-value, not by-reference object? */
        class State : public afl::base::RefCounted {
         public:
            int screenNumber;
            client::si::OutputState::Target ownTarget;
            interpreter::Process::ProcessKind taskType;
            String_t keymapName;

            State(int screenNumber, client::si::OutputState::Target ownTarget, interpreter::Process::ProcessKind taskType, const String_t& keymapName)
                : screenNumber(screenNumber), ownTarget(ownTarget), taskType(taskType), keymapName(keymapName)
                { }

            game::map::ObjectCursor* getCursor(game::Session& session) const;
            game::map::Object* getObject(game::Session& session) const;
        };

     private:
        class ContextProvider;
        class Updater;
        class Proprietor;
        class ProprietorFromSession;

        Session& m_session;
        int m_number;
        const Definition& m_definition;
        afl::base::Ref<client::screens::ControlScreen::State> m_state;
        afl::base::Deleter m_deleter;
        ui::EventLoop m_loop;
        client::si::OutputState m_outputState;

        // Control:
        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handleSetViewRequest(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymapRequest(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessageRequest(client::si::RequestLink2 link, String_t text);
        virtual client::si::ContextProvider* createContextProvider();

        void setId(game::Id_t id);
        void setPositions(game::map::Point origin, game::map::Point target);
        void clearPositions();
        void onScannerMove(game::map::Point target);
        void onDoubleClick(game::map::Point target);

        ui::widgets::Panel m_panel;
        client::map::Widget m_mapWidget;
        client::map::ScannerOverlay m_scannerOverlay;
        client::map::MovementOverlay m_movementOverlay;
        client::map::MinefieldMissionOverlay m_minefieldOverlay;
        client::widgets::ScanResult m_scanResult;
        game::map::Point m_center;

        // should be last:
        util::RequestReceiver<ControlScreen> m_reply;
        util::RequestSender<Proprietor> m_proprietor;
    };

} }

#endif
