/**
  *  \file util/constantanswerprovider.hpp
  */
#ifndef C2NG_UTIL_CONSTANTANSWERPROVIDER_HPP
#define C2NG_UTIL_CONSTANTANSWERPROVIDER_HPP

#include "util/answerprovider.hpp"

namespace util {

    /** Implementation of AnswerProvider to provide constant answers. */
    class ConstantAnswerProvider : public AnswerProvider {
     public:
        ConstantAnswerProvider(bool answer)
            : m_answer(answer)
            { }

        virtual Result ask(int questionId, String_t question);

        /** Constant answer provider that always answers affirmative. */
        static ConstantAnswerProvider sayYes;

        /** Constant answer provider that always answers negative. */
        static ConstantAnswerProvider sayNo;

     private:
        const bool m_answer;
    };

    
}

#endif
