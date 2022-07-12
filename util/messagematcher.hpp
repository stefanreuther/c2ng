/**
  *  \file util/messagematcher.hpp
  *  \brief Class util::MessageMatcher
  */
#ifndef C2NG_UTIL_MESSAGEMATCHER_HPP
#define C2NG_UTIL_MESSAGEMATCHER_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/filenamepattern.hpp"

namespace util {

    /** Configurable log message classifier.
        This class can match messages according to their channel name and severity using rules specified as a string.

        The configuration string consists of a colon-separated list of rules.
        Each rule consists of
        - a channel name wildcard ("game*")
        - an optional level restriction: "@Info" for just Info level, "@Info+" for Info and higher, "@-Info" for Info and lower
        - an action "=action"

        If multiple rules match, the first one will be taken.

        For example, "*@Info+=show:*@Trace=drop:*=hide" will produce the action "show" for all messages of level Info or higher,
        "drop" for Trace, and "hide" for everything else.

        The action is not interpreted by MessageMatcher; its meaning is defined by the caller. */
    class MessageMatcher {
     public:
        /** Constructor.
            Makes a blank MessageMatcher that will not match anything. */
        MessageMatcher();

        /** Destructor. */
        ~MessageMatcher();

        /** Set configuration.
            Replaces this MessageMatcher's ruleset by the one given in \c value.
            \param value New ruleset. See class description.
            \param tx Translator (for error messages)
            \throw std::runtime_error on syntax errors */
        void setConfiguration(const String_t& value, afl::string::Translator& tx);

        /** Match message.
            \param msg [in] Message to check
            \param result [out] Result
            \retval true Matching rule was found, result has been produced
            \retval false No matching rule, result unchanged */
        bool match(const afl::sys::LogListener::Message& msg, String_t& result);

     private:
        typedef afl::bits::SmallSet<afl::sys::LogListener::Level> LevelSet_t;
        struct Rule {
            LevelSet_t levels;
            FileNamePattern namePattern;
            String_t result;
            Rule(LevelSet_t levels, const String_t& namePattern, const String_t& result)
                : levels(levels), namePattern(namePattern), result(result)
                { }
        };
        std::vector<Rule> m_rules;
    };

}

#endif
