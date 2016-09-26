/**
  *  \file game/nu/browserhandler.cpp
  */

#include "game/nu/browserhandler.hpp"
#include "afl/data/access.hpp"
#include "afl/data/defaultvaluefactory.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/bufferedstream.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/json/parser.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/net/parameterencoder.hpp"
#include "afl/string/format.hpp"
#include "game/browser/account.hpp"
#include "game/browser/usercallback.hpp"
#include "game/nu/accountfolder.hpp"

namespace {
    const char LOG_NAME[] = "game.nu";

    using afl::sys::LogListener;
    using afl::string::Format;

    String_t buildUrl(const game::browser::Account& acc)
    {
        String_t url = acc.get("url", "http://api." + acc.get("host", "planets.nu") + "/");
        if (!url.empty() && url[url.size()-1] == '/') {
            url.erase(url.size()-1);
        }
        return url;
    }
}


game::nu::BrowserHandler::BrowserHandler(game::browser::Browser& b, afl::net::http::Manager& mgr, afl::base::Ptr<afl::io::Directory> defaultSpecificationDirectory)
    : m_browser(b),
      m_manager(mgr),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_gameList(),
      m_gameListAccount()
{ }

bool
game::nu::BrowserHandler::handleFolderName(String_t /*name*/, afl::container::PtrVector<game::browser::Folder>& /*result*/)
{
    // FIXME: do we need this? When given a virtual folder name, construct matching path.
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

afl::base::Ptr<game::Root>
game::nu::BrowserHandler::loadGameRoot(afl::base::Ptr<afl::io::Directory> /*dir*/)
{
    // FIXME: do we need this? Look at folder content, if it's nu, open it.
    return 0;
}

bool
game::nu::BrowserHandler::login(game::browser::Account& acc)
{
    // Ask for password
    afl::data::Segment answer;
    std::vector<game::browser::UserCallback::Element> vec(1);
    vec[0].type = game::browser::UserCallback::AskPassword;
    vec[0].prompt = Format(translator().translateString("Password for %s").c_str(), acc.getName());
    if (!m_browser.callback().askInput(Format("planets.nu"), vec, answer)) {
        return false;
    }

    // Try to log in
    afl::net::HeaderTable tab;
    tab.set("username", acc.getUser());
    tab.set("password", afl::data::Access(answer[0]).toString());
    std::auto_ptr<afl::data::Value> result(callServer(acc, "/account/login?version=2", tab));
    if (result.get() == 0) {
        log().write(LogListener::Error, LOG_NAME, translator().translateString("Login failed"));
        return false;
    }

    afl::data::Access parsedResult(result);
    if (!parsedResult("success").toInteger()) {
        log().write(LogListener::Error, LOG_NAME, translator().translateString("Login did not succeed; wrong password?"));
        return false;
    }
    acc.set("apikey", parsedResult("apikey").toString(), false);
    return true;
}

// endpoint must start with a "/"
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
        log().write(LogListener::Error, LOG_NAME, Format(translator().translateString("Malformed URL \"%s\"").c_str(), url));
        return std::auto_ptr<afl::data::Value>();
    }
    log().write(LogListener::Trace, LOG_NAME, Format(translator().translateString("Calling \"%s\"").c_str(), url));

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
        log().write(LogListener::Error, LOG_NAME, Format(translator().translateString("%s: network access failed").c_str(), url));
        return std::auto_ptr<afl::data::Value>();
     case afl::net::http::SimpleDownloadListener::TimedOut:
        log().write(LogListener::Error, LOG_NAME, Format(translator().translateString("%s: network access timed out").c_str(), url));
        return std::auto_ptr<afl::data::Value>();
     case afl::net::http::SimpleDownloadListener::LimitExceeded:
        log().write(LogListener::Error, LOG_NAME, Format(translator().translateString("%s: network access exceeded limit").c_str(), url));
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
        log().write(LogListener::Error, LOG_NAME, Format(translator().translateString("%s: received invalid data from network").c_str(), url));
        log().write(LogListener::Info,  LOG_NAME, translator().translateString("Parse error"), e);

        // Log failing fragment
        afl::io::Stream::FileSize_t pos = buf.getPos();
        if (pos > 0) {
            --pos;
            buf.setPos(pos);
        }
        uint8_t tmp[30];
        afl::base::Bytes_t bytes(tmp);
        bytes.trim(buf.read(bytes));

        log().write(LogListener::Trace, LOG_NAME, Format(translator().translateString("at byte %d, \"%s\"").c_str(), pos, afl::string::fromBytes(bytes)));
        return std::auto_ptr<afl::data::Value>();
    }
}

afl::data::Access
game::nu::BrowserHandler::getGameList(game::browser::Account& acc)
{
    // Cached?
    if (m_gameList.get() != 0 && m_gameListAccount == buildUrl(acc)) {
        return m_gameList;
    }

    // Not cached -> load it
    m_gameList.reset();
    m_gameListAccount = buildUrl(acc);
    if (const String_t* key = acc.get("apikey")) {
        afl::net::HeaderTable tab;
        tab.add("apikey", *key);
        m_gameList = callServer(acc, "/account/mygames?version=2", tab);
    }
    if (m_gameList.get() == 0 || !afl::data::Access(m_gameList)("games").getValue()) {
        if (login(acc)) {
            if (const String_t* key = acc.get("apikey")) {
                afl::net::HeaderTable tab;
                tab.add("apikey", *key);
                m_gameList = callServer(acc, "/account/mygames?version=2", tab);
            }
        }
    }
    return m_gameList;
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

afl::base::Ptr<afl::io::Directory>
game::nu::BrowserHandler::getDefaultSpecificationDirectory()
{
    return m_defaultSpecificationDirectory;
}
