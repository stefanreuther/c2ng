/**
  *  \file game/interface/friendlycodecontext.hpp
  *  \brief Class game::interface::FriendlyCodeContext
  */
#ifndef C2NG_GAME_INTERFACE_FRIENDLYCODECONTEXT_HPP
#define C2NG_GAME_INTERFACE_FRIENDLYCODECONTEXT_HPP

#include "afl/base/ref.hpp"
#include "afl/string/translator.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/simplecontext.hpp"

namespace game { namespace interface {

    /** Friendly code context.
        Implements the result of the "FriendlyCode" function, which publishes friendly code definitions.

        @see FriendlyCodeFunction */
    class FriendlyCodeContext : public interpreter::SimpleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            @param slot     Slot in friendly code definitions (game::spec::FriendlyCodeList::at)
            @param root     Root (for player names)
            @param shipList Ship list (for friendly code definitions)
            @param tx       Translator (for player names) */
        FriendlyCodeContext(size_t slot,
                            afl::base::Ref<const Root> root,
                            afl::base::Ref<game::spec::ShipList> shipList,
                            afl::string::Translator& tx);

        /** Destructor. */
        ~FriendlyCodeContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual FriendlyCodeContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        size_t m_slot;
        afl::base::Ref<const Root> m_root;
        afl::base::Ref<game::spec::ShipList> m_shipList;
        afl::string::Translator& m_translator;
    };

} }

#endif
