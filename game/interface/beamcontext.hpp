/**
  *  \file game/interface/beamcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_BEAMCONTEXT_HPP
#define C2NG_GAME_INTERFACE_BEAMCONTEXT_HPP

#include "interpreter/context.hpp"
#include "game/spec/shiplist.hpp"
#include "game/root.hpp"
#include "afl/base/ref.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class BeamContext : public interpreter::Context, public interpreter::Context::PropertyAccessor {
     public:
        BeamContext(int nr, afl::base::Ref<game::spec::ShipList> shipList, afl::base::Ref<game::Root> root);
        ~BeamContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual BeamContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        static BeamContext* create(int nr, Session& session);

     private:
        int m_number;
        afl::base::Ref<game::spec::ShipList> m_shipList;
        afl::base::Ref<game::Root> m_root;
    };

} }

#endif
