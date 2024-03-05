/**
  *  \file util/helpindex.cpp
  *  \brief Class util::HelpIndex
  */

#include <stdexcept>
#include <algorithm>
#include "util/helpindex.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "util/charsetfactory.hpp"

namespace {
    const char*const LOG_NAME = "help";

    struct CompareNodes {
        bool operator()(const util::HelpIndex::Node* pa, const util::HelpIndex::Node* pb) const
            {
                // Sort by priority (lower value goes first),
                //    -- this is the order in which merged pages appear
                // then age (higher value goes first),
                //    -- this means later files replace older ones
                // then position (lower value goes first).
                //    -- error recovery: if a file contains multiple pages of same value, only use first one
                if (pa->priority != pb->priority) {
                    return pa->priority < pb->priority;
                }
                if (pa->file.serial != pb->file.serial) {
                    return pa->file.serial > pb->file.serial;
                }
                return pa->pos < pb->pos;
            }
    };
}

util::HelpIndex::HelpIndex()
    : m_files(),
      m_nodes(),
      m_counter(0)
{ }

util::HelpIndex::~HelpIndex()
{ }

void
util::HelpIndex::addFile(String_t name, String_t origin)
{
    m_files.pushBackNew(new File(name, origin, m_counter++));
}

void
util::HelpIndex::removeFilesByOrigin(String_t origin)
{
    // Remove all nodes that point at a file of this origin
    for (std::multimap<String_t, Node>::iterator it = m_nodes.begin(); it != m_nodes.end(); ) {
        if (it->second.file.origin == origin) {
            m_nodes.erase(it++);
        } else {
            ++it;
        }
    }

    // Remove all files of this origin
    for (size_t i = 0; i < m_files.size(); ) {
        if (m_files[i]->origin == origin) {
            m_files.erase(m_files.begin() + i);
        } else {
            ++i;
        }
    }
}

void
util::HelpIndex::find(String_t page, NodeVector_t& out, afl::io::FileSystem& fs, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // Make sure index is up-to-date
    scanNewFiles(fs, log, tx);

    // Build list of possible nodes
    std::multimap<String_t, Node>::const_iterator it = m_nodes.lower_bound(page);
    while (it != m_nodes.end() && it->first == page) {
        out.push_back(&it->second);
        ++it;
    }

    // Sort and filter out duplicates
    // Note that "toc" is never filtered
    std::sort(out.begin(), out.end(), CompareNodes());
    if (!out.empty() && page != "toc") {
        NodeVector_t::iterator it = out.begin();
        int pri = (*it)->priority;
        ++it;

        while (it != out.end()) {
            if (pri == (*it)->priority) {
                it = out.erase(it);
            } else {
                pri = (*it)->priority;
                ++it;
            }
        }
    }
}

void
util::HelpIndex::scanNewFiles(afl::io::FileSystem& fs, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    for (size_t i = 0, n = m_files.size(); i < n; ++i) {
        if (!m_files[i]->scanned) {
            m_files[i]->scanned = true;
            try {
                scanFile(*m_files[i], fs);
                log.write(log.Info, LOG_NAME, afl::string::Format(tx("Scanned help file %s.").c_str(), m_files[i]->name));
            }
            catch (std::exception& e) {
                log.write(log.Warn, LOG_NAME, tx("Error scanning help file"), e);
            }
        }
    }
}

void
util::HelpIndex::scanFile(const File& file, afl::io::FileSystem& fs)
{
    using afl::io::xml::Reader;
    afl::base::Ref<afl::io::Stream> s = fs.openFile(file.name, fs.OpenRead);
    CharsetFactory csf;
    Reader rdr(*s, afl::io::xml::DefaultEntityHandler::getInstance(), csf);

    std::vector<int> priorityStack;
    priorityStack.push_back(100);

    bool atPageStart = false;
    bool atGroupStart = false;
    FileSize_t pagePos = 0;
    while (1) {
        switch (rdr.readNext()) {
         case Reader::TagStart:
            atPageStart = (rdr.getTag() == "page");
            atGroupStart = (rdr.getTag() == "group" || rdr.getTag() == "help");
            pagePos = rdr.getPos();
            priorityStack.push_back(priorityStack.back());
            break;
         case Reader::TagAttribute:
            if (atPageStart && rdr.getName() == "id") {
                m_nodes.insert(std::make_pair(rdr.getValue(), Node(priorityStack.back(), file, pagePos)));
            }
            if (atGroupStart && rdr.getName() == "priority") {
                afl::string::strToInteger(rdr.getValue(), priorityStack.back());
            }
            break;
         case Reader::TagEnd:
            if (priorityStack.size() > 1) {
                priorityStack.pop_back();
            }
            break;
         case Reader::Eof:
            goto out;
         default:
            break;
        }
    }
 out:;
}
