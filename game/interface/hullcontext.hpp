/**
  *  \file game/interface/hullcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_HULLCONTEXT_HPP
#define C2NG_GAME_INTERFACE_HULLCONTEXT_HPP

#include "interpreter/context.hpp"
#include "game/spec/shiplist.hpp"
#include "game/root.hpp"
#include "afl/base/ref.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    class HullContext : public interpreter::Context {
     public:
        HullContext(int nr, afl::base::Ref<game::spec::ShipList> shipList, afl::base::Ref<game::Root> root);
        ~HullContext();

        // Context:
        virtual HullContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual HullContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        static HullContext* create(int nr, Session& session);

     private:
        int m_number;
        afl::base::Ref<game::spec::ShipList> m_shipList;
        afl::base::Ref<game::Root> m_root;
    };

} }

#endif
