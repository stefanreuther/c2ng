/**
  *  \file game/interface/missionfunction.hpp
  *  \brief Class game::interface::MissionFunction
  */
#ifndef C2NG_GAME_INTERFACE_MISSIONFUNCTION_HPP
#define C2NG_GAME_INTERFACE_MISSIONFUNCTION_HPP

#include "game/interface/missioncontext.hpp"
#include "game/session.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Implementation of the "Mission()" function. */
    class MissionFunction : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param session Session */
        explicit MissionFunction(Session& session);

        // IndexableValue:
        virtual MissionContext* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const;
        virtual MissionContext* makeFirstContext();
        virtual MissionFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Session& m_session;
    };

} }

#endif
