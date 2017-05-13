/**
  *  \file util/syntax/keywordtable.cpp
  */

#include "util/syntax/keywordtable.hpp"
#include "afl/string/format.hpp"
#include "util/fileparser.hpp"

namespace {
    const char LOG_NAME[] = "syntaxdb";

    class SyntaxParser : public util::FileParser {
     public:
        SyntaxParser(util::syntax::KeywordTable& table, afl::sys::LogListener& log);
        virtual void handleLine(const String_t& fileName, int lineNr, String_t line);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line);

     private:
        void error(const String_t& fileName, int lineNr, const char* message);
        util::syntax::KeywordTable& m_table;
        afl::sys::LogListener& m_log;
        std::vector<String_t> m_prefixes;
    };
}

SyntaxParser::SyntaxParser(util::syntax::KeywordTable& table, afl::sys::LogListener& log)
    : FileParser(";#"),
      m_table(table),
      m_log(log),
      m_prefixes()
{
    // ex SyntaxParser::SyntaxParser
    m_prefixes.push_back(String_t());
}

void
SyntaxParser::handleLine(const String_t& fileName, int lineNr, String_t line)
{
    // ex SyntaxParser::process
    // Parse
    String_t::size_type n = line.find_first_of("={}");
    if (n == String_t::npos) {
        error(fileName, lineNr, "syntax error");
        return;
    }

    // Key and value
    String_t key   = afl::string::strTrim(String_t(line, 0, n));
    String_t value = afl::string::strTrim(String_t(line, n+1));
    switch (line[n]) {
     case '=':
        if (key.empty()) {
            error(fileName, lineNr, "missing key in assignment");
        } else if (!value.empty() && value[0] == '$') {
            // reference
            value.erase(0, 1);
            if (const String_t* p = m_table.get(value)) {
                m_table.add(m_prefixes.back() + key, *p);
            } else {
                error(fileName, lineNr, "reference to non-existant key");
            }
        } else {
            m_table.add(m_prefixes.back() + key, value);
        }
        break;

     case '{':
        if (key.empty() || !value.empty()) {
            error(fileName, lineNr, "group must have form \"<name> {\"");
        } else {
            m_prefixes.push_back(m_prefixes.back() + key + ".");
        }
        break;

     case '}':
        if (!key.empty() || !value.empty()) {
            error(fileName, lineNr, "group end must have form \"}\"");
        } else if (m_prefixes.size() <= 1) {
            error(fileName, lineNr, "too many group terminators");
        } else {
            m_prefixes.pop_back();
        }
        break;
    }
}

void
SyntaxParser::handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
{ }

void
SyntaxParser::error(const String_t& fileName, int lineNr, const char* message)
{
    // ex SyntaxParser::error
    m_log.write(m_log.Warn, LOG_NAME, afl::string::Format("%s:%d: %s", fileName, lineNr, message));
}


util::syntax::KeywordTable::KeywordTable()
    : m_data()
{
    // ex SyntaxDatabase::SyntaxDatabase
}

util::syntax::KeywordTable::~KeywordTable()
{ }

void
util::syntax::KeywordTable::load(afl::io::Stream& in, afl::sys::LogListener& log)
{
    // ex SyntaxDatabase::load
    SyntaxParser(*this, log).parseFile(in);
}

void
util::syntax::KeywordTable::add(const String_t& key, const String_t& value)
{
    // ex SyntaxDatabase::add
    m_data[afl::string::strLCase(key)] = value;
}

const String_t*
util::syntax::KeywordTable::get(const String_t& key) const
{
    // ex SyntaxDatabase::get
    std::map<String_t, String_t>::const_iterator it = m_data.find(afl::string::strLCase(key));
    if (it != m_data.end()) {
        return &it->second;
    } else {
        return 0;
    }
}
