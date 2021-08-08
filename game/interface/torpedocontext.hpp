/**
  *  \file game/interface/torpedocontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_TORPEDOCONTEXT_HPP
#define C2NG_GAME_INTERFACE_TORPEDOCONTEXT_HPP

#include "interpreter/context.hpp"
#include "game/spec/shiplist.hpp"
#include "game/root.hpp"
#include "afl/base/ref.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class TorpedoContext : public interpreter::Context, public interpreter::Context::PropertyAccessor {
     public:
        TorpedoContext(bool useLauncher, int nr, afl::base::Ref<game::spec::ShipList> shipList, afl::base::Ref<game::Root> root);
        ~TorpedoContext();

        // Context:
        virtual TorpedoContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual TorpedoContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        static TorpedoContext* create(bool useLauncher, int nr, Session& session);

     private:
        const bool m_useLauncher;
        int m_number;
        afl::base::Ref<game::spec::ShipList> m_shipList;
        afl::base::Ref<game::Root> m_root;

        afl::data::Value* getProperty(const game::spec::Weapon& w, PropertyIndex_t index);
    };

} }

#endif
