/**
  *  \file game/nu/browserhandler.cpp
  *  \brief Class game::nu::BrowserHandler
  */

#include "game/nu/browserhandler.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/data/access.hpp"
#include "afl/data/defaultvaluefactory.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/bufferedstream.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/json/parser.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/net/parameterencoder.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/browser/account.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/usercallback.hpp"
#include "game/nu/accountfolder.hpp"
#include "game/nu/gamefolder.hpp"

using afl::string::Format;
using afl::sys::LogListener;
using game::browser::UserCallback;

namespace {
    const char LOG_NAME[] = "game.nu";

    String_t buildUrl(const game::browser::Account& acc)
    {
        String_t url = acc.get("url", "https://api." + acc.get("host", "planets.nu") + "/");
        if (!url.empty() && url[url.size()-1] == '/') {
            url.erase(url.size()-1);
        }
        return url;
    }
}

class game::nu::BrowserHandler::LoginTask : public Task_t {
 public:
    LoginTask(BrowserHandler& parent, game::browser::Account& acc, std::auto_ptr<Task_t>& then)
        : m_parent(parent), m_account(acc), m_then(then),
          conn_passwordResult(parent.m_browser.callback().sig_passwordResult.add(this, &LoginTask::onPasswordResult))
        { }
    virtual void call()
        {
            // Nothing to do if already logged in
            if (m_account.get("api_key") != 0) {
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
                m_parent.log().write(LogListener::Warn, LOG_NAME, m_parent.translator()("Login canceled"));
                m_then->call();
                return;
            }

            // Try to log in
            afl::net::HeaderTable tab;
            tab.set("username", m_account.getUser());
            tab.set("password", resp.password);
            std::auto_ptr<afl::data::Value> result(m_parent.callServer(m_account, "/account/login?version=2", tab));
            if (result.get() == 0) {
                m_parent.log().write(LogListener::Error, LOG_NAME, m_parent.translator()("Login failed"));
                m_then->call();
                return;
            }

            afl::data::Access parsedResult(result);
            if (!parsedResult("success").toInteger()) {
                m_parent.log().write(LogListener::Error, LOG_NAME, m_parent.translator()("Login did not succeed; wrong password?"));
                m_then->call();
                return;
            }
            m_account.set("api_key", parsedResult("apikey").toString(), false);
            m_then->call();
        }

 private:
    BrowserHandler& m_parent;
    game::browser::Account& m_account;
    std::auto_ptr<Task_t> m_then;
    afl::base::SignalConnection conn_passwordResult;
};


game::nu::BrowserHandler::BrowserHandler(game::browser::Browser& b, afl::net::http::Manager& mgr, afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory)
    : m_browser(b),
      m_manager(mgr),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_gameList(),
      m_gameListAccount(),
      m_accountInfo(),
      m_accountInfoAccount()
{ }

bool
game::nu::BrowserHandler::handleFolderName(String_t /*name*/, afl::container::PtrVector<game::browser::Folder>& /*result*/)
{
    // TODO: recognize URLs such as "https://planets.nu/#/sector/182370" and construct virtual path.
    return false;
}

game::browser::Folder*
game::nu::BrowserHandler::createAccountFolder(game::browser::Account& acc)
{
    if (acc.isValid() && acc.getType() == "nu") {
        return new AccountFolder(*this, acc);
    } else {
        return 0;
    }
}

std::auto_ptr<game::Task_t>
game::nu::BrowserHandler::loadGameRootMaybe(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t>& then)
{
    if (config.getGameType() == "nu") {
        // Verify that we know the account
        game::browser::Account* a = m_browser.accounts().findAccount(config[config.Game_User](), config.getGameType(), config[config.Game_Host]());
        if (!a) {
            return std::auto_ptr<Task_t>();
        }
        int32_t gameId;
        if (!afl::string::strToInteger(config[config.Game_Id](), gameId)) {
            return std::auto_ptr<Task_t>();
        }

        // FIXME: verify that the game Id is valid

        // Record this mapping
        a->setGameFolderName(Format("%d", gameId), dir->getDirectoryName());

        // Use temporary GameFolder to load the game
        return GameFolder(*this, *a, gameId, 0).loadGameRoot(config, then);
    } else {
        return std::auto_ptr<Task_t>();
    }
}

std::auto_ptr<game::Task_t>
game::nu::BrowserHandler::login(game::browser::Account& acc, std::auto_ptr<Task_t> then)
{
    return std::auto_ptr<Task_t>(new LoginTask(*this, acc, then));
}

std::auto_ptr<afl::data::Value>
game::nu::BrowserHandler::callServer(game::browser::Account& acc,
                                     String_t endpoint,
                                     const afl::net::HeaderTable& args)
{
    // Build URL
    String_t url = buildUrl(acc);
    url += endpoint;

    afl::net::Url parsedUrl;
    if (!parsedUrl.parse(url)) {
        log().write(LogListener::Error, LOG_NAME, Format(translator()("Malformed URL \"%s\""), url));
        return std::auto_ptr<afl::data::Value>();
    }
    log().write(LogListener::Trace, LOG_NAME, Format("Calling \"%s\"", url));

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
        log().write(LogListener::Error, LOG_NAME, Format(translator()("%s: network access failed (%s)"), url, toString(listener.getFailureReason())));
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

        log().write(LogListener::Trace, LOG_NAME, Format("at byte %d, \"%s\"", pos, afl::string::fromBytes(bytes)));
        return std::auto_ptr<afl::data::Value>();
    }
}

afl::data::Access
game::nu::BrowserHandler::getGameListPreAuthenticated(game::browser::Account& acc)
{
    // Cached?
    if (m_gameList.get() != 0 && m_gameListAccount == &acc) {
        return m_gameList;
    }

    // Not cached -> load it
    m_gameList.reset();
    m_gameListAccount = &acc;
    if (const String_t* key = acc.get("api_key")) {
        afl::net::HeaderTable tab;
        tab.add("apikey", *key);
        m_gameList = callServer(acc, "/account/mygames?version=2", tab);
    }
    return m_gameList;
}

afl::data::Access
game::nu::BrowserHandler::getAccountInfoPreAuthenticated(game::browser::Account& acc)
{
    // Cached?
    if (m_accountInfo.get() != 0 && m_accountInfoAccount == &acc) {
        return m_accountInfo;
    }

    // Not cached -> load it
    m_accountInfo.reset();
    m_accountInfoAccount = &acc;
    if (const String_t* key = acc.get("api_key")) {
        afl::net::HeaderTable tab;
        tab.add("apikey", *key);
        m_accountInfo = callServer(acc, "/account/load?version=2", tab);
    }
    return m_accountInfo;
}

afl::string::Translator&
game::nu::BrowserHandler::translator()
{
    return m_browser.translator();
}

afl::sys::LogListener&
game::nu::BrowserHandler::log()
{
    return m_browser.log();
}

game::browser::Browser&
game::nu::BrowserHandler::browser()
{
    return m_browser;
}

afl::base::Ref<afl::io::Directory>
game::nu::BrowserHandler::getDefaultSpecificationDirectory()
{
    return m_defaultSpecificationDirectory;
}
