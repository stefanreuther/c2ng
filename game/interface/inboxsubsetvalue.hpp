/**
  *  \file game/interface/inboxsubsetvalue.hpp
  */
#ifndef C2NG_GAME_INTERFACE_INBOXSUBSETVALUE_HPP
#define C2NG_GAME_INTERFACE_INBOXSUBSETVALUE_HPP

#include <vector>
#include <cstddef>
#include "interpreter/indexablevalue.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace interface {

    /** Value for a subset of inbox ("Messages" property).

        This publishes a subset of InboxContext.
        It uses InboxContext internally.

        This needs to be a separate context instead of a (generalized) InboxContext looking at a SubsetMailbox
        because the Ids it publishes are Ids of the original inbox
        (i.e. Messages(2).Id=17 if InMsg(17).FullText=Messages(2).FullText (not Messages(2).Id=2). */
    class InboxSubsetValue : public interpreter::IndexableValue {
     public:
        InboxSubsetValue(const std::vector<size_t>& indexes,
                         afl::string::Translator& tx,
                         afl::base::Ref<const Root> root,
                         afl::base::Ref<const Game> game);
        ~InboxSubsetValue();

        virtual afl::data::Value* get(interpreter::Arguments& args);
        virtual void set(interpreter::Arguments& args, afl::data::Value* value);
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();
        virtual InboxSubsetValue* clone() const;
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        static InboxSubsetValue* create(const std::vector<size_t>& indexes, afl::string::Translator& tx, afl::base::Ref<const Root> root, afl::base::Ref<const Game> game);

     private:
        std::vector<size_t> m_indexes;
        afl::string::Translator& m_translator;
        afl::base::Ref<const Root> m_root;
        afl::base::Ref<const Game> m_game;
    };

} }

#endif
