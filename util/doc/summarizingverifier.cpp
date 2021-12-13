/**
  *  \file util/doc/summarizingverifier.cpp
  *  \brief Class util::doc::SummarizingVerifier
  */

#include "util/doc/summarizingverifier.hpp"
#include "afl/string/format.hpp"
#include "util/doc/index.hpp"

using afl::string::Format;

util::doc::SummarizingVerifier::SummarizingVerifier()
    : m_messages()
{ }

util::doc::SummarizingVerifier::~SummarizingVerifier()
{ }

bool
util::doc::SummarizingVerifier::hasMessage(Message msg) const
{
    size_t idx = static_cast<size_t>(msg);
    return idx < m_messages.size() && m_messages[idx] != 0;
}

void
util::doc::SummarizingVerifier::printMessage(Message msg,
                                             const Index& idx,
                                             bool brief,
                                             afl::string::Translator& tx,
                                             afl::io::TextWriter& out) const
{
    out.writeLine(getMessage(msg, tx));

    size_t i = static_cast<size_t>(msg);
    if (i < m_messages.size() && m_messages[i] != 0) {
        const MessageMap_t& m = *m_messages[i];
        for (MessageMap_t::const_iterator it = m.begin(); it != m.end(); ++it) {
            if (brief) {
                out.writeLine(Format("  %s", it->first));
            } else {
                String_t name = getNodeName(idx, it->second.refNode);
                if (it->second.count != 1) {
                    name += Format(" (+%d)", it->second.count-1);
                }
                if (it->first.empty()) {
                    out.writeLine(Format("  %s", name));
                } else {
                    out.writeLine(Format("  %s: %s", name, it->first));
                }
            }
        }
    }
}

void
util::doc::SummarizingVerifier::reportMessage(Message msg, const Index& /*idx*/, Index::Handle_t refNode, String_t info)
{
    // Ensure sufficient size of m_messages
    size_t idx = static_cast<size_t>(msg);
    if (idx >= m_messages.size()) {
        m_messages.resize(idx+1);
    }

    // Ensure MessageMap_t is present
    if (m_messages[idx] == 0) {
        m_messages.replaceElementNew(idx, new MessageMap_t());
    }

    // Insert MessageInfo (or use existing one) and increase its count.
    m_messages[idx]->insert(std::make_pair(info, MessageInfo(refNode, 0)))
        .first->second.count++;
}
