/**
  *  \file server/mailout/template.cpp
  *  \brief Class server::mailout::Template
  *
  *  FIXME: This is a very rough port that needs some love.
  *  Actually, we'd need to merge the template engines used in CGI, monitor and this one into one.
  */

#include <stdexcept>
#include "server/mailout/template.hpp"
#include "afl/net/resp/client.hpp"
#include "afl/net/url.hpp"
#include "afl/string/format.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/ports.hpp"
#include "util/string.hpp"

using afl::string::Format;

namespace {
    bool checkCondition(const String_t& str)
    {
        // Quick & simple: the condition is true if there is any non-zero character in it.
        // No expressions yet.
        return str.find_first_not_of("0 \t") != String_t::npos;
    }

    String_t eatWord(String_t& str)
    {
        String_t::size_type i = str.find_first_not_of(" \t");
        if (i >= str.size()) {
            return String_t();
        }
        String_t::size_type j = str.find_first_of(" \t", i);
        String_t result;
        if (j >= str.size()) {
            result.assign(str, i, str.size()-i);
            str.clear();
        } else {
            result.assign(str, i, j-i);
            str.erase(0, j);
        }
        return result;
    }

    /** Generate an attachment.
        \param forUser user to use for authentication
        \param result generate the attachment here
        \param u URL to fetch
        \param net NetworkStack instance */
    void generateAttachment(String_t forUser, afl::net::MimeBuilder& result, const afl::net::Url& u, afl::net::NetworkStack& net)
    {
        if (u.getScheme() == "c2file") {
            // Parameters
            afl::net::Name name = u.getName(afl::string::Format("%d", server::HOSTFILE_PORT));
            String_t user = u.getUser();
            if (user.empty()) {
                user = forUser;
            }

            // Do it
            afl::net::resp::Client client(net, name);
            server::interface::BaseClient(client).setUserContext(user);

            String_t content = server::interface::FileBaseClient(client).getFile(u.getPath().substr(1));
            result.addBase64(afl::string::toBytes(content));
        } else {
            // We only speak c2file protocol so far
            throw std::runtime_error(Format("unsupported protocol '%s' in attachment URL", u.getScheme()));
        }
    }

    String_t getMimeType(String_t basename)
    {
        String_t::size_type i = basename.rfind('.');
        if (i < basename.size()) {
            /* This is the same repertoire as in file.cgi as of 02/Apr/2012. */
            String_t ext = afl::string::strLCase(basename.substr(i+1));
            if (ext == "ini" || ext == "src" || ext == "txt" || ext == "cfg" || ext == "log" || ext == "q" || ext == "frag") {
                return "text/plain; charset=ISO-8859-1";
            } else if (ext == "html" || ext == "htm") {
                return "text/html";
            } else if (ext == "png") {
                return "image/png";
            } else if (ext == "gif") {
                return "image/gif";
            } else if (ext == "jpg" || ext == "jpeg") {
                return "image/jpeg";
            } else if (ext == "bmp") {
                return "image/bmp";
            } else if (ext == "zip") {
                return "application/zip";
            } else {
                return "application/octet-stream";
            }
        } else {
            return "application/octet-stream";
        }
    }
}

struct server::mailout::Template::ConditionState {
    uint32_t disabled;

    ConditionState()
        : disabled(0)
        { }
};


/******************************** Template *******************************/

// Default constructor.
server::mailout::Template::Template()
    : m_variables(),
      m_attachments()
{
    // ex Template::Template
}

// Destructor.
server::mailout::Template::~Template()
{ }

// Add a variable value for expansion.
void
server::mailout::Template::addVariable(String_t name, String_t value)
{
    // ex Template::addVariable
    m_variables[name] = value;
}

// Add a file as attachment.
void
server::mailout::Template::addFile(String_t url)
{
    // ex Template::addFile
    m_attachments.push_back(url);
}

// Build message from configured parameters.
std::auto_ptr<afl::net::MimeBuilder>
server::mailout::Template::generate(afl::io::TextReader& in, afl::net::NetworkStack& net, String_t forUser, String_t smtpAddress)
{
    // ex Template::generate
    bool haveAttachments = !m_attachments.empty();

    // With no attachments, create single-part MimeBuilder
    std::auto_ptr<afl::net::MimeBuilder> result(new afl::net::MimeBuilder(haveAttachments ? "multipart/mixed" : ""));

    // Generate header.
    String_t bodyContentType = "text/plain; charset=UTF-8";
    String_t bodyCTE = "quoted-printable";
    String_t line;
    ConditionState cond;
    while (in.readLine(line) && !line.empty()) {
        if (line[0] == '!') {
            processCommand(cond, line);
        } else if (!cond.disabled) {
            String_t::size_type i = line.find(':');
            if (i == line.npos) {
                throw std::runtime_error(Format("syntax error in template line '%s'", line));
            }
            String_t headerName(line, 0, i);
            String_t headerValue(afl::string::strTrim(line.substr(i+1)));
            if (afl::string::strCaseCompare("Content-Type", headerName) == 0) {
                bodyContentType = headerValue;
            } else if (afl::string::strCaseCompare("Content-Transfer-Encoding", headerName) == 0) {
                bodyCTE = headerValue;
            } else {
                result->addHeader(headerName, util::encodeMimeHeader(expand(headerValue), "UTF-8"));
            }
        }
    }
    result->addHeader("To", smtpAddress);
    if (!haveAttachments) {
        result->addHeader("Content-Type", bodyContentType);
        result->addHeader("Content-Transfer-Encoding", bodyCTE);
    }

    // Generate text body
    if (haveAttachments) {
        result->addBoundary();
        result->addHeader("Content-Type", bodyContentType);
        result->addHeader("Content-Disposition", "inline");
        result->addHeader("Content-Transfer-Encoding", bodyCTE);
    }
    while (in.readLine(line)) {
        processLine(*result, cond, line);
    }

    // Generate attachments
    if (haveAttachments) {
        for (std::list<String_t>::const_iterator p = m_attachments.begin(); p != m_attachments.end(); ++p) {
            // Parse URL
            afl::net::Url u;
            if (!u.parse(*p)) {
                throw std::runtime_error(Format("invalid attachment URL '%s'", *p));
            }

            // Do it
            String_t baseName = u.getPath();
            String_t::size_type n = baseName.rfind('/');
            if (n != String_t::npos) {
                baseName.erase(0, n+1);
            }
            result->addBoundary();
            result->addHeader("Content-Type", getMimeType(baseName));
            result->addHeader("Content-Disposition", "attachment; filename=\"" + baseName + "\"");
            result->addHeader("Content-Transfer-Encoding", "base64");
            generateAttachment(forUser, *result, u, net);
        }

        result->addBoundary();
    }

    result->finish();
    return result;
}

void
server::mailout::Template::processLine(afl::net::MimeBuilder& result, ConditionState& state, String_t text)
{
    // ex Template::processLine
    if (text.empty()) {
        // Blank line, just add it.
        if (!state.disabled) {
            result.addLineQP(String_t());
        }
    } else if (text[0] == '!') {
        // Command
        processCommand(state, text);
    } else {
        // If a nonempty line expands to empty text, ignore it.
        if (!state.disabled) {
            String_t x(expand(text));
            if (!x.empty()) {
                result.addLineQP(x);
            }
        }
    }
}

void
server::mailout::Template::processCommand(ConditionState& state, String_t text)
{
    // ex Template::processCommand
    text.erase(0, 1);
    String_t keyword = eatWord(text);
    if (keyword.empty() || keyword[0] == '-') {
        // comment
    } else if (keyword == "set") {
        String_t name = expand(eatWord(text));
        String_t value = expand(afl::string::strTrim(text));
        if (!name.empty()) {
            m_variables[name] = value;
        }
    } else if (keyword == "if") {
        state.disabled <<= 1;
        if (!checkCondition(expand(text))) {
            state.disabled |= 1;
        }
    } else if (keyword == "else") {
        state.disabled ^= 1;
    } else if (keyword == "endif") {
        state.disabled >>= 1;
    } else {
        // huh?
        throw std::runtime_error(afl::string::Format("unsupported keyword '%s' in template", keyword));
    }
}

String_t
server::mailout::Template::expand(String_t text) const
{
    // ex Template::expand
    std::vector<String_t> stack;
    stack.push_back(String_t());

    String_t::size_type n = 0;
    String_t::size_type p;
    while ((p = text.find_first_of("$)", n)) != String_t::npos) {
        stack.back().append(text, n, p-n);
        if (text[p] == ')') {
            if (stack.size() > 1) {
                std::map<String_t, String_t>::const_iterator i = m_variables.find(stack.back());
                stack.pop_back();
                if (i != m_variables.end()) {
                    stack.back().append(i->second);
                }
            } else {
                stack.back().append(1, ')');
            }
            ++p;
        } else {
            ++p;
            if (p < text.size()) {
                if (text[p] == '(') {
                    stack.push_back(String_t());
                } else {
                    stack.back().append(text, p, 1);
                }
                ++p;
            }
        }
        n = p;
    }
    stack.back().append(text, n, text.size()-n);
    return stack.front();
}
