/**
  *  \file util/doc/summarizingverifier.hpp
  *  \brief Class util::doc::SummarizingVerifier
  */
#ifndef C2NG_UTIL_DOC_SUMMARIZINGVERIFIER_HPP
#define C2NG_UTIL_DOC_SUMMARIZINGVERIFIER_HPP

#include <map>
#include "afl/container/ptrvector.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/translator.hpp"
#include "util/doc/verifier.hpp"

namespace util { namespace doc {

    /** Documentation verifier that combines all messages.
        Messages are grouped by type and info, with one reference node as specimen.
        For example, instead of 100x "dead link to a/b",
        this will report "dead link to a/b at node X, +99 others". */
    class SummarizingVerifier : public Verifier {
     public:
        /** Constructor. */
        SummarizingVerifier();

        /** Destructor. */
        ~SummarizingVerifier();

        /** Check presence of a message.
            Call after verify().
            @param msg Message
            @return true if at least one message of the given type was produced. */
        bool hasMessage(Message msg) const;

        /** Print message to standard output in human-readable form.
            @param msg Message
            @param idx Index
            @param brief true: Show just the message; false: also include node names
            @param tx  Translator
            @param out Output */
        void printMessage(Message msg, const Index& idx, bool brief, afl::string::Translator& tx, afl::io::TextWriter& out) const;

     protected:
        void reportMessage(Message msg, const Index& idx, Index::Handle_t refNode, String_t info);

     private:
        struct MessageInfo {
            Index::Handle_t refNode;
            size_t count;
            MessageInfo(Index::Handle_t refNode, size_t count)
                : refNode(refNode), count(count)
                { }
        };

        typedef std::map<String_t, MessageInfo> MessageMap_t;

        afl::container::PtrVector<MessageMap_t> m_messages;
    };

} }

#endif
