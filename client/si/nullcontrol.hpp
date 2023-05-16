/**
  *  \file client/si/nullcontrol.hpp
  *  \brief Class client::si::NullControl
  */
#ifndef C2NG_CLIENT_SI_NULLCONTROL_HPP
#define C2NG_CLIENT_SI_NULLCONTROL_HPP

#include "client/si/control.hpp"

namespace client { namespace si {

    /** Null Control implementation.
        This class fails all user-interface callouts.
        This allows invoking scripts in a top-level context where we don't expect to be responsive,
        but need to do things in the background, e.g., plugin installation. */
    class NullControl : public Control {
     public:
        /** Constructor.
            @param us UserSide instance */
        explicit NullControl(UserSide& us)
            : Control(us)
            { }

        // Control:
        virtual void handleStateChange(RequestLink2 link, OutputState::Target target);
        virtual void handleEndDialog(RequestLink2 link, int code);
        virtual void handlePopupConsole(RequestLink2 link);
        virtual void handleScanKeyboardMode(RequestLink2 link);
        virtual void handleSetView(RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(RequestLink2 link, String_t text);
        virtual afl::base::Optional<game::Id_t> getFocusedObjectId(game::Reference::Type type) const;
        virtual game::interface::ContextProvider* createContextProvider();

     private:
        void fail(RequestLink2 link);
    };


} }

#endif
