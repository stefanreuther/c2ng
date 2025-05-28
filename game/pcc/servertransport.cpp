/**
  *  \file game/pcc/servertransport.cpp
  *  \brief Class game::pcc::ServerTransport
  */

#include "game/pcc/servertransport.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/data/access.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/string/format.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/posixfilenames.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/pcc/browserhandler.hpp"
#include "server/interface/hostturn.hpp"
#include "util/stringparser.hpp"

using afl::data::Access;
using afl::except::FileProblemException;
using afl::net::http::SimpleDownloadListener;
using afl::string::Format;
using afl::string::Messages;
using afl::string::PosixFileNames;
using afl::sys::LogListener;
using server::interface::HostTurn;
using util::ServerDirectory;
using util::StringParser;

namespace {
    const char LOG_NAME[] = "game.pcc";

    /* Turn statuses.
       These must match the ones in server::interfaces::HostTurn, but we do not want to depend on that.
       No need to map MissingTurn, NeedlessTurn; those cannot be results of a turn submission. */
    static const int32_t GreenTurn    = 1;
    static const int32_t YellowTurn   = 2;
    static const int32_t RedTurn      = 3;
    static const int32_t BadTurn      = 4;
    static const int32_t StaleTurn    = 5;
    static const int32_t TemporaryTurnFlag = 16;

    static_assert(GreenTurn  == HostTurn::GreenTurn,  "GreenTurn");
    static_assert(YellowTurn == HostTurn::YellowTurn, "YellowTurn");
    static_assert(RedTurn    == HostTurn::RedTurn,    "RedTurn");
    static_assert(BadTurn    == HostTurn::BadTurn,    "BadTurn");
    static_assert(StaleTurn  == HostTurn::StaleTurn,  "StaleTurn");
    static_assert(TemporaryTurnFlag == HostTurn::TemporaryTurnFlag, "TemporaryTurnFlag");

    String_t formatTurnStatus(int32_t result, afl::string::Translator& tx)
    {
        switch (result & ~TemporaryTurnFlag) {
         case GreenTurn:  return Format(tx("Turn was accepted"));
         case YellowTurn: return Format(tx("Turn was accepted with warnings (yellow)"));
         case RedTurn:    return Format(tx("Turn was rejected (red)"));
         case BadTurn:    return Format(tx("Turn was rejected (invalid)"));
         case StaleTurn:  return Format(tx("Turn was stale"));
         default:         return Format(tx("Unknown turn status (%d)"), result);
        }
    }

    int checkTurnFile(const String_t& str)
    {
        int nr = 0;
        StringParser p(str);
        if (p.parseString("player")
            && p.parseInt(nr)
            && p.parseString(".trn")
            && p.parseEnd()
            && (nr > 0) && (nr <= game::MAX_PLAYERS))
        {
            return nr;
        } else {
            return 0;
        }
    }
}


// Constructor.
game::pcc::ServerTransport::ServerTransport(BrowserHandler& handler, const afl::base::Ref<game::browser::Account>& acc, String_t name, int32_t hostGameNumber)
    : m_handler(handler),
      m_account(acc),
      m_name(name),
      m_urls(),
      m_hostGameNumber(hostGameNumber),
      m_temporaryTurn(false)
{ }

// Destructor.
game::pcc::ServerTransport::~ServerTransport()
{ }

// Get file content.
void
game::pcc::ServerTransport::getFile(String_t name, afl::base::GrowableBytes_t& data)
{
    // Find URL
    std::map<String_t, String_t>::const_iterator it = m_urls.find(name);
    if (it == m_urls.end()) {
        // Should not happen if called by ServerDirectory
        throw FileProblemException(name, Messages::fileNotFound());
    }

    // Download the file
    SimpleDownloadListener listener;
    m_handler.getFilePreAuthenticated(m_account, it->second, listener);

    switch (listener.wait()) {
     case SimpleDownloadListener::Succeeded:
        if (listener.getStatusCode() != 200) {
            throw FileProblemException(name, Messages::fileNotFound());
        }
        break;
     case SimpleDownloadListener::Failed:
     case SimpleDownloadListener::TimedOut:
     case SimpleDownloadListener::LimitExceeded:
        throw FileProblemException(name, Messages::networkError());
    }

    // Produce output
    data.append(listener.getResponseData());
}

// Store file content.
void
game::pcc::ServerTransport::putFile(String_t name, afl::base::ConstBytes_t data)
{
    const int playerNr = checkTurnFile(name);
    if (playerNr != 0 && m_hostGameNumber > 0) {
        // Turn upload
        std::auto_ptr<afl::data::Value> result(m_handler.uploadTurnPreAuthenticated(m_account, m_hostGameNumber, playerNr, data));
        Access a(result);
        if (a("result").toInteger()) {
            // Turn status
            LogListener& log = m_handler.log();
            afl::string::Translator& tx = m_handler.translator();
            log.write(LogListener::Info, LOG_NAME, formatTurnStatus(a("status").toInteger(), tx));

            // Turn checker output
            String_t output = a("output").toString();
            if (!output.empty()) {
                log.write(LogListener::Info, LOG_NAME, tx("Turn checker output:"));
                String_t::size_type p = 0, n;
                while ((n = output.find('\n', p)) != String_t::npos) {
                    log.write(LogListener::Info, LOG_NAME, "> " + output.substr(p, n-p));
                    p = n+1;
                }
                if (p < output.size()) {
                    log.write(LogListener::Info, LOG_NAME, "> " + output.substr(p));
                }
            }

            // Mark temporary
            if (a("allowtemp").toInteger() && m_temporaryTurn) {
                m_handler.markTurnTemporaryPreAuthenticated(m_account, m_hostGameNumber, playerNr, 1);
                log.write(LogListener::Info, LOG_NAME, tx("Turn marked temporary."));
            }
        } else {
            throw FileProblemException(name, a("error").toString());
        }
    } else {
        // File upload
        std::auto_ptr<afl::data::Value> result(m_handler.putFilePreAuthenticated(m_account, PosixFileNames().makePathName(m_name, name), data));
        Access a(result);
        if (!a("result").toInteger()) {
            throw FileProblemException(name, a("error").toString());
        }
    }
}

// Erase a file.
void
game::pcc::ServerTransport::eraseFile(String_t name)
{
    std::auto_ptr<afl::data::Value> result(m_handler.eraseFilePreAuthenticated(m_account, PosixFileNames().makePathName(m_name, name)));
    Access a(result);
    if (!a("result").toInteger()) {
        throw FileProblemException(name, a("error").toString());
    }
}

// Get content of directory.
void
game::pcc::ServerTransport::getContent(std::vector<util::ServerDirectory::FileInfo>& result)
{
    // Clear URL cache
    m_urls.clear();

    // Load directory from server
    std::auto_ptr<afl::data::Value> content(m_handler.getDirectoryContentPreAuthenticated(m_account, m_name));
    Access a(content);
    if (a("result").toInteger()) {
        for (size_t i = 0, n = a("reply").getArraySize(); i < n; ++i) {
            Access e = a("reply")[i];
            String_t type = e("type").toString();
            String_t name = e("name").toString();
            if (type == "file") {
                m_urls[name] = e("url").toString();
                result.push_back(ServerDirectory::FileInfo(name, e("size").toInteger(), true));
            } else {
                result.push_back(ServerDirectory::FileInfo(name, 0, false));
            }
        }
    } else {
        String_t error = a("error").toString();
        if (error.empty()) {
            throw FileProblemException(m_name, Messages::networkError());
        } else {
            throw FileProblemException(m_name, Format(m_handler.translator()("The server reported an error: %s"), error));
        }
    }
}

// Check validity of a file name.
bool
game::pcc::ServerTransport::isValidFileName(String_t name) const
{
    // Server requires: not empty, does not start with ".", does not contain "\0 : / \\"
    // Frontend requires: a-z, A-Z, 0-9, - . _, converted to lower-case, does not start with ". -"
    // We therefore limit to:
    //   lower case letters (if we accept upper-case, we have to anticipate case folding)
    //   digits
    //   underscore
    //   "." or "-" at any but first position
    if (name.empty()) {
        return false;
    }
    for (size_t i = 0; i < name.size(); ++i) {
        char ch = name[i];
        if ((ch >= 'a' && ch <= 'z')
            || (ch >= '0' && ch <= '9')
            || (ch == '_')
            || (i != 0 && (ch == '.' || ch == '-')))
        {
            // accept
        } else {
            return false;
        }
    }
    return true;
}

bool
game::pcc::ServerTransport::isWritable() const
{
    return true;
}
