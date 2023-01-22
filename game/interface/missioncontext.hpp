/**
  *  \file game/interface/missioncontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_MISSIONCONTEXT_HPP
#define C2NG_GAME_INTERFACE_MISSIONCONTEXT_HPP

#include "interpreter/simplecontext.hpp"
#include "afl/base/ref.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace interface {

    class MissionContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        MissionContext(size_t slot,
                       afl::base::Ref<game::spec::ShipList> shipList);
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
        afl::base::Ref<game::spec::ShipList> m_shipList;
    };

} }

#endif
