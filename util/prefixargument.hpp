/**
  *  \file util/prefixargument.hpp
  *  \brief Class util::PrefixArgument
  */
#ifndef C2NG_UTIL_PREFIXARGUMENT_HPP
#define C2NG_UTIL_PREFIXARGUMENT_HPP

#include "util/key.hpp"
#include "afl/string/translator.hpp"

namespace util {

    /** Prefix argument common user logic.
        This class processes keys intended for a prefix argument.
        It can be used by widgets that implement prefix arguments. */
    class PrefixArgument {
     public:
        enum Action {
            NotHandled,             ///< This key was not recognized.
            Accepted,               ///< This key was processed and the widget remains active.
            Canceled                ///< This key cancelled the widget.
        };

        /** Constructor.
            \param initialValue Initial prefix argument (the key that initiated input). Should not be zero. */
        explicit PrefixArgument(int initialValue);

        /** Get current text for widget.
            \param tx Translator
            \return text */
        String_t getText(afl::string::Translator& tx) const;

        /** Get current effective value of prefix argument.
            \return value */
        int getValue() const;

        /** Process a key.
            \param key
            \return action. Note that actions only depend on the key, not on the state of the widget.
            If a key is recognized but does not lead to a state change, the result will be Accepted anyway. */
        Action handleKey(Key_t key);

     private:
        /** Operator. */
        enum Operator {
            NoOp,                   ///< No operator specified so far
            MultiplyOp,             ///< '*' operator.
            DivideOp                ///< '/' operator.
        };

        int m_value;                ///< First value. Only value if NoOp.
        int m_secondValue;          ///< Second value for operators. Can be 0 if none entered yet (in this case, the operator is ignored as well).
        Operator m_operator;        ///< Active operator.
    };

}

#endif
