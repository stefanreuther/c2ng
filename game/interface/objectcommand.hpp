/**
  *  \file game/interface/objectcommand.hpp
  *  \brief Class game::interface::ObjectCommand
  */
#ifndef C2NG_GAME_INTERFACE_OBJECTCOMMAND_HPP
#define C2NG_GAME_INTERFACE_OBJECTCOMMAND_HPP

#include "game/game.hpp"
#include "game/map/object.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/procedurevalue.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    /** Object command.

        <b>Lifetime</b>

        Like all interpreter objects, this object does not outlive its session.
        However, it might outlive its game.
        We therefore keep a smart pointer to Game to keep it alive.
        The game object itself must not be ever deleted.

        For objects that can be deleted, we need to refer to the object by name;
        see MinefieldMethod. */
    class ObjectCommand : public interpreter::ProcedureValue {
     public:
        /** Function to call.
            @param session Session
            @param obj     Object
            @param proc    Process
            @param args    Parameters */
        typedef void (*Function_t)(game::Session&, game::map::Object&, interpreter::Process&, interpreter::Arguments&);

        /** Constructor.
            @param session Session
            @param obj     Object
            @param func    Function */
        ObjectCommand(game::Session& session, game::map::Object& obj, Function_t func);

        /** Destructor. */
        ~ObjectCommand();

        // ProcedureValue:
        virtual void call(interpreter::Process& proc, interpreter::Arguments& args);
        virtual ObjectCommand* clone() const;

     private:
        game::Session& m_session;
        game::map::Object& m_object;
        afl::base::Ptr<game::Game> m_game;
        Function_t m_function;
    };

    /** Implementation of "Mark" command, ObjectCommand version.
        @param session Session
        @param obj     Object
        @param proc    Process
        @param args    Parameters */
    void IFObjMark(game::Session& session, game::map::Object& obj, interpreter::Process& proc, interpreter::Arguments& args);

    /** Implementation of "Mark" command, simple version.
        @param obj     Object
        @param args    Parameters */
    void IFObjMark(game::map::Object& obj, interpreter::Arguments& args);

    /** Implementation of "Unmark" command, ObjectCommand version.
        @param session Session
        @param obj     Object
        @param proc    Process
        @param args    Parameters */
    void IFObjUnmark(game::Session& session, game::map::Object& obj, interpreter::Process& proc, interpreter::Arguments& args);

    /** Implementation of "Unmark" command, simple version.
        @param obj     Object
        @param args    Parameters */
    void IFObjUnmark(game::map::Object& obj, interpreter::Arguments& args);

} }

#endif
