/**
  *  \file client/si/keymaphandler.hpp
  *  \brief Class client::si::KeymapHandler
  */
#ifndef C2NG_CLIENT_SI_KEYMAPHANDLER_HPP
#define C2NG_CLIENT_SI_KEYMAPHANDLER_HPP

#include "client/si/control.hpp"
#include "game/proxy/keymapproxy.hpp"
#include "ui/eventloop.hpp"
#include "ui/simplewidget.hpp"
#include "gfx/timer.hpp"
#include "gfx/font.hpp"

namespace client { namespace si {

    /** Keymap Handler.
        This implements the "UseKeymap" command for most cases.
        To use,
        - create object
        - call run()
        - process the result after destroying the KeymapHandler object */
    class KeymapHandler : public ui::SimpleWidget, public Control, private game::proxy::KeymapProxy::Listener {
     public:
        /** Result action.
            These actions are direct results of the inbound process,
            i.e. a StateChange result only appears if the same process/process group that called "UseKeymap" also calls "UI.GotoScreen".
            This is a rare usecase and it would be legitimate to ignore those nested calls,
            but supporting them isn't too hard. */
        enum Action {
            /** No action. Keymap was canceled. */
            NoAction,

            /** Key pressed.
                Call executeCommandWait() with the keymapName, key, and prefix provided in the result. */
            KeyCommand,

            /** State change (UI.GotoScreen).
                Call handleStateChange() with the target, link provided in the result. */
            StateChange,

            /** UI.EndDialog.
                Call handleEndDialog() with the code, link provided in the result. */
            EndDialog,

            /** UI.PopupConsole.
                Call handlePopupConsole() with the link provided in the result. */
            PopupConsole,

            /** UI.ScanKeyboardMode.
                Call handleScanKeyboardMode() with the link provided in the result. */
            ScanKeyboardMode
        };

        /** Result structure. */
        struct Result {
            Action action;                ///< Action.
            util::Key_t key;              ///< Key, for KeyCommand.
            String_t keymapName;          ///< Keymap name, for KeyCommand.
            int prefix;                   ///< Prefix, for KeyCommand.
            OutputState::Target target;   ///< Target state, for StateChange.
            int code;                     ///< Exit code, for EndDialog.
            RequestLink2 link;            ///< Process link, for StateChange/EndDialog/PopupConsole.

            Result()
                : action(NoAction), key(), keymapName(), prefix(), target(), code(), link()
                { }
        };

        /** Constructor.
            \param parentControl  Parent control
            \param name           Name of keymap
            \param prefix         Prefix argument */
        KeymapHandler(Control& parentControl, String_t name, int prefix);
        ~KeymapHandler();

        /** Run.
            \param link  Inbound process (which called UseKeymap)
            \return Result action to execute. */
        Result run(RequestLink2 link);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        // Control:
        virtual void handleStateChange(RequestLink2 link, OutputState::Target target);
        virtual void handleEndDialog(RequestLink2 link, int code);
        virtual void handlePopupConsole(RequestLink2 link);
        virtual void handleScanKeyboardMode(RequestLink2 link);
        virtual void handleSetView(RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(RequestLink2 link, String_t text);
        virtual ContextProvider* createContextProvider();

     private:
        // KeymapProxy::Listener:
        virtual void updateKeyList(util::KeySet_t& keys);

        void show();
        void requestKeys();

        /** Parent control. */
        Control& m_parentControl;

        /** Timer. */
        afl::base::Ref<gfx::Timer> m_timer;

        /** Current keymap name. */
        String_t m_keymapName;

        /** Prefix argument. */
        int m_prefix;

        /** Keymap proxy to access key set. */
        game::proxy::KeymapProxy m_proxy;

        /** Set of bound keys. */
        util::KeySet_t m_keys;

        /** true if popup is visible. */
        bool m_shown;

        /** Dialog result. */
        Result m_result;

        /** Event loop. */
        ui::EventLoop m_loop;

        String_t getText();
        afl::base::Ref<gfx::Font> getFont();
    };

} }

#endif
