/**
  *  \file client/map/screen.hpp
  */
#ifndef C2NG_CLIENT_MAP_SCREEN_HPP
#define C2NG_CLIENT_MAP_SCREEN_HPP

#include "afl/base/deleter.hpp"
#include "client/map/callback.hpp"
#include "client/map/location.hpp"
#include "client/map/widget.hpp"
#include "client/si/control.hpp"
#include "client/si/inputstate.hpp"
#include "client/si/outputstate.hpp"
#include "game/map/movementcontroller.hpp"
#include "game/map/point.hpp"
#include "game/proxy/configurationobserverproxy.hpp"
#include "game/proxy/drawingproxy.hpp"
#include "game/proxy/keymapproxy.hpp"
#include "game/proxy/lockproxy.hpp"
#include "game/proxy/maplocationproxy.hpp"
#include "game/proxy/referencelistproxy.hpp"
#include "game/proxy/referenceobserverproxy.hpp"
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
                   private game::proxy::KeymapProxy::Listener,
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

        /* What distance is considered "near" for drawings? */
        static const int NEAR_DISTANCE = 21;

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
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        bool handleMouseRelative(gfx::Point pt, MouseButtons_t pressedButtons);

        // Control:
        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target);
        virtual void handleEndDialog(client::si::RequestLink2 link, int code);
        virtual void handlePopupConsole(client::si::RequestLink2 link);
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link);
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text);
        virtual game::interface::ContextProvider* createContextProvider();

        // Location::Listener:
        virtual void requestObjectList(game::map::Point pos);
        virtual void requestLockObject(game::map::Point pos, game::proxy::LockProxy::Flags_t flags);

        // KeymapProxy:
        virtual void updateKeyList(util::KeySet_t& keys);

        // ColorScheme:
        virtual gfx::Color_t getColor(util::SkinColor::Color index);
        virtual void drawBackground(gfx::Canvas& can, const gfx::Rectangle& area);

        void drawPanel(gfx::Canvas& can, gfx::Rectangle area);
        void drawTiles(gfx::Canvas& can);
        void drawObjectList(gfx::Canvas& can);

        void setNewOverlay(Layer layer, Overlay* pOverlay);
        void removeOverlay(Overlay* pOverlay);
        bool hasOverlay(Layer layer) const;

        void setDrawingTagFilter(util::Atom_t tag, String_t tagName);
        void clearDrawingTagFilter();
        void ensureDrawingTagVisible(const String_t& tagName);
        bool hasDrawingTagFilter() const;
        const afl::base::Optional<util::Atom_t>& getDrawingTagFilter() const;
        const String_t& getDrawingTagFilterName() const;
        void selectNearestVisibleDrawing();
        bool hasVisibleDrawings() const;

        void lockObject(game::proxy::LockProxy::Flags_t flags);
        void browse(game::map::Location::BrowseFlags_t flags);
        bool handleKeymapKey(util::Key_t key, int prefix);
        int getMouseWheelMode() const;

        void run(client::si::InputState& in, client::si::OutputState& out);

        // FIXME: should this be here?
        game::proxy::DrawingProxy& drawingProxy()
            { return m_drawingProxy; }
        game::proxy::MapLocationProxy& locationProxy()
            { return m_locationProxy; }
        util::RequestSender<game::Session> gameSender()
            { return m_gameSender; }
        client::map::Widget& mapWidget()
            { return m_widget; }

        afl::base::Signal<void()> sig_effectTimer;

     private:
        class SharedState;
        class Properties;
        class PropertiesFromSession;

        ui::Root& m_root;
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<Screen> m_replyReceiver;
        client::map::Widget m_widget;
        ui::Group m_tileContainer;           // ex WStarchartTileContainer
        afl::base::Deleter m_tileHolder;
        afl::base::Ref<SharedState> m_sharedState;
        afl::base::Ref<gfx::Timer> m_effectTimer;

        Location m_location;
        int m_locationCycleBreaker;

        game::map::MovementController m_movement;
        gfx::Point m_pendingMovement;
        int m_mouseStickyness;
        int m_mouseWheelMode;

        game::proxy::MapLocationProxy m_locationProxy;
        game::proxy::ReferenceListProxy m_refListProxy;
        game::proxy::KeymapProxy m_keymapProxy;
        game::proxy::ReferenceObserverProxy m_observerProxy;
        game::proxy::DrawingProxy m_drawingProxy;
        game::proxy::LockProxy m_lockProxy;
        game::proxy::ConfigurationObserverProxy m_configProxy;

        util::RequestSender<Properties> m_propertyProxy;

        game::ref::UserList m_refList;
        game::Reference m_currentObject;

        afl::base::Optional<util::Atom_t> m_drawingTagFilter;
        String_t m_drawingTagFilterName;

        String_t m_viewName;
        String_t m_keymapName;
        util::KeySet_t m_keymapKeys;

        client::si::OutputState m_outputState;
        bool m_stopped;

        std::auto_ptr<Overlay> m_overlays[NUM_LAYERS];

        void updateCenter();
        void onLocationResult(game::Reference ref, game::map::Point pt, game::map::Configuration config);
        void onMapConfigChange(game::map::Configuration config);
        void onPositionChange(game::map::Point pt);
        void onBrowseResult(game::Reference ref, game::map::Point pt);
        void onListChange(const game::ref::UserList& list);
        void onListFinish();
        void onLocationChange(game::map::Point pt);
        void onObjectChanged(game::Reference ref);
        void onLockResult(game::map::Point pt);
        void onEffectTimer();
        void onConfigChange(int id, int32_t value);

        void setContextFromObject();
        void setKeymapName(const String_t& name);
        void setViewName(const String_t& name);

        void startPrefixArgument(int initialValue);

        void setTilePosition();
    };

} }

#endif
