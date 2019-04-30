/**
  *  \file client/map/screen.hpp
  */
#ifndef C2NG_CLIENT_MAP_SCREEN_HPP
#define C2NG_CLIENT_MAP_SCREEN_HPP

#include "afl/base/deleter.hpp"
#include "client/map/callback.hpp"
#include "client/map/location.hpp"
#include "client/map/widget.hpp"
#include "client/proxy/keymapproxy.hpp"
#include "client/proxy/maplocationproxy.hpp"
#include "client/proxy/referencelistproxy.hpp"
#include "client/proxy/referenceobserverproxy.hpp"
#include "client/si/control.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/outputstate.hpp"
#include "game/map/point.hpp"
#include "game/session.hpp"
#include "ui/group.hpp"
#include "ui/widget.hpp"
#include "util/requestsender.hpp"

namespace client { namespace map {

    class Overlay;

    /** Starchart screen.
        This ties together a bunch of proxies and a map widget. */
    class Screen : public ui::Widget,
                   public client::si::Control,
                   private Location::Listener,
                   private client::proxy::KeymapProxy::Listener,
                   private gfx::ColorScheme<util::SkinColor::Color>
    {
     public:
        enum Layer {
            BaseLayer,          // Base mode (bottom-most, must be first). ex ChartMode_BaseMode.
            PrimaryLayer,       // Primary mode with own keymap (e.g. marker). ex ChartMode_PrimaryMode.
            PrefixLayer,        // Prefix argument. ex ChartMode_Prefix.
            MessageLayer        // Message (top-most, must be last). ex ChartMode_Message.
        };
        static const size_t NUM_LAYERS = MessageLayer+1;

        Screen(client::si::UserSide& userSide,
               ui::Root& root,
               afl::string::Translator& tx,
               util::RequestSender<game::Session> gameSender);
        ~Screen();

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        bool handleMouseRelative(gfx::Point pt, MouseButtons_t pressedButtons);

        // Control:
        virtual void handleStateChange(client::si::UserSide& ui, client::si::RequestLink2 link, client::si::OutputState::Target /*target*/);
        virtual void handleEndDialog(client::si::UserSide& ui, client::si::RequestLink2 link, int /*code*/);
        virtual void handlePopupConsole(client::si::UserSide& ui, client::si::RequestLink2 link);
        virtual void handleSetViewRequest(client::si::UserSide& ui, client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual client::si::ContextProvider* createContextProvider();

        // Location::Listener:
        virtual void updateObjectList();

        // KeymapProxy:
        virtual void updateKeyList(util::KeySet_t& keys);

        // ColorScheme:
        virtual gfx::Color_t getColor(util::SkinColor::Color index);
        virtual void drawBackground(gfx::Canvas& can, const gfx::Rectangle& area);

        void drawPanel(gfx::Canvas& can, gfx::Rectangle area);
        void drawTiles(gfx::Canvas& can);
        void drawObjectList(gfx::Canvas& can);

        void setNewOverlay(Layer layer, Overlay* pOverlay);

        void run(client::si::InputState& in, client::si::OutputState& out);

     private:
        class SharedState;
        class Properties;

        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<Screen> m_replyReceiver;
        client::map::Widget m_widget;
        ui::Group m_tileContainer;           // ex WStarchartTileContainer
        afl::base::Deleter m_tileHolder;
        afl::base::Ref<SharedState> m_sharedState;

        Location m_location;
        int m_locationCycleBreaker;

        client::proxy::MapLocationProxy m_locationProxy;
        client::proxy::ReferenceListProxy m_refListProxy;
        client::proxy::KeymapProxy m_keymapProxy;
        client::proxy::ReferenceObserverProxy m_observerProxy;

        util::SlaveRequestSender<game::Session, Properties> m_propertyProxy;

        game::ref::UserList m_refList;
        game::Reference m_currentObject;

        String_t m_viewName;
        String_t m_keymapName;
        util::KeySet_t m_keymapKeys;

        client::si::OutputState m_outputState;
        bool m_stopped;

        std::auto_ptr<Overlay> m_overlays[NUM_LAYERS];

        void onLocationResult(game::Reference ref, game::map::Point pt, game::map::Configuration config);
        void onPositionChange(game::map::Point pt);
        void onListChange(const game::ref::UserList& list);
        void onListFinish();
        void onLocationChange(game::map::Point pt);
        void onObjectChanged(game::Reference ref);
        void onLockResult(game::map::Point pt);

        void setContextFromObject();
        void setKeymapName(const String_t& name);
        void setViewName(const String_t& name);

        void startPrefixArgument(int initialValue);

        void setTilePosition();
    };

} }

#endif
