/**
  *  \file game/interface/missioncontext.hpp
  *  \brief Class game::interface::MissionContext
  */
#ifndef C2NG_GAME_INTERFACE_MISSIONCONTEXT_HPP
#define C2NG_GAME_INTERFACE_MISSIONCONTEXT_HPP

#include "afl/base/ref.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Mission context.
        Publishes a mission definition from a ship list.
        Implements the return value of the "Mission()" function. */
    class MissionContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param slot      Index into ShipList's MissionList, see game::spec::MissionList::at().
            @param shipList  Ship list */
        MissionContext(size_t slot, afl::base::Ref<game::spec::ShipList> shipList);

        /** Destructor. */
        ~MissionContext();

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
        afl::base::Ref<game::spec::ShipList> m_shipList;
    };

} }

#endif
