/**
  *  \file util/answerprovider.hpp
  */
#ifndef C2NG_UTIL_ANSWERPROVIDER_HPP
#define C2NG_UTIL_ANSWERPROVIDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"

namespace util {

    /** Interface to provide answers to questions. */
    class AnswerProvider : public afl::base::Deletable {
     public:
        enum Result {
            No,                 // Negative answer / do not process this element.
            Yes,                // Positive answer / do process this element.
            Cancel              // Cancel / do not process this element nor anything that follows.
        };

        /** Answer a question.
            \param questionId Identifier of the question, program interface
            \param question   Question, user interface */
        virtual Result ask(int questionId, String_t question) = 0;
    };

}

#endif
