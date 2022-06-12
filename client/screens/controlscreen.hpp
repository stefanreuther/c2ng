/**
  *  \file client/screens/controlscreen.hpp
  */
#ifndef C2NG_CLIENT_SCREENS_CONTROLSCREEN_HPP
#define C2NG_CLIENT_SCREENS_CONTROLSCREEN_HPP

#include "afl/base/deleter.hpp"
#include "client/map/minefieldmissionoverlay.hpp"
#include "client/map/movementoverlay.hpp"
#include "client/map/scanneroverlay.hpp"
#include "client/map/widget.hpp"
#include "client/screenhistory.hpp"
#include "client/si/control.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "client/widgets/scanresult.hpp"
#include "game/map/object.hpp"
#include "game/map/objectcursor.hpp"
#include "game/proxy/fleetproxy.hpp"
#include "game/proxy/taskeditorproxy.hpp"
#include "interpreter/process.hpp"
#include "ui/eventloop.hpp"
#include "ui/widgets/panel.hpp"
#include "client/tiles/historyadaptor.hpp"

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
        static const Definition HistoryScreen;
        static const Definition FleetScreen;
        static const Definition ShipTaskScreen;
        static const Definition PlanetTaskScreen;
        static const Definition BaseTaskScreen;

        ControlScreen(client::si::UserSide& us, int nr, const Definition& def);

        ControlScreen& withTaskEditor(interpreter::Process::ProcessKind kind);

        ControlScreen& withFleetProxy();

        ControlScreen& withHistoryAdaptor();

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

        int m_number;
        const Definition& m_definition;
        afl::base::Ref<client::screens::ControlScreen::State> m_state;
        afl::base::Deleter m_deleter;
        ui::EventLoop m_loop;
        client::si::OutputState m_outputState;

        // Control:
        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text);
        virtual game::interface::ContextProvider* createContextProvider();

        void setId(game::Id_t id);
        void setPositions(game::map::Point origin, game::map::Point target, bool isHyperdriving);
        void setTarget(game::map::Point target);
        void setIsHyperdriving(bool isHyperdriving);
        void clearPositions();
        void onScannerMove(game::map::Point target);
        void onDoubleClick(game::map::Point target);
        void onTaskEditorShipChange(const game::proxy::TaskEditorProxy::ShipStatus& st);
        void onFleetChange();
        void onHistoryTurnChange();

        ui::widgets::Panel m_panel;
        client::map::Widget m_mapWidget;
        client::map::ScannerOverlay m_scannerOverlay;
        client::map::MovementOverlay m_movementOverlay;
        client::map::MinefieldMissionOverlay m_minefieldOverlay;
        client::widgets::ScanResult m_scanResult;
        game::map::Point m_center;

        std::auto_ptr<game::proxy::TaskEditorProxy> m_taskEditorProxy;
        interpreter::Process::ProcessKind m_taskKind;
        std::auto_ptr<game::proxy::FleetProxy> m_fleetProxy;
        std::auto_ptr<client::tiles::HistoryAdaptor> m_historyAdaptor;

        // should be last:
        util::RequestReceiver<ControlScreen> m_reply;
        util::RequestSender<Proprietor> m_proprietor;
    };

} }

#endif
