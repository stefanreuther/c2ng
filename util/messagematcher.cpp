/**
  *  \file util/messagematcher.cpp
  *  \brief Class util::MessageMatcher
  */

#include "util/messagematcher.hpp"
#include "util/stringparser.hpp"

// Constructor.
util::MessageMatcher::MessageMatcher()
    : m_rules()
{ }

// Destructor.
util::MessageMatcher::~MessageMatcher()
{ }

// Set configuration.
void
util::MessageMatcher::setConfiguration(const String_t& value, afl::string::Translator& tx)
{
    using afl::sys::LogListener;

    // Room for new rules
    std::vector<Rule> rules;

    // Colon-separated rules
    StringParser p(value);
    while (!p.parseEnd()) {
        /*
          Syntax is:
              wildcard[@level]=action
        */

        // Wildcard
        String_t wildcard;
        p.parseDelim("@=:", wildcard);

        // Level
        LevelSet_t levels = LevelSet_t::allUpTo(LogListener::Error);
        if (p.parseCharacter('@')) {
            // "-level" syntax
            bool below = p.parseCharacter('-');

            // Level
            LogListener::Level level;
            if (p.parseString("Trace")) {
                level = LogListener::Trace;
            } else if (p.parseString("Debug")) {
                level = LogListener::Debug;
            } else if (p.parseString("Info")) {
                level = LogListener::Info;
            } else if (p.parseString("Warn")) {
                level = LogListener::Warn;
            } else if (p.parseString("Error")) {
                level = LogListener::Error;
            } else {
                throw std::runtime_error(tx("Invalid log level in message match expression"));
            }

            // "level+" syntax
            bool above = p.parseCharacter('+');

            // Adjust
            if (above) {
                if (below) {
                    // "-foo+": interpret as "everything"
                } else {
                    // "foo+": given level and higher
                    levels -= LevelSet_t::allUpTo(level);
                    levels += level;
                }
            } else {
                if (below) {
                    // "-foo": given level and below
                    levels = LevelSet_t::allUpTo(level);
                } else {
                    // "foo": just one level
                    levels = LevelSet_t(level);
                }
            }
        }

        // Action
        if (!p.parseCharacter('=')) {
            throw std::runtime_error(tx("Missing '=' in message match expression"));
        }
        String_t action;
        p.parseDelim(":", action);

        // Build result
        rules.push_back(Rule(levels, wildcard, action));

        // Next iteration?
        if (p.parseEnd()) {
            break;
        }
        if (!p.parseCharacter(':')) {
            throw std::runtime_error(tx("Missing ':' in message match expression"));
        }
    }

    // On success, swap rules
    m_rules.swap(rules);
}

// Match message.
bool
util::MessageMatcher::match(const afl::sys::LogListener::Message& msg, String_t& result)
{
    for (size_t i = 0, n = m_rules.size(); i < n; ++i) {
        const Rule& r = m_rules[i];
        if (r.levels.contains(msg.m_level) && r.namePattern.match(msg.m_channel)) {
            result = r.result;
            return true;
        }
    }
    return false;
}
