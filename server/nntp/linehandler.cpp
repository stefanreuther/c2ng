/**
  *  \file server/nntp/linehandler.cpp
  *  \brief Class server::nntp::LineHandler
  *
  *  Minimum command set:
  *  - AUTHINFO (required for authentication)
  *  - QUIT (trivial)
  *  - LIST ACTIVE (required to get list of groups)
  *  - GROUP (required to enter a group)
  *  - ARTICLE/HEAD (required to access articles)
  *  This is enough to make 'tin' work.
  *
  *  Additional commands to make it practical:
  *  - HELP (trivial)
  *  - BODY/STAT (trivial once we have ARTICLE)
  *  - OVER/XOVER (Mozilla doesn't work without)
  *  - LIST SUBSCRIPTIONS (why not)
  *  - LIST OVERVIEW.FMT (required for XOVER)
  *  - MODE (trivial, ignored)
  *
  *  References:
  *  - RFC 3977 Network News Transfer Protocol (NNTP)
  *  - RFC 2980 Common NNTP Extensions
  *  - hamsrv (an earlier NNTP implementation I wrote)
  */

#include "server/nntp/linehandler.hpp"
#include "afl/base/countof.hpp"
#include "afl/data/access.hpp"
#include "afl/data/hash.hpp"
#include "afl/net/line/linesink.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/talknntpclient.hpp"
#include "server/interface/talkpostclient.hpp"
#include "server/interface/talkrender.hpp"
#include "server/interface/usermanagementclient.hpp"
#include "server/nntp/root.hpp"
#include "server/nntp/session.hpp"
#include "server/types.hpp"

using afl::string::Format;
using server::interface::BaseClient;
using server::interface::TalkNNTP;
using server::interface::TalkNNTPClient;
using server::interface::TalkPostClient;
using server::interface::TalkRender;
using server::interface::UserManagementClient;

namespace {
    const char*const LOG_NAME = "nntp.command";

    const char*const TOO_FEW_ARGS        = "501 Too few arguments";
    const char*const TOO_MANY_ARGS       = "501 Too many arguments";
    const char*const SYNTAX_ERROR        = "501 Syntax error";
    const char*const NOT_SUPPORTED_MAJOR = "500 Unsupported command";
    const char*const NOT_SUPPORTED_MINOR = "501 Unsupported command";
    const char*const NEED_AUTH           = "480 Need authentication";
    const char*const NOT_IN_GROUP        = "412 Not currently in a newsgroup";
    const char*const NO_SUCH_GROUP       = "411 No such group";

    /** List of overview fields. */
    const char*const OVERVIEW_FIELDS[] = {
        "Subject",
        "From",
        "Date",
        "Message-ID",
        "References",
        ":bytes",
        ":lines",
        "Xref",
    };

    /** Index of first "full" overview field. All but the standard fields must be "full"
        fields including the header name. Therefore, this value follows directly from
        the specification.

        'tin' only accepts the Xref header in full format.
        If Xref is not listed in full format, it attempts to access Xref using XHDR
        which we do not support. */
    const size_t OVERVIEW_FIELDS_FIRST_FULL = 7;

    /** Eat a word from the string.
        \param cmd [in/out] String
        \return First word of string */
    String_t eatWord(String_t& cmd)
    {
        String_t::size_type i = cmd.find_first_not_of(" \t\r\n");
        String_t result;
        if (i == String_t::npos) {
            cmd.clear();
        } else {
            String_t::size_type n = cmd.find_first_of(" \t\r\n", i);
            if (n == String_t::npos) {
                result.assign(cmd, i, cmd.size()-i);
                cmd.clear();
            } else {
                result.assign(cmd, i, n-i);
                cmd.erase(0, n);
            }
        }
        return result;
    }

    /** Eat up remainder of string.
        \param cmd [in/out] String (will be cleared)
        \return Rest of string */
    String_t eatRest(String_t& cmd)
    {
        String_t result(afl::string::strTrim(cmd));
        cmd.clear();
        return result;
    }

    /** Sanitize a header field value.
        Header fields can contain spaces, tabs, and newlines, which cannot be transmitted
        in a news overview file. This function sanitizes them by replacing runs of
        \r\n\t or space, starting with a \r\n\t, by a single space. */
    String_t sanitizeFieldValue(String_t value)
    {
        String_t::size_type i = 0;
        while (1) {
            String_t::size_type j = value.find_first_of("\r\n\t", i);
            if (j == String_t::npos) {
                break;
            }
            String_t::size_type k = value.find_first_not_of("\r\n\t ", j);
            if (k == String_t::npos) {
                value.erase(j);
                break;
            }
            value.replace(j, k-j, 1, ' ');
            i = j+1;
        }
        if (value.empty()) {
            value = " ";
        }
        return value;
    }

    /** Escape dots.
        Prepends dots to lines starting with a dot. */
    String_t escapeDots(String_t value)
    {
        // Start of string is the start of a line
        String_t::size_type i = 0;
        while (i < value.size()) {
            if (value[i] == '.') {
                value.insert(i, 1, '.');
            }

            // Find next line. Since linefeeds are CRLF, it suffices to look for LF.
            i = value.find('\n', i);
            if (i == String_t::npos) {
                break;
            }
            ++i;
        }
        return value;
    }

    struct CompareNewsgroupNames {
        bool operator()(const TalkNNTP::Info& a, const TalkNNTP::Info& b) const
            { return a.newsgroupName < b.newsgroupName; }
    };
}

server::nntp::LineHandler::LineHandler(Root& root, Session& session)
    : m_root(root),
      m_session(session),
      m_id(root.allocateId()),
      m_status(ReadCommand)
{
    // ex NntpWorker::NntpWorker [part]
    // FIXME: -classic would log the incoming IP:
    // printf("[main] [fd:%d] [id:%d] connection accepted from '%s:%s'\n", fd, worker->getId(), name, serv);
    m_root.log().write(afl::sys::Log::Info, LOG_NAME, Format("[id:%d] connected", m_id));
}

server::nntp::LineHandler::~LineHandler()
{
    m_root.log().write(afl::sys::Log::Info, LOG_NAME, Format("[id:%d] disconnected", m_id));
}

bool
server::nntp::LineHandler::handleOpening(afl::net::line::LineSink& response)
{
    response.handleLine("200 c2nntp-ng says hello");
    return false;
}

bool
server::nntp::LineHandler::handleLine(const String_t& line, afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleRead [part]
    // FIXME: -classic handles overlong lines:
    //    const std::size_t LIMIT = 20000;
    //    send("501 Line too long\r\n");
    class LogWrapper : public afl::net::line::LineSink {
     public:
        LogWrapper(afl::sys::LogListener& log, int32_t id, afl::net::line::LineSink& parent)
            : m_log(log), m_id(id), m_parent(parent), m_did(false)
            { }
        virtual void handleLine(const String_t& line)
            {
                if (!m_did) {
                    m_did = true;
                    m_log.write(afl::sys::Log::Info, LOG_NAME, Format("[id:%d] < %s", m_id, line));
                }
                m_parent.handleLine(line);
            }
     private:
        afl::sys::LogListener& m_log;
        int32_t m_id;
        afl::net::line::LineSink& m_parent;
        bool m_did;
    };

    try {
        LogWrapper wrap(m_root.log(), m_id, response);
        switch (m_status) {
         case ReadCommand:  return handleCommand(line, wrap);  break;
         case ReadPostData: return handlePostData(line, wrap); break;
        }
    }
    catch (std::exception& e) {
        m_root.log().write(afl::sys::Log::Error, LOG_NAME, "Exception", e);
        response.handleLine("403 Internal error");
    }
    return true;
}

void
server::nntp::LineHandler::handleConnectionClose()
{ }

bool
server::nntp::LineHandler::handleCommand(String_t line, afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleCommand
    // Figure out command verb
    const String_t verb = afl::string::strUCase(eatWord(line));
    if (verb.empty()) {
        response.handleLine(SYNTAX_ERROR);
        return false;
    }

    // Log it
    if (verb != "AUTHINFO") {
        m_root.log().write(afl::sys::Log::Info, LOG_NAME, Format("[id:%d] > %s%s", m_id, verb, line));
    }

    // Process it
    if (verb == "QUIT") {
        response.handleLine("205 Good bye");
        return true;
    } else if (verb == "ARTICLE") {
        // ARTICLE (977, 3977)
        handleArticle(line, true, true, response);
        return false;
    } else if (verb == "AUTHINFO") {
        // AUTHINFO (2980)
        handleAuthinfo(line, response);
        return false;
    } else if (verb == "BODY") {
        // BODY (977, 3977)
        handleArticle(line, false, true, response);
        return false;
    } else if (verb == "GROUP") {
        // GROUP (977, 3977)
        handleGroup(line, response);
        return false;
    } else if (verb == "HEAD") {
        // HEAD (977, 3977)
        handleArticle(line, true, false, response);
        return false;
    } else if (verb == "HELP") {
        // HELP (977, 3977)
        handleHelp(response);
        return false;
    } else if (verb == "LIST") {
        // LIST (977, 2980, 3977)
        handleList(line, response);
        return false;
    } else if (verb == "LISTGROUP") {
        // LISTGROUP (2980, 3977)
        handleListGroup(line, response);
        return false;
    } else if (verb == "MODE") {
        // MODE STREAM|READER (2980, 3977)
        response.handleLine("200 Ignored");
        return false;
    } else if (verb == "STAT") {
        // STAT (977, 3977)
        handleArticle(line, false, false, response);
        return false;
    } else if (verb == "OVER" || verb == "XOVER") {
        handleOver(line, response);
        return false;
    } else {
        // CAPABILITIES                                                    3977
        // CHECK <msgid>                                                   2980
        // DATE                                                            2980, 3977
        //          Answer: yyyymmddhhmmss
        // HDR                                                             3977
        //          see XHDR
        // IHAVE <msgid>                                                   977, 3977
        // LAST                                                            977, 3977
        //          decrease internal current-article pointer
        //          Answer: sequence number, msgid
        // NEWGROUPS yymmdd hhmmss [GMT] [distributions]                   977, 3977
        //          Answer: list of newsgroups
        // NEWNEWS newsgroups date time [GMT] [distributions]              977, 3977
        // NEXT                                                            977, 3977
        //          see LAST
        // POST                                                            977, hamsrv, 3977
        // SLAVE                                                           977
        // TAKETHIS <msgid>                                                2980
        // XGTITLE [wildmat]       same as LIST NEWSGROUPS, deprecated     2980
        // XHDR header [range|<msgid>]                                     2980
        //          Answer: header field value for an article
        // XINDEX [wildmat]        deprecated                              2980
        // XPAT header range|<msgid> pat...                                2980
        // XPATH <msgid>           deprecated                              2980
        // XREPLIC ...                                                     2980
        // XROVER [range]                                                  2980
        //          Same as XHDR References
        // XTHREAD [DBINIT|THREAD] deprecated                              2980
        response.handleLine(NOT_SUPPORTED_MAJOR);
        return false;
    }
}

bool
server::nntp::LineHandler::handlePostData(String_t /*line*/, afl::net::line::LineSink& /*response*/)
{
    // ex NntpWorker::handlePostData
    // FIXME: unimplemented (also in -classic)
    return false;
}

// /** Check authentication.
//     \retval true Authentication succeeded, command processing can proceed
//     \retval false Not authenticated. An error message has been sent, command processing must abort */
bool
server::nntp::LineHandler::checkAuth(afl::net::line::LineSink& response)
{
    // ex NntpWorker::checkAuth
    if (m_session.auth_status != Session::Authenticated) {
        response.handleLine(NEED_AUTH);
        return false;
    } else {
        m_root.configureReconnect();
        BaseClient(m_root.talk()).setUserContext(m_session.auth_uid);
        return true;
    }
}

// /** Fill cache containing newsgroups.
//     \retval true Cache is available, proceed
//     \retval false Error. An error message has been sent, command processing must abort */
bool
server::nntp::LineHandler::fillGroupListCache(afl::net::line::LineSink& response)
{
    // ex NntpWorker::fillGroupListCache
    // Boilerplate
    if (!checkAuth(response)) {
        return false;
    }

    // Do we have it already?
    if (!m_session.m_groupListCache.empty()) {
        return true;
    }

    // Reload
    TalkNNTPClient(m_root.talk()).listNewsgroups(m_session.m_groupListCache);

    // For reproducability, sort by newsgroup name.
    // c2talk outputs this in whatever form the database has it.
    m_session.m_groupListCache.sort(CompareNewsgroupNames());
    return true;
}

/** Resolve sequence number into message number.
    \param [in]  seq       Sequence number
    \param [out] rfcMsgId  Message Id
    \param [out] response  Write response here
    \retval nonzero Message number
    \retval 0 Error. An error message has been sent, command processing must abort */
int32_t
server::nntp::LineHandler::resolveSequenceNumber(int32_t seq, String_t& rfcMsgId, afl::net::line::LineSink& response)
{
    // Must be in a forum
    if (m_session.current_forum == 0) {
        response.handleLine(NOT_IN_GROUP);
        return 0;
    }

    // Look up in cache
    std::map<int32_t, int32_t>::const_iterator it = m_session.current_seq_map.find(seq);
    if (it == m_session.current_seq_map.end() || it->second == 0) {
        response.handleLine("423 No such article number");
        return 0;
    }

    // Get message Id
    rfcMsgId = TalkPostClient(m_root.talk()).getHeaderField(it->second, "rfcmsgid");

    // OK
    return it->second;
}

/** Parse sequence number range.
    \param [in] range Range specified by user
    \param [out] min Lower bound
    \param [out] max Upper bound
    \param [out] response Write response here
    \retval true Success
    \retval false Error. An error message has been sent, command processing must abort */
bool
server::nntp::LineHandler::parseRange(const String_t& range, int32_t& min, int32_t& max, afl::net::line::LineSink& response)
{
    if (range.empty()) {
        min = max = m_session.current_seq;
        return true;
    } else {
        String_t::size_type i = range.find('-');
        int32_t a;
        if (i == String_t::npos) {
            if (afl::string::strToInteger(range, a)) {
                min = max = a;
                return true;
            } else {
                response.handleLine(SYNTAX_ERROR);
                return false;
            }
        } else {
            // Minimum
            if (i == 0) {
                min = 1;
            } else if (afl::string::strToInteger(range.substr(0, i), a)) {
                min = a;
            } else {
                response.handleLine(SYNTAX_ERROR);
                return false;
            }

            // Maximum
            if (i+1 == range.size()) {
                max = 0x7FFFFFFF;
            } else if (afl::string::strToInteger(range.substr(i+1), a)) {
                max = a;
            } else {
                response.handleLine(SYNTAX_ERROR);
                return false;
            }

            return true;
        }
    }
}

/** Enter a newsgroup.
    This loads the cache of sequence numbers to message numbers.
    \param groupName Name of newsgroup
    \param response Write responses here
    \retval true Success
    \retval false Error. An error message has been sent, command processing must abort */
bool
server::nntp::LineHandler::enterGroup(const String_t& groupName, afl::net::line::LineSink& response)
{
    // Find newsgroup. FIXME: use grouplist_cache?
    TalkNNTP::Info groupInfo;
    try {
        groupInfo = TalkNNTPClient(m_root.talk()).findNewsgroup(groupName);
    }
    catch (...) {
        response.handleLine(NO_SUCH_GROUP);
        return false;
    }

    // OK?
    int32_t forumId = groupInfo.forumId;
    if (forumId == 0) {
        response.handleLine(NO_SUCH_GROUP);
        return false;
    }

    // OK, group exists. Load list of sequence numbers.
    m_session.current_group = groupName;
    m_session.current_forum = forumId;
    m_session.current_seq = 0;
    m_session.current_seq_map.clear();

    afl::data::IntegerList_t seqList;
    TalkNNTPClient(m_root.talk()).listMessages(forumId, seqList);
    for (size_t i = 0, n = seqList.size(); i+1 < n; i += 2) {
        m_session.current_seq_map.insert(std::make_pair(seqList[i], seqList[i+1]));
    }

    return true;
}

/** ARTICLE/HEAD/BODY/STAT command.
    - References: RFC 977, RFC 3977, hamsrv
    - Syntax: <verb> msgid
    - Syntax: <verb> sequenceNumber

    Responses:
    - 220 n message-id Article follows (multi-line)
    - 221 n message-id Headers follow (multi-line)
    - 222 n message-id Body follows (multi-line)
    - 223 n message-id Article exists
    - 412 No newsgroup selected
    - 423 No article with that number
    - 420 Current article number is invalid (is this possible?)

    \param args   Parameters
    \param header true to send the header (HEAD, ARTICLE)
    \param body   true to send the body (BODY, ARTICLE)
    \param response Write response here */
void
server::nntp::LineHandler::handleArticle(String_t args, bool header, bool body, afl::net::line::LineSink& response)
{
    // Boilerplate
    if (!checkAuth(response)) {
        return;
    }

    // Check Id
    String_t id(eatWord(args));
    int32_t msgId;
    int32_t seqNr;
    String_t rfcMsgId;
    if (id == "") {
        // No parameter: get current article
        seqNr = m_session.current_seq;
        msgId = resolveSequenceNumber(m_session.current_seq, rfcMsgId, response);
        if (msgId == 0) {
            // Message sent by resolveSequenceNumber
            return;
        }
    } else if (afl::string::strToInteger(id, seqNr)) {
        // Sequence number
        msgId = resolveSequenceNumber(seqNr, rfcMsgId, response);
        if (msgId == 0) {
            // Message sent by resolveSequenceNumber
            return;
        }
    } else if (id.size() > 2 && id[0] == '<' && id[id.size()-1] == '>') {
        // could be a mid
        try {
            seqNr = 0;
            rfcMsgId = id.substr(1, id.size()-2);
            msgId = TalkNNTPClient(m_root.talk()).findMessage(rfcMsgId);
        }
        catch (...) {
            response.handleLine("430 No such article");
            return;
        }
    } else {
        response.handleLine(SYNTAX_ERROR);
        return;
    }

    // At this point, msgId is a resolved message number.
    // Fetch required data.
    // If anything fails here, we'll get into the
    // throw-an-exception-log-internal-error branch,
    // which should be good enough.

    afl::base::Ptr<afl::data::Hash> headerArray;
    String_t bodyString;
    if (header) {
        headerArray = TalkNNTPClient(m_root.talk()).getMessageHeader(msgId).asPtr();
    }
    if (body) {
        TalkRender::Options opts;
        opts.baseUrl = m_root.getBaseUrl();
        opts.format = String_t("news");
        bodyString = escapeDots(TalkPostClient(m_root.talk()).render(msgId, opts));
    }

    // Send success response
    int code = (header
                ? body ? 220 : 221
                : body ? 222 : 223);
    response.handleLine(Format("%d %d <%s> found", code, seqNr, rfcMsgId));
    if (seqNr != 0) {
        m_session.current_seq = seqNr;
    }
    if (header) {
        for (size_t i = 0, n = headerArray->getKeys().getNumNames(); i < n; ++i) {
            const String_t& headerName = headerArray->getKeys().getNameByIndex(i);
            if (!headerName.empty() && headerName[0] != ':') {
                response.handleLine(Format("%s: %s", headerName, toString(headerArray->getValues()[i])));
            }
        }
    }
    if (header && body) {
        response.handleLine(String_t());
    }
    if (body) {
        response.handleLine(bodyString);
    }
    if (header || body) {
        response.handleLine(".");
    }
}

// /** AUTHINFO command.
//     - References: RFC 2980
//     - Syntax: AUTHINFO USER user
//     - Syntax: AUTHINFO PASS pass
//     - Syntax: AUTHINFO SIMPLE
//     - Syntax: AUTHINFO GENERIC args

//     Responses:
//     - 281 Authentication accepted
//     - 381 More authentication information required
//     - 480 Authentication required
//     - 482 Authentication rejected
//     - 502 No permission

//     USER/PASS is the simplest scheme and is implemented here.
//     tin uses AUTHINFO GENERIC as a first guess, passing us the log-in name.
//     That would probably work in a trusted setup with identd,
//     but there's no way to make use of that for our application. */
void
server::nntp::LineHandler::handleAuthinfo(String_t args, afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleAuthinfo
    // Do we have a sub-verb? Also log it.
    String_t kind(afl::string::strUCase(eatWord(args)));
    m_root.log().write(afl::sys::Log::Info, LOG_NAME, Format("[id:%d] > AUTHINFO %s [...]", m_id, kind));
    if (kind.empty()) {
        response.handleLine(TOO_FEW_ARGS);
        return;
    }

    if (kind == "USER") {
        // AUTHINFO USER
        m_session.auth_user = eatRest(args);
        m_session.auth_status = Session::NeedPass;
        response.handleLine("381 Send password");
    } else if (kind == "PASS") {
        // AUTHINFO PASS
        if (m_session.auth_status == Session::NeedPass) {
            m_root.configureReconnect();
            try {
                m_session.auth_uid = UserManagementClient(m_root.user()).login(m_session.auth_user, eatRest(args));
                m_session.auth_status = Session::Authenticated;
                m_session.m_groupListCache.clear();
                response.handleLine("281 Authentification accepted");
                m_root.log().write(afl::sys::Log::Info, LOG_NAME, Format("[id:%d] [user:%s] Authenticated as '%s'", m_id, m_session.auth_uid, m_session.auth_user));
            }
            catch (...) {
                response.handleLine("482 Authentification rejected");
                m_session.auth_status = Session::NeedUser;
            }
        } else {
            response.handleLine("501 Need AUTHINFO USER first");
        }
    } else {
        response.handleLine(NOT_SUPPORTED_MINOR);
    }
}

// /** GROUP command.
//     - References: RFC 977, RFC 3977, hamsrv
//     - Indicating capability: READER
//     - Syntax: GROUP groupname

//     Responses:
//     - 211 num-articles first last sequence groupname
//     - 411 No such newsgroup */
void
server::nntp::LineHandler::handleGroup(String_t args, afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleGroup
    // Must be authenticated
    if (!checkAuth(response)) {
        return;
    }

    // Must have group name
    String_t groupName(eatRest(args));
    if (groupName.empty()) {
        response.handleLine(SYNTAX_ERROR);
        return;
    }

    // Look up group
    if (!enterGroup(groupName, response)) {
        return;
    }

    // Generate output
    if (m_session.current_seq_map.empty()) {
        response.handleLine(Format("211 0 0 0 %s is empty", groupName));
    } else {
        int32_t firstSeq = m_session.current_seq_map.begin()->first;
        int32_t lastSeq = (--m_session.current_seq_map.end())->first;
        response.handleLine(Format("211 %d %d %d %s selected", lastSeq - firstSeq + 1, firstSeq, lastSeq, groupName));
        m_session.current_seq = firstSeq;
    }
}

// /** LIST command.
//     Dispatches to various sub-commands. */
void
server::nntp::LineHandler::handleList(String_t args, afl::net::line::LineSink& response)
{
    // LIST ACTIVE.TIMES                                               2980, 3977
    //          Answer: newsgroup creation-time creator-email
    // LIST DISTRIBUTIONS                                              2980
    // LIST DISTRIB.PATS                                               2980, 3977
    // LIST HEADERS                                                    3977
    String_t what = afl::string::strUCase(eatWord(args));
    if (what == "" || what == "ACTIVE") {
        handleListActive(response);
    } else if (what == "NEWSGROUPS") {
        handleListNewsgroups(response);
    } else if (what == "SUBSCRIPTIONS") {
        handleListSubscriptions(response);
    } else if (what == "OVERVIEW.FMT") {
        handleListOverviewFormat(response);
    } else {
        response.handleLine(NOT_SUPPORTED_MINOR);
    }
}

// /** LIST / LIST ACTIVE command.
//     - References: RFC 977, RFC 3977, RFC 2980, hamsrv
//     - Indicating capability: READER
//     - Syntax: LIST
//     - Syntax: LIST ACTIVE [wildmat]

//     Responses:
//     - 215 (multiline, followed by "newsgroup lastSeq firstSeq postingAllowedFlag")

//     FIXME: wildmat is not implemented. */
void
server::nntp::LineHandler::handleListActive(afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleListActive
    // Fetch group list
    if (!fillGroupListCache(response)) {
        return;
    }

    // Send it
    response.handleLine("215 List of newsgroups follows");
    for (size_t i = 0, n = m_session.m_groupListCache.size(); i < n; ++i) {
        if (const TalkNNTP::Info* ele = m_session.m_groupListCache[i]) {
            response.handleLine(Format("%s %d %d %c",
                                       ele->newsgroupName,
                                       ele->lastSequenceNumber,
                                       ele->firstSequenceNumber,
                                       ele->writeAllowed ? 'y' : 'n'));
        }
    }
    response.handleLine(".");
}

// /** LIST NEWSGROUPS command.
//     - References: RFC 2980, RFC 3977, hamsrv
//     - Indicating capability: READER
//     - Syntax: LIST NEWSGROUPS [wildmat]

//     Responses:
//     - 215 information follows (multiline, followed by newsgroup/description pairs)
//     - 503 program error, function not performed

//     FIXME: wildmat is not implemented. */
void
server::nntp::LineHandler::handleListNewsgroups(afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleListNewsgroups
    // Fetch group list
    if (!fillGroupListCache(response)) {
        return;
    }

    // Send it
    response.handleLine("215 List of newsgroups follows");
    for (size_t i = 0, n = m_session.m_groupListCache.size(); i < n; ++i) {
        if (const TalkNNTP::Info* ele = m_session.m_groupListCache[i]) {
            String_t description = ele->description;
            String_t::size_type n = description.find_first_of("\r\n");
            if (n != String_t::npos) {
                description.erase(n);
            }
            response.handleLine(Format("%s %s", ele->newsgroupName, description));
        }
    }
    response.handleLine(".");
}

// /** LIST SUBSCRIPTIONS command.
//     - References: RFC 2980
//     - Syntax: LIST SUBSCRIPTIONS

//     Response codes:
//     - 215 information follows (multi-line, list of newsgroups follows)
//     - 503 program error, function not performed */
void
server::nntp::LineHandler::handleListSubscriptions(afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleListSubscriptions
    // Boilerplate
    if (!checkAuth(response)) {
        return;
    }

    // Fetch list
    // For now, the subscription list is defined as root group.
    // We'd have to deviate from that when introducing a special NNTP-only group.
    afl::data::StringList_t list;
    TalkNNTPClient(m_root.talk()).listNewsgroupsByGroup("root", list);

    // Send result
    response.handleLine("215 Recommendations follow");
    for (afl::data::StringList_t::const_iterator i = list.begin(); i != list.end(); ++i) {
        response.handleLine(*i);
    }
    response.handleLine(".");
}

// /** LIST OVERVIEW.FMT command.
//     - References: RFC 3977, RFC 2980, hamsrv
//     - Indicating capability: OVER
//     - Syntax: LIST OVERVIEW.FMT

//     Responses:
//     - 215 Information follows (multi-line)
//     - 503 program error, function not performed */
void
server::nntp::LineHandler::handleListOverviewFormat(afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleListOverviewFormat
    response.handleLine("215 List follows");
    response.handleLine("Subject:");
    response.handleLine("From:");
    response.handleLine("Date:");
    response.handleLine("Message-ID:");
    response.handleLine("References:");
    response.handleLine(":bytes");
    response.handleLine(":lines");
    response.handleLine("Xref:full");
    response.handleLine(".");
}

// /** LISTGROUP command.
//     - References: RFC 3977, RFC 2980, hamsrv
//     - Indicating capability: READER
//     - Syntax: LISTGROUP [group [range]]

//     Responses:
//     - 211 number low high group Article numbers follow (multi-line)
//     - 411 No such newsgroup
//     - 412 No newsgroup selected

//     Actions: same as GROUP (select the group, reset current article),
//     and list sequence numbers. */
void
server::nntp::LineHandler::handleListGroup(String_t args, afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleListGroup
    // Boilerplate
    if (!checkAuth(response)) {
        return;
    }

    // If group specified, enter it
    String_t groupName(eatWord(args));
    if (!groupName.empty()) {
        if (!enterGroup(groupName, response)) {
            return;
        }
    }
    if (m_session.current_forum == 0) {
        response.handleLine(NOT_IN_GROUP);
        return;
    }

    // Range specified?
    int32_t min, max;
    String_t range(eatWord(args));
    if (!range.empty()) {
        if (!parseRange(range, min, max, response)) {
            return;
        }
    } else {
        min = 1;
        max = 0x7FFFFFFF;
    }

    // List
    if (m_session.current_seq_map.empty()) {
        response.handleLine(Format("211 0 0 0 %s is empty", m_session.current_group));
    } else {
        int32_t firstSeq = m_session.current_seq_map.begin()->first;
        int32_t lastSeq = (--m_session.current_seq_map.end())->first;
        response.handleLine(Format("211 %d %d %d %s selected", lastSeq - firstSeq + 1, firstSeq, lastSeq, m_session.current_group));
        m_session.current_seq = firstSeq;

        std::map<int32_t, int32_t>::const_iterator i = m_session.current_seq_map.lower_bound(min), e = m_session.current_seq_map.end();
        while (i != e && i->first <= max) {
            response.handleLine(Format("%d", i->first));
            ++i;
        }
    }
    response.handleLine(".");
}

// /** HELP command.
//     - References: RFC 977, RFC 3977
//     - Syntax: HELP

//     Responses:
//     - 100 help follows (multi-line) */
void
server::nntp::LineHandler::handleHelp(afl::net::line::LineSink& response)
{
    response.handleLine("100 Help");
    response.handleLine("c2nntp-ng implements the following commands:");
    response.handleLine("  AUTHINFO {USER user|PASS pass}");
    response.handleLine("  QUIT");
    response.handleLine(".");
}

/** OVER / XOVER command.
    - References: RFC 3977, RFC 2980, hamsrv
    - Syntax: OVER [range]
    - Syntax: OVER <msgid>
    - Syntax: XOVER [range]

    Responses:
    - 224 Overview information follows (multi-line)
    - 430 No article with that message-id
    - 412 No newsgroup selected
    - 423 No articles in that range
    - 420 Current article number is invalid

    The response contains a list of header fields: sequence, Subject, From, Date, Message-Id,
    References, byte size, line count, and optional fields (Xref).

    FIXME: OVER <msgid> is not implemented.

    FIXME: special case for OVER with empty range not implemented (should give 423 instead of 224).

    \param args Parameters
    \param response Write response here */
void
server::nntp::LineHandler::handleOver(String_t args, afl::net::line::LineSink& response)
{
    // ex NntpWorker::handleOver
    if (!checkAuth(response)) {
        return;
    }

    // Must be in a forum
    if (m_session.current_forum == 0) {
        response.handleLine(NOT_IN_GROUP);
        return;
    }

    // Resolve range
    int32_t min, max;
    if (!parseRange(eatWord(args), min, max, response)) {
        return;
    }

    // Build the request
    afl::data::IntegerList_t req, seqNrs;
    std::map<int32_t, int32_t>::iterator i = m_session.current_seq_map.lower_bound(min), e = m_session.current_seq_map.end();
    while (i != e && i->first <= max) {
        req.push_back(i->second);
        seqNrs.push_back(i->first);
        ++i;
    }

    // Do it
    afl::data::Segment results;
    TalkNNTPClient(m_root.talk()).getMessageHeader(req, results);
    response.handleLine("224 Overview follows");
    for (size_t i = 0; i < results.size() && i < seqNrs.size(); ++i) {
        afl::data::Access a(results[i]);
        if (a.getValue() != 0) {
            // Values
            String_t values[countof(OVERVIEW_FIELDS)];

            // This used to extract the :Seq header field for the article number.
            // This is no longer reliable in the presence of cross-posting;
            // thus, we always use the article number we obtained from the current_seq_map.
            const int32_t articleNumber = seqNrs[i];

            // Iterate over keys
            afl::data::StringList_t keys;
            a.getHashKeys(keys);
            for (size_t j = 0; j < keys.size(); ++j) {
                const String_t& fieldName = keys[j];
                for (size_t fieldIndex = 0; fieldIndex < countof(OVERVIEW_FIELDS); ++fieldIndex) {
                    if (afl::string::strCaseCompare(fieldName, OVERVIEW_FIELDS[fieldIndex]) == 0) {
                        values[fieldIndex] = sanitizeFieldValue(a(fieldName).toString());
                        break;
                    }
                }
            }

            String_t line = Format("%d", articleNumber);
            for (size_t fieldIndex = 0; fieldIndex < countof(OVERVIEW_FIELDS); ++fieldIndex) {
                line += "\t";
                if (fieldIndex >= OVERVIEW_FIELDS_FIRST_FULL) {
                    line += OVERVIEW_FIELDS[fieldIndex];
                    line += ": ";
                }
                line += values[fieldIndex];
            }
            response.handleLine(line);
        }
    }
    response.handleLine(".");
}
