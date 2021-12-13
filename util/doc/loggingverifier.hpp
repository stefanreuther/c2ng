/**
  *  \file util/doc/loggingverifier.hpp
  *  \brief Class util::doc::LoggingVerifier
  */
#ifndef C2NG_UTIL_DOC_LOGGINGVERIFIER_HPP
#define C2NG_UTIL_DOC_LOGGINGVERIFIER_HPP

#include "util/doc/verifier.hpp"

namespace util { namespace doc {

    /** Documentation verifier that logs all messages.
        Messages are logged as they arrive, in no particular order. */
    class LoggingVerifier : public Verifier {
     public:
        /** Constructor.
            @param tx Translator
            @param out Messages are logged here */
        LoggingVerifier(afl::string::Translator& tx, afl::io::TextWriter& out);

        /** Destructor. */
        ~LoggingVerifier();

     protected:
        void reportMessage(Message msg, const Index& idx, Index::Handle_t refNode, String_t info);

     private:
        afl::string::Translator& m_translator;
        afl::io::TextWriter& m_out;
    };

} }

#endif
