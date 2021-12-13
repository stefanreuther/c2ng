/**
  *  \file util/doc/loggingverifier.cpp
  *  \brief Class util::doc::LoggingVerifier
  */

#include "util/doc/loggingverifier.hpp"
#include "util/string.hpp"

util::doc::LoggingVerifier::LoggingVerifier(afl::string::Translator& tx, afl::io::TextWriter& out)
    : m_translator(tx),
      m_out(out)
{ }

util::doc::LoggingVerifier::~LoggingVerifier()
{ }

void
util::doc::LoggingVerifier::reportMessage(Message msg, const Index& idx, Index::Handle_t refNode, String_t info)
{
    String_t s = getNodeName(idx, refNode);
    addListItem(s, ": ", getMessage(msg, m_translator));
    addListItem(s, ": ", info);
    m_out.writeLine(s);
}
