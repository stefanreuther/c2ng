/**
  *  \file game/interface/completionlist.cpp
  *  \brief Class game::interface::CompletionList
  */

#include "game/interface/completionlist.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "game/config/configuration.hpp"
#include "game/interface/globalcontext.hpp"
#include "game/root.hpp"
#include "interpreter/keywords.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "util/string.hpp"

game::interface::CompletionList::CompletionList(const String_t& stem)
    : m_data(),
      m_stem(stem)
{ }

game::interface::CompletionList::~CompletionList()
{ }

void
game::interface::CompletionList::setStem(const String_t& stem)
{
    m_stem = stem;
    clear();
}

const String_t&
game::interface::CompletionList::getStem() const
{
    return m_stem;
}

void
game::interface::CompletionList::clear()
{
    m_data.clear();
}

void
game::interface::CompletionList::addCandidate(const String_t& candidate)
{
    // Only accept correct stem
    if (candidate.size() < m_stem.size() || afl::string::strCaseCompare(candidate.substr(0, m_stem.size()), m_stem) != 0) {
        return;
    }

    // Never offer as completion anything with an embedded (!= final) '$'.
    // This is used to avoid completing 'CC$foo' commands.
    String_t::size_type n = candidate.find('$', m_stem.size());
    if (n != String_t::npos && n != candidate.size()-1) {
        return;
    }

    // OK
    m_data.insert(candidate);
}

String_t
game::interface::CompletionList::getImmediateCompletion() const
{
    // Do we have any completions after all?
    Iterator_t p = m_data.begin();
    if (p == m_data.end()) {
        return String_t();
    }

    // Find longest common completion
    const String_t& first = *p;
    String_t::size_type len = first.size();
    while (p != m_data.end()) {
        const String_t& me = *p;
        if (me.size() < len) {
            len = me.size();
        }
        for (String_t::size_type i = m_stem.size(); i < len; ++i) {
            if (me[i] != first[i]) {
                len = i;
                break;
            }
        }
        ++p;
    }
    return first.substr(0, len);
}

game::interface::CompletionList::Iterator_t
game::interface::CompletionList::begin() const
{
    return m_data.begin();
}

game::interface::CompletionList::Iterator_t
game::interface::CompletionList::end() const
{
    return m_data.end();
}

bool
game::interface::CompletionList::isEmpty() const
{
    return m_data.empty();
}

/************************** buildCompletionList **************************/

namespace {
    class CompletionBuilder : public interpreter::PropertyAcceptor {
     public:
        CompletionBuilder(game::interface::CompletionList& out,
                          bool acceptCommands, bool onlyCommands)
            : m_out(out),
              m_acceptCommands(acceptCommands),
              m_onlyCommands(onlyCommands)
            { }
        virtual void addProperty(const String_t& name, interpreter::TypeHint th)
            {
                // Only accept commands when valid at current place
                if (th == interpreter::thProcedure && !m_acceptCommands) {
                    return;
                }

                // Do not accept non-commands when required
                if ((th != interpreter::thProcedure && th != interpreter::thNone) && m_acceptCommands && m_onlyCommands) {
                    return;
                }

                m_out.addCandidate(util::formatName(name));
            }
     private:
        game::interface::CompletionList& m_out;
        bool m_acceptCommands;
        bool m_onlyCommands;
    };


    void completeFileNames(game::interface::CompletionList& list,
                           afl::io::FileSystem& fs,
                           String_t stem)
    {
        using afl::io::DirectoryEntry;
        using afl::base::Enumerator;
        using afl::base::Ref;
        using afl::base::Ptr;

        // Completion requires that the file name is a proper suffix of the completion.
        String_t fileName = fs.getFileName(stem);
        if (fileName.size() > stem.size() || stem.compare(stem.size() - fileName.size(), fileName.size(), fileName) != 0) {
            return;
        }

        // List content
        try {
            // Brute force directory separator
            const char*const dirSuffix = fs.isPathSeparator('\\') ? "\\" : fs.isPathSeparator('/') ? "/" : "";

            // Read directory content
            Ref<Enumerator<Ptr<DirectoryEntry> > > entries = fs.openDirectory(fs.getDirectoryName(stem))->getDirectoryEntries();
            Ptr<DirectoryEntry> entry;
            while (entries->getNextElement(entry)) {
                // Build the complete file name without going through FileSystem's normalisation;
                // this guarantees that we produce a possible suffix.
                // Note that this requires exact case match even on Windows; addCandidate() would be case-blind but that'd be wrong on Linux.
                String_t entryName = stem.substr(0, stem.size() - fileName.size()) + entry->getTitle();
                if (entryName.size() >= stem.size() && entryName.compare(0, stem.size(), stem) == 0) {
                    if (entry->getFileType() == DirectoryEntry::tDirectory) {
                        entryName += dirSuffix;
                    }
                    list.addCandidate(entryName);
                }
            }
        }
        catch (...) {
            // Ignore
        }
    }

    void completeKeymaps(game::interface::CompletionList& list, const util::KeymapTable& tab)
    {
        for (size_t i = 0, n = tab.getNumKeymaps(); i < n; ++i) {
            if (util::KeymapRef_t p = tab.getKeymapByIndex(i)) {
                list.addCandidate(util::formatName(p->getName()));
            }
        }
    }

    void completeOptions(game::interface::CompletionList& out, const game::config::Configuration& config)
    {
        afl::base::Ref<game::config::Configuration::Enumerator_t> e(config.getOptions());
        game::config::Configuration::OptionInfo_t oi;
        while (e->getNextElement(oi)) {
            out.addCandidate(oi.first);
        }
    }
}

void
game::interface::buildCompletionList(CompletionList& out,
                                     const String_t& text,
                                     Session& session,
                                     bool onlyCommands,
                                     const afl::container::PtrVector<interpreter::Context>& contexts)
{
    // ex client/dialogs/consoledlg.cc:doComplete [part]

    // Find word to complete. The string is encoded in UTF-8, and pos is a character number.
    // Use a simple forward algorithm instead of trying to parse UTF-8 backwards.
    // Note that this function assumes that all possible completions are ASCII.
    String_t stem;
    bool acceptCommands = true;
    afl::charset::Utf8Reader rdr(afl::string::toBytes(text), 0);

    enum {
        Normal,
        SeenConfigCommand,
        SeenConfigFunction,
        SeenConfigFunctionParen,
        SeenPrefCommand,
        SeenPrefFunction,
        SeenPrefFunctionParen,
        SeenFileCommand,
        SeenConfigQuote,
        SeenPrefQuote,
        SeenFileQuote,
        SeenKeymapCommand
    } state = Normal;

    while (rdr.hasMore()) {
        // Letters, '$' and '_' can start a word, '0'..'9' and '.' can continue
        afl::charset::Unichar_t ch = rdr.eat();
        if ((state == SeenFileQuote)
            || (ch >= 'A' && ch <= 'Z')
            || (ch >= 'a' && ch <= 'z')
            || ch == '_'
            || ch == '$'
            || (!stem.empty()
                && ((ch >= '0' && ch <= '9')
                    || ch == '.')))
        {
            // Valid word character
            stem += char(ch);
        } else {
            // Not a word.
            // Process previous word.
            if (!stem.empty()) {
                if (acceptCommands && afl::string::strCaseCompare(stem, "ADDCONFIG") == 0) {
                    state = SeenConfigCommand;
                } else if (acceptCommands && afl::string::strCaseCompare(stem, "ADDPREF") == 0) {
                    state = SeenPrefCommand;
                } else if (acceptCommands && (afl::string::strCaseCompare(stem, "LOAD") == 0
                                              || afl::string::strCaseCompare(stem, "TRYLOAD") == 0
                                              || afl::string::strCaseCompare(stem, "OPEN") == 0))
                {
                    state = SeenFileCommand;
                } else if (afl::string::strCaseCompare(stem, "BIND") == 0 || afl::string::strCaseCompare(stem, "USEKEYMAP") == 0) {
                    state = SeenKeymapCommand;
                } else if (afl::string::strCaseCompare(stem, "CFG") == 0) {
                    state = SeenConfigFunction;
                } else if (afl::string::strCaseCompare(stem, "PREF") == 0) {
                    state = SeenPrefFunction;
                } else {
                    state = Normal;
                }
            }

            // Process non-word
            switch (ch) {
             case ' ':
                break;
             case '(':
                state = (state == SeenConfigFunction ? SeenConfigFunctionParen : state == SeenPrefFunction   ? SeenPrefFunctionParen : Normal);
                break;
             case '"':
             case '\'':
                if (state == SeenConfigFunctionParen || state == SeenConfigCommand) {
                    state = SeenConfigQuote;
                } else if (state == SeenPrefFunctionParen || state == SeenPrefCommand) {
                    state = SeenPrefQuote;
                } else if (state == SeenFileCommand) {
                    state = SeenFileQuote;
                } else {
                    state = Normal;
                }
                break;
             default:
                state = Normal;
            }

            if (ch != ' ' || !stem.empty()) {
                acceptCommands = false;
            }
            stem.clear();
        }
    }

    // Prepare the completion list
    out.clear();
    out.setStem(stem);

    // Do we have a stem? No completion of empty word.
    if (stem.empty()) {
        return;
    }

    // Iterate possible words
    if (state == SeenConfigQuote) {
        // Options
        if (Root* r = session.getRoot().get()) {
            completeOptions(out, r->hostConfiguration());
        }
    } else if (state == SeenPrefQuote) {
        // User preferences
        if (Root* r = session.getRoot().get()) {
            completeOptions(out, r->userConfiguration());
        }
    } else if (state == SeenFileQuote) {
        // File
        completeFileNames(out, session.world().fileSystem(), stem);
    } else if (state == SeenKeymapCommand) {
        // Keymap
        completeKeymaps(out, session.world().keymaps());
    } else {
        // Script things
        CompletionBuilder builder(out, acceptCommands, onlyCommands);
        for (size_t i = 0, n = contexts.size(); i < n; ++i) {
            if (const interpreter::Context* p = contexts[i]) {
                p->enumProperties(builder);
            }
        }
        GlobalContext(session).enumProperties(builder);
        session.world().enumSpecialCommands(builder);
        enumKeywords(builder);
    }
}
