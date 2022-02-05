/**
  *  \file game/pcc/browserhandler.cpp
  *  \brief Class game::pcc::BrowserHandler
  */

#include "game/pcc/browserhandler.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/data/defaultvaluefactory.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/json/parser.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/net/parameterencoder.hpp"
#include "afl/string/format.hpp"
#include "game/browser/account.hpp"
#include "game/browser/usercallback.hpp"
#include "game/pcc/accountfolder.hpp"
#include "game/v3/rootloader.hpp"

using afl::string::Format;
using afl::sys::LogListener;
using game::browser::UserCallback;

namespace {
    const char LOG_NAME[] = "game.pcc";

    String_t buildUrl(const game::browser::Account& acc)
    {
        String_t url = acc.get("url", "https://" + acc.get("host", "planetscentral.com") + "/api/");
        if (url.empty() || url[url.size()-1] != '/') {
            url += '/';
        }
        return url;
    }
}


class game::pcc::BrowserHandler::LoginTask : public Task_t {
 public:
    LoginTask(BrowserHandler& parent, game::browser::Account& acc, std::auto_ptr<Task_t>& then)
        : m_parent(parent),
          m_account(acc),
          m_then(then),
          conn_passwordResult(parent.m_browser.callback().sig_passwordResult.add(this, &LoginTask::onPasswordResult))
        { }
    virtual void call()
        {
            // Already logged in?
            if (m_account.get("api_token") != 0 && m_account.get("api_user") != 0) {
                m_parent.log().write(LogListener::Trace, LOG_NAME, "Task: BrowserHandler.login: already logged in");
                m_then->call();
                return;
            }
            m_parent.log().write(LogListener::Trace, LOG_NAME, "Task: BrowserHandler.login");

            // Ask for password
            UserCallback::PasswordRequest req;
            req.accountName = m_account.getName();
            req.hasFailed = false;
            m_parent.m_browser.callback().askPassword(req);
        }

    void onPasswordResult(UserCallback::PasswordResponse resp)
        {
            if (resp.canceled) {
                m_parent.log().write(LogListener::Error, LOG_NAME, m_parent.translator()("Login canceled"));
                m_then->call();
                return;
            }

            // Try to log in
            afl::net::HeaderTable tab;
            tab.set("api_user", m_account.getUser());
            tab.set("api_password", resp.password);
            tab.set("action", "whoami");
            std::auto_ptr<afl::data::Value> result(m_parent.callServer(m_account, "user", tab));
            if (result.get() == 0) {
                m_parent.log().write(LogListener::Error, LOG_NAME, m_parent.translator()("Login failed"));
                m_then->call();
                return;
            }

            afl::data::Access parsedResult(result);
            if (!parsedResult("result").toInteger()) {
                m_parent.log().write(LogListener::Error, LOG_NAME, m_parent.translator()("Login did not succeed; wrong password?"));
                m_then->call();
                return;
            }

            m_account.setEncoded("api_token", parsedResult("api_token").toString(), false);
            m_account.setEncoded("api_user",  parsedResult("username").toString(), false);
            m_then->call();
        }

 private:
    BrowserHandler& m_parent;
    game::browser::Account& m_account;
    std::auto_ptr<Task_t> m_then;
    afl::base::SignalConnection conn_passwordResult;
};

game::pcc::BrowserHandler::BrowserHandler(game::browser::Browser& b,
                                          afl::net::http::Manager& mgr,
                                          afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                          util::ProfileDirectory& profile)
    : m_browser(b),
      m_manager(mgr),
      m_nullFS(),
      m_v3Loader(defaultSpecificationDirectory, &profile, b.translator(), b.log(), m_nullFS),
      m_gameList(),
      m_gameListAccount()
{ }

// Handler:
bool
game::pcc::BrowserHandler::handleFolderName(String_t /*name*/, afl::container::PtrVector<game::browser::Folder>& /*result*/)
{
    // TODO: if we have some URL scheme to invoke a PCC game, build the appropriate path.
    return false;
}

game::browser::Folder*
game::pcc::BrowserHandler::createAccountFolder(game::browser::Account& acc)
{
    if (acc.isValid() && acc.getType() == "pcc") {
        return new AccountFolder(*this, acc);
    } else {
        return 0;
    }
}

std::auto_ptr<game::Task_t>
game::pcc::BrowserHandler::loadGameRootMaybe(afl::base::Ref<afl::io::Directory> /*dir*/, const game::config::UserConfiguration& /*config*/, std::auto_ptr<game::browser::LoadGameRootTask_t>& /*then*/)
{
    // TODO: if this is the local directory for a server game, load that.
    // (For now, we have no local directories.)
    return std::auto_ptr<Task_t>();
}

std::auto_ptr<game::Task_t>
game::pcc::BrowserHandler::login(game::browser::Account& acc, std::auto_ptr<Task_t> then)
{
    return std::auto_ptr<Task_t>(new LoginTask(*this, acc, then));
}

std::auto_ptr<afl::data::Value>
game::pcc::BrowserHandler::callServer(game::browser::Account& acc,
                                      String_t endpoint,
                                      const afl::net::HeaderTable& args)
{
    // Build URL
    String_t url = buildUrl(acc);
    url += endpoint;
    url += ".cgi";

    afl::net::Url parsedUrl;
    if (!parsedUrl.parse(url)) {
        log().write(LogListener::Error, LOG_NAME, Format(translator()("Malformed URL \"%s\""), url));
        return std::auto_ptr<afl::data::Value>();
    }
    log().write(LogListener::Trace, LOG_NAME, Format(translator()("Calling \"%s\""), url));

    // Build query
    String_t query;
    afl::net::ParameterEncoder fmt(query, '\0');
    args.enumerateHeaders(fmt);

    // Call it
    afl::net::http::SimpleDownloadListener listener;
    m_manager.postFile(parsedUrl, query, "application/x-www-form-urlencoded; charset=UTF-8", listener);
    switch (listener.wait()) {
     case afl::net::http::SimpleDownloadListener::Succeeded:
        break;
     case afl::net::http::SimpleDownloadListener::Failed:
        log().write(LogListener::Error, LOG_NAME, Format(translator()("%s: network access failed"), url));
        return std::auto_ptr<afl::data::Value>();
     case afl::net::http::SimpleDownloadListener::TimedOut:
        log().write(LogListener::Error, LOG_NAME, Format(translator()("%s: network access timed out"), url));
        return std::auto_ptr<afl::data::Value>();
     case afl::net::http::SimpleDownloadListener::LimitExceeded:
        log().write(LogListener::Error, LOG_NAME, Format(translator()("%s: network access exceeded limit"), url));
        return std::auto_ptr<afl::data::Value>();
    }

    // Parse JSON
    afl::data::DefaultValueFactory factory;
    afl::io::ConstMemoryStream cms(listener.getResponseData());
    afl::io::BufferedStream buf(cms);
    try {
        return std::auto_ptr<afl::data::Value>(afl::io::json::Parser(buf, factory).parseComplete());
    }
    catch (std::exception& e) {
        log().write(LogListener::Error, LOG_NAME, Format(translator()("%s: received invalid data from network"), url));
        log().write(LogListener::Info,  LOG_NAME, translator()("Parse error"), e);

        // Log failing fragment
        afl::io::Stream::FileSize_t pos = buf.getPos();
        if (pos > 0) {
            --pos;
            buf.setPos(pos);
        }
        uint8_t tmp[30];
        afl::base::Bytes_t bytes(tmp);
        bytes.trim(buf.read(bytes));

        log().write(LogListener::Trace, LOG_NAME, Format(translator()("at byte %d, \"%s\""), pos, afl::string::fromBytes(bytes)));
        return std::auto_ptr<afl::data::Value>();
    }
}

afl::data::Access
game::pcc::BrowserHandler::getGameListPreAuthenticated(game::browser::Account& acc)
{
    // Cached?
    if (m_gameList.get() != 0 && m_gameListAccount == &acc) {
        return m_gameList;
    }

    m_gameList.reset();
    m_gameListAccount = &acc;

    String_t token;
    String_t user;
    if (acc.getEncoded("api_token", token) && acc.getEncoded("api_user", user)) {
        afl::net::HeaderTable tab;
        tab.set("api_token", token);
        tab.set("dir", "u/" + user);
        tab.set("action", "lsgame");
        m_gameList = callServer(acc, "file", tab);
    }
    return m_gameList.get();
}

std::auto_ptr<afl::data::Value>
game::pcc::BrowserHandler::getDirectoryContentPreAuthenticated(game::browser::Account& acc, String_t dirName)
{
    String_t token;
    std::auto_ptr<afl::data::Value> result;
    if (acc.getEncoded("api_token", token)) {
        afl::net::HeaderTable tab;
        tab.set("api_token", token);
        tab.set("dir", dirName);
        tab.set("action", "ls");
        result = callServer(acc, "file", tab);
    }
    return result;
}

void
game::pcc::BrowserHandler::getFilePreAuthenticated(game::browser::Account& acc, String_t fileName, afl::net::http::DownloadListener& listener)
{
    // Build URL to download
    afl::net::Url mainUrl;
    afl::net::Url fileUrl;
    if (!mainUrl.parse(buildUrl(acc)) || !fileUrl.parse(fileName)) {
        listener.handleFailure(afl::net::http::ClientRequest::UnsupportedProtocol, translator()("Invalid URL"));
        return;
    }
    fileUrl.mergeFrom(mainUrl);
    log().write(LogListener::Trace, LOG_NAME, Format(translator()("Downloading \"%s\""), fileUrl.toString()));

    String_t token;
    if (acc.getEncoded("api_token", token)) {
        // Attach token to URL
        String_t filePath(fileUrl.getPath());
        afl::net::ParameterEncoder(filePath, '?').handleHeader("api_token", token);
        fileUrl.setPath(filePath);

        // Download the file
        m_manager.getFile(fileUrl, listener);
    } else {
        // Immediately fail non-logged-in request
        listener.handleFailure(afl::net::http::ClientRequest::ServerError, translator()("Not logged in"));
    }
}

afl::string::Translator&
game::pcc::BrowserHandler::translator()
{
    return m_browser.translator();
}

afl::sys::LogListener&
game::pcc::BrowserHandler::log()
{
    return m_browser.log();
}

game::v3::RootLoader&
game::pcc::BrowserHandler::loader()
{
    return m_v3Loader;
}
