/**
  *  \file game/interface/vcrsidecontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_VCRSIDECONTEXT_HPP
#define C2NG_GAME_INTERFACE_VCRSIDECONTEXT_HPP

#include "interpreter/simplecontext.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "afl/base/ref.hpp"
#include "game/vcr/battle.hpp"

namespace game { namespace interface {

    class VcrSideContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        VcrSideContext(size_t battleNumber,
                       size_t side,
                       Session& session,
                       afl::base::Ref<Root> root,     // for PlayerList
                       afl::base::Ref<Turn> turn,     // for Turn
                       afl::base::Ref<game::spec::ShipList> shipList);
        ~VcrSideContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual VcrSideContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        game::vcr::Battle* getBattle() const;

        /** Create a VcrSideContext for the current turn.
            \param battleNumber Number of battle (0-based!)
            \param side Side (0-based!)
            \param session Session */
        static VcrSideContext* create(size_t battleNumber, size_t side, Session& session);

     private:
        const size_t m_battleNumber;
        size_t m_side;
        Session& m_session;
        afl::base::Ref<Root> m_root;
        afl::base::Ref<Turn> m_turn;
        afl::base::Ref<game::spec::ShipList> m_shipList;
    };

} }

#endif
