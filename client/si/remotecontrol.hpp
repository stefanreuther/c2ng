/**
  *  \file client/si/remotecontrol.hpp
  *  \brief Remote-control related functions
  */
#ifndef C2NG_CLIENT_SI_REMOTECONTROL_HPP
#define C2NG_CLIENT_SI_REMOTECONTROL_HPP

#include "afl/data/value.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "interpreter/arguments.hpp"
#include "ui/draw.hpp"

namespace client { namespace si {

    /*
     *  C++ Interface
     */

    /** Get color for remote-control frame.
        \param session Game session
        \param shipId  Ship Id
        \return color for ship in viewpoint turn (NoFrame if preconditions not satisfied)
        \throw game::Exception if no game/root available
        \see game::actions::RemoteControlAction::getNewState() */
    ui::FrameType getRemoteControlFrameColor(game::Session& session, game::Id_t shipId);

    /** Get question for user "toggle remote control" action.
        \param session Game session
        \param shipId  Ship Id
        \return question; Nothing if preconditions not satisfied
        \throw game::Exception if no game/root available
        \see game::actions::RemoteControlAction::getNewState() */
    afl::base::Optional<String_t> getRemoteControlQuestion(game::Session& session, game::Id_t shipId);

    /** Toggle remote control.
        \param session Game session
        \param shipId  Ship Id
        \throw game::Exception if no game/root available
        \see game::actions::RemoteControlAction::toggleState() */
    void toggleRemoteControl(game::Session& session, game::Id_t shipId);


    /*
     *  Script Interface
     */

    afl::data::Value* IFCCRemoteGetColor(game::Session& session, interpreter::Arguments& args);
    afl::data::Value* IFCCRemoteGetQuestion(game::Session& session, interpreter::Arguments& args);
    void IFCCRemoteToggle(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);
    void IFCCRemoteSet(interpreter::Process& proc, game::Session& session, interpreter::Arguments& args);

} }

#endif
