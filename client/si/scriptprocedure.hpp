/**
  *  \file client/si/scriptprocedure.hpp
  *  \brief Class client::si::ScriptProcedure
  */
#ifndef C2NG_CLIENT_SI_SCRIPTPROCEDURE_HPP
#define C2NG_CLIENT_SI_SCRIPTPROCEDURE_HPP

#include "interpreter/callablevalue.hpp"
#include "interpreter/arguments.hpp"
#include "client/si/requestlink1.hpp"
#include "game/session.hpp"
#include "afl/base/weaklink.hpp"

namespace client { namespace si {

    class ScriptSide;
    class RequestLink1;

    /** User Interface command.
        User interface commands need a ScriptSide object to be able to talk to the UI.
        This class provides the ScriptSide to a regular function.

        <b>Implementation of the User Interface Commands</b>

        Each user interface command is implemented as a function that takes
        - a game session
        - a ScriptSide
        - a RequestLink1 (required by the ScriptSide to suspend/restart the process in case it has to wait for UI)
        - the arguments

        Each such command can
        - just execute normally like any other command (e.g. detect errors or exit early)
        - post a user-interface task using ScriptSide::postNewTask.
          This will place the invoking process into the Waiting status.

        Since we're dealing with commands only, ScriptProcedure handles the wantResult flag internally;
        implementations need not deal with it.

        <b>Lifetime</b>

        Like all script objects, this one lives in a interpreter::World.
        Because the ScriptSide may die before the World, the ScriptSide has a WeakLink pointing at the ScriptSide.
        After the ScriptSide died, the ScriptProcedure will become inactive
        and fail all requests with a interpreter::Error::contextError(). */
    class ScriptProcedure : public interpreter::CallableValue {
     public:
        /** Constructor.
            \param session     Session
            \param pScriptSide pointer to ScriptSide.
            \param pFunction implementation function; see class description. */
        ScriptProcedure(game::Session& session, ScriptSide* pScriptSide, void (*pFunction)(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args));

        /** Destructor. */
        ~ScriptProcedure();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        // CallableValue:
        virtual void call(interpreter::Process& proc, afl::data::Segment& args, bool want_result);
        virtual bool isProcedureCall() const;
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual ScriptProcedure* clone() const;
     private:
        game::Session& m_session;
        afl::base::WeakLink<ScriptSide> m_pScriptSide;
        void (*m_pFunction)(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    };

} }

#endif
