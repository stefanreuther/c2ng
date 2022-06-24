/**
  *  \file client/si/remotecontrol.cpp
  *  \brief Remote-control related functions
  *
  *  This module provides script access to a game::action::RemoteControlAction.
  *  The underlying UI flows are simple so we don't need a full-blown proxy for now.
  *  In addition, access to remote-control related attributes is currently both from script and C++.
  */

#include <memory>
#include "client/si/remotecontrol.hpp"
#include "afl/string/translator.hpp"
#include "client/si/values.hpp"
#include "game/actions/preconditions.hpp"
#include "game/actions/remotecontrolaction.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "interpreter/values.hpp"

using game::actions::RemoteControlAction;
using game::config::HostConfiguration;
using interpreter::makeOptionalStringValue;
using interpreter::makeStringValue;

namespace {
    /* Common argument parsing */
    int32_t getShipId(interpreter::Arguments& args)
    {
        args.checkArgumentCount(1);

        int32_t result = 0;
        interpreter::checkIntegerArg(result, args.getNext());
        return result;
    }

    /* Create RemoteControlAction from parameters.
       Checks all preconditions and creates an action or not. */
    RemoteControlAction* createAction(game::Session& session, game::Id_t shipId)
    {
        // Check configuration
        game::Root& r = game::actions::mustHaveRoot(session);
        if (!r.hostConfiguration()[HostConfiguration::CPEnableRemote]()) {
            return 0;
        }

        // Obtain turn
        game::Game& g = game::actions::mustHaveGame(session);
        game::Turn* t = g.getViewpointTurn().get();
        if (t == 0) {
            return 0;    // should not happen
        }

        // Obtain viewpoint player
        const int playerId = g.getViewpointPlayer();

        // Refuse totally invalid ship Ids
        if (t->universe().ships().get(shipId) == 0) {
            return 0;
        }

        // Create action
        return new RemoteControlAction(*t, shipId, playerId);
    }
}


ui::FrameType
client::si::getRemoteControlFrameColor(game::Session& session, game::Id_t shipId)
{
    // ex getRemoteControlColor, phost.pas::rc_Colors (sort-of)
    ui::FrameType result = ui::NoFrame;

    std::auto_ptr<RemoteControlAction> action(createAction(session, shipId));
    if (action.get()) {
        switch (action->getNewState()) {
         case RemoteControlAction::Forbidden:            result = ui::RedFrame;    break;
         case RemoteControlAction::Normal:               result = ui::NoFrame;     break;
         case RemoteControlAction::RemoteControlled:     result = ui::GreenFrame;  break;
         case RemoteControlAction::Applying:             result = ui::GreenFrame;  break;
         case RemoteControlAction::Dropping:             result = ui::YellowFrame; break;
         case RemoteControlAction::Other:                result = ui::NoFrame;     break;
         case RemoteControlAction::OtherForbidden:       result = ui::RedFrame;    break;
         case RemoteControlAction::OurRemoteControlled:  result = ui::NoFrame;     break;
        }
    }

    return result;
}

afl::base::Optional<String_t>
client::si::getRemoteControlQuestion(game::Session& session, game::Id_t shipId)
{
    // ex phost.pas:NRemoteControl (part)
    std::auto_ptr<RemoteControlAction> action(createAction(session, shipId));
    if (action.get()) {
        afl::string::Translator& tx = session.translator();
        switch (action->getNewState()) {
         case RemoteControlAction::Forbidden:            return tx("Allow remote control of this ship?");
         case RemoteControlAction::Normal:               return tx("Forbid remote control of this ship?");
         case RemoteControlAction::RemoteControlled:     return tx("Drop remote control of this ship?");
         case RemoteControlAction::Applying:             return tx("Cancel request for remote control of this ship?");
         case RemoteControlAction::Dropping:             return tx("Cancel dropping remote control of this ship?");
         case RemoteControlAction::Other:                return tx("Request remote control of this ship?");
         case RemoteControlAction::OtherForbidden:       return tx("Request remote control of this ship?");
         case RemoteControlAction::OurRemoteControlled:  return tx("Forbid remote control of this ship?");
        }
    }
    return afl::base::Nothing;
}

void
client::si::toggleRemoteControl(game::Session& session, game::Id_t shipId)
{
    std::auto_ptr<RemoteControlAction> action(createAction(session, shipId));
    if (action.get() && action->toggleState()) {
        // ok
    }
}

/* @q CC$RemoteGetColor(shipId:Int):Str (Internal)
   @since PCC2 2.40.9 */
afl::data::Value*
client::si::IFCCRemoteGetColor(game::Session& session, interpreter::Arguments& args)
{
    return makeStringValue(formatFrameType(getRemoteControlFrameColor(session, getShipId(args))));
}

/* @q CC$RemoteGetQuestion(shipId:Int):Str (Internal)
   @since PCC2 2.40.9 */
afl::data::Value*
client::si::IFCCRemoteGetQuestion(game::Session& session, interpreter::Arguments& args)
{
    return makeOptionalStringValue(getRemoteControlQuestion(session, getShipId(args)));
}

/* @q CC$RemoteToggle shipId:Int (Internal)
   @since PCC2 2.40.9 */
void
client::si::IFCCRemoteToggle(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    toggleRemoteControl(session, getShipId(args));
}

/* @q CC$RemoteSet shipId:Int, verb:Str (Internal)
   @since PCC2 2.40.13 */
void
client::si::IFCCRemoteSet(interpreter::Process& /*proc*/, game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(2);

    int32_t shipId = 0;
    String_t verb;
    if (!interpreter::checkIntegerArg(shipId, args.getNext()) || !interpreter::checkStringArg(verb, args.getNext())) {
        return;
    }

    game::actions::RemoteControlAction::Verb v;
    if (!RemoteControlAction::parseVerb(verb, v)) {
        throw interpreter::Error("Invalid verb");
    }

    std::auto_ptr<RemoteControlAction> action(createAction(session, shipId));
    if (action.get() == 0 || !action->setState(v)) {
        throw interpreter::Error("Impossible");
    }
}
