/**
  *  \file game/interface/friendlycodecontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_FRIENDLYCODECONTEXT_HPP
#define C2NG_GAME_INTERFACE_FRIENDLYCODECONTEXT_HPP

#include "interpreter/context.hpp"
#include "afl/base/ref.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace interface {

    class FriendlyCodeContext : public interpreter::Context {
     public:
        FriendlyCodeContext(size_t slot,
                            afl::base::Ref<Root> root,
                            afl::base::Ref<game::spec::ShipList> shipList);
        ~FriendlyCodeContext();

        // Context:
        virtual FriendlyCodeContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual FriendlyCodeContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

     private:
        size_t m_slot;
        afl::base::Ref<Root> m_root;
        afl::base::Ref<game::spec::ShipList> m_shipList;
    };

} }

#endif
