/**
  *  \file game/interface/inboxsubsetvalue.hpp
  *  \brief Class game::interface::InboxSubsetValue
  */
#ifndef C2NG_GAME_INTERFACE_INBOXSUBSETVALUE_HPP
#define C2NG_GAME_INTERFACE_INBOXSUBSETVALUE_HPP

#include <vector>
#include <cstddef>
#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "interpreter/indexablevalue.hpp"

namespace game { namespace interface {

    /** Value for a subset of inbox ("Messages" property).

        This publishes a subset of InboxContext.
        It uses InboxContext internally.

        This needs to be a separate context instead of a (generalized) InboxContext looking at a SubsetMailbox
        because the Ids it publishes are Ids of the original inbox
        (i.e. Messages(2).Id=17 if InMsg(17).FullText=Messages(2).FullText (not Messages(2).Id=2).

        To create, usually use InboxSubsetValue::create(). */
    class InboxSubsetValue : public interpreter::IndexableValue {
     public:
        /** Constructor.
            @param indexes   Indexes (0-based)
            @param session   Session
            @param turn      Turn */
        InboxSubsetValue(const std::vector<size_t>& indexes,
                         game::Session& session,
                         const afl::base::Ref<const Turn>& turn);

        /** Destructor. */
        ~InboxSubsetValue();

        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value);
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual InboxSubsetValue* clone() const;
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Constructor.
            @param indexes   Indexes (0-based)
            @param session   Session
            @param turn      Turn
            @return newly-allocated InboxSubsetValue. Null if indexes is empty. */
        static InboxSubsetValue* create(const std::vector<size_t>& indexes,
                                        game::Session& session,
                                        const afl::base::Ref<const Turn>& turn);

     private:
        const std::vector<size_t> m_indexes;
        game::Session& m_session;
        const afl::base::Ref<const Turn> m_turn;
    };

} }

#endif
