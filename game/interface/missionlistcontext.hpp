/**
  *  \file game/interface/missionlistcontext.hpp
  *  \brief Class game::interface::MissionListContext
  */
#ifndef C2NG_GAME_INTERFACE_MISSIONLISTCONTEXT_HPP
#define C2NG_GAME_INTERFACE_MISSIONLISTCONTEXT_HPP

#include "game/spec/missionlist.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/singlecontext.hpp"

namespace game { namespace interface {

    /** Mission List context.
        Represents an object of type game::spec::MissionList to the script side. */
    class MissionListContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param data MissionList data */
        MissionListContext(const afl::base::Ref<game::spec::MissionList>& data);
        ~MissionListContext();

        /** Access contained MissionList.
            @return MissionList */
        game::spec::MissionList& missions()
            { return *m_data; }

        // ReadOnlyAccessor:
        virtual afl::data::Value* get(PropertyIndex_t index);

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual MissionListContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        afl::base::Ref<game::spec::MissionList> m_data;
    };

    /** Implementation of MissionList() constructor function.
        @param args Parameters
        @return newly-allocated MissionListContext object. */
    afl::data::Value* IFMissionList(interpreter::Arguments& args);

} }

#endif
