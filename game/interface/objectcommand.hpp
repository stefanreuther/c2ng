/**
  *  \file game/interface/objectcommand.hpp
  */
#ifndef C2NG_GAME_INTERFACE_OBJECTCOMMAND_HPP
#define C2NG_GAME_INTERFACE_OBJECTCOMMAND_HPP

#include "interpreter/callablevalue.hpp"
#include "game/session.hpp"
#include "game/map/object.hpp"
#include "interpreter/process.hpp"
#include "game/game.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    /** Object command.

        <b>Lifetime</b>

        Like all interpreter objects, this object does not outlive its session.
        However, it might outlive its game.
        We therefore keep a smart pointer to Game to keep it alive.

        FIXME: we must also keep the object alive, see bug #308.
        Alternatively, refer to the object by name somehow. */
    class ObjectCommand : public interpreter::CallableValue {
     public:
        typedef void (*Function_t)(game::Session&, game::map::Object&, interpreter::Process&, interpreter::Arguments&);

        ObjectCommand(game::Session& session, game::map::Object& obj, Function_t func);
        ~ObjectCommand();

        // CallableValue:
        virtual void call(interpreter::Process& proc, afl::data::Segment& args, bool wantResult);
        virtual bool isProcedureCall();
        virtual int32_t getDimension(int32_t which);
        virtual interpreter::Context* makeFirstContext();
        virtual ObjectCommand* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

     private:
        game::Session& m_session;
        game::map::Object& m_object;
        afl::base::Ptr<game::Game> m_game;
        Function_t m_function;
    };

    void IFObjMark(game::Session& session, game::map::Object& obj, interpreter::Process& proc, interpreter::Arguments& args);
    void IFObjUnmark(game::Session& session, game::map::Object& obj, interpreter::Process& proc, interpreter::Arguments& args);

} }

#endif
