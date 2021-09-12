/**
  *  \file game/interface/vcrcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_VCRCONTEXT_HPP
#define C2NG_GAME_INTERFACE_VCRCONTEXT_HPP

#include "interpreter/context.hpp"
#include "game/session.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace interface {

    class VcrContext : public interpreter::Context, public interpreter::Context::ReadOnlyAccessor {
     public:
        VcrContext(size_t battleNumber,
                   Session& session,
                   afl::base::Ref<Root> root,     // for PlayerList
                   afl::base::Ref<Turn> turn,     // for Turn
                   afl::base::Ref<game::spec::ShipList> shipList);
        ~VcrContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual VcrContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        game::vcr::Battle* getBattle() const;

        /** Create a VcrContext for the current turn.
            \param battleNumber Number of battle (0-based!)
            \param session Session */
        static VcrContext* create(size_t battleNumber, Session& session);

     private:
        size_t m_battleNumber;
        Session& m_session;
        afl::base::Ref<Root> m_root;
        afl::base::Ref<Turn> m_turn;
        afl::base::Ref<game::spec::ShipList> m_shipList;
    };

} }

#endif
