/**
  *  \file game/interface/missioncontext.hpp
  *  \brief Class game::interface::MissionContext
  */
#ifndef C2NG_GAME_INTERFACE_MISSIONCONTEXT_HPP
#define C2NG_GAME_INTERFACE_MISSIONCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/spec/missionlist.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Mission context.
        Publishes a mission definition from a ship list.
        Implements the return value of the "Mission()" function. */
    class MissionContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param slot  Index into MissionList, see game::spec::MissionList::at().
            @param list  Mission list */
        MissionContext(size_t slot, const afl::base::Ref<game::spec::MissionList>& list);

        /** Destructor. */
        ~MissionContext();

        /** Get mission that this context is looking at.
            @return Mission (null if slot out of range) */
        const game::spec::Mission* getMission() const;

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual MissionContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        size_t m_slot;

        // As of 20230404, intentionally not const to allow possible future modifications
        afl::base::Ref<game::spec::MissionList> m_list;
    };

} }

#endif
