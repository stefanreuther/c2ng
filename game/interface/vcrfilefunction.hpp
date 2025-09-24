/**
  *  \file game/interface/vcrfilefunction.hpp
  *  \brief Class game::interface::VcrFileFunction
  */
#ifndef C2NG_GAME_INTERFACE_VCRFILEFUNCTION_HPP
#define C2NG_GAME_INTERFACE_VCRFILEFUNCTION_HPP

#include "afl/base/ref.hpp"
#include "game/interface/vcrcontext.hpp"
#include "game/vcr/database.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the result of the "VcrFile()" function. */
    class VcrFileFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        static VcrFileFunction* create(Session& session, afl::base::Ref<game::vcr::Database> db);

        /** Access battles.
            @return Battles */
        afl::base::Ref<game::vcr::Database>& battles();

        // IndexableValue:
        virtual VcrContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);

        // CallableValue:
        virtual size_t getDimension(size_t which) const;
        virtual VcrContext* makeFirstContext();
        virtual VcrFileFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        VcrFileFunction(Session& session, const afl::base::Ref<game::vcr::Database>& db);
        size_t getNumBattles() const;

        Session& m_session;
        afl::base::Ref<game::vcr::Database> m_battles;
    };

    /** Implementation of the VcrFile() function.
        @param session Session
        @param args Arguments
        @return VcrFileFunction object */
    afl::data::Value* IFVcrFile(game::Session& session, interpreter::Arguments& args);

} }

inline afl::base::Ref<game::vcr::Database>&
game::interface::VcrFileFunction::battles()
{
    return m_battles;
}

#endif
