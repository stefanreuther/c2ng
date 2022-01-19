/**
  *  \file game/pcc/browserhandler.cpp
  */

#include "game/pcc/browserhandler.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/defaultvaluefactory.hpp"
#include "afl/data/segment.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/json/parser.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/net/parameterencoder.hpp"
#include "afl/string/format.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/posixfilenames.hpp"
#include "game/browser/account.hpp"
#include "game/browser/usercallback.hpp"
#include "game/pcc/accountfolder.hpp"
#include "game/pcc/gamefolder.hpp"
#include "game/pcc/serverdirectory.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/rootloader.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/v3/stringverifier.hpp"

namespace {
    const char LOG_NAME[] = "game.pcc";

    using afl::sys::LogListener;
    using afl::string::Format;

    String_t buildUrl(const game::browser::Account& acc)
    {
        String_t url = acc.get("url", "http://" + acc.get("host", "planetscentral.com") + "/api/");
        if (url.empty() || url[url.size()-1] != '/') {
            url += '/';
        }
        return url;
    }
}


namespace {

}

game::pcc::BrowserHandler::BrowserHandler(game::browser::Browser& b,
                                          afl::net::http::Manager& mgr,
                                          afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                          util::ProfileDirectory& profile)
    : m_browser(b),
      m_manager(mgr),
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_profile(profile),
      m_nullFS(),
      m_v3Loader(defaultSpecificationDirectory, &profile, b.translator(), b.log(), m_nullFS),
      m_gameList(),
      m_gameListAccount()
{ }

// Handler:
bool
game::pcc::BrowserHandler::handleFolderName(String_t /*name*/, afl::container::PtrVector<game::browser::Folder>& /*result*/)
{
    // FIXME: do we need this? When given a virtual folder name, construct matching path.
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

std::auto_ptr<game::browser::Task_t>
game::pcc::BrowserHandler::loadGameRootMaybe(afl::base::Ref<afl::io::Directory> /*dir*/, const game::config::UserConfiguration& /*config*/, std::auto_ptr<game::browser::LoadGameRootTask_t>& /*then*/)
{
    // FIXME: If this folder is linked with a server game, load that.
    return std::auto_ptr<game::browser::Task_t>();
}

bool
game::pcc::BrowserHandler::login(game::browser::Account& acc)
{
    // Ask for password
    afl::data::Segment answer;
    std::vector<game::browser::UserCallback::Element> vec(1);
    vec[0].type = game::browser::UserCallback::AskPassword;
    vec[0].prompt = Format(translator().translateString("Password for %s").c_str(), acc.getName());
    if (!m_browser.callback().askInput(Format/*<-FIXME?*/("planetscentral.com"), vec, answer)) {
        return false;
    }

    // Try to log in
    afl::net::HeaderTable tab;
    tab.set("api_user", acc.getUser());
    tab.set("api_password", afl::data::Access(answer[0]).toString());
    tab.set("action", "whoami");
    std::auto_ptr<afl::data::Value> result(callServer(acc, "user", tab));
    if (result.get() == 0) {
        log().write(LogListener::Error, LOG_NAME, translator().translateString("Login failed"));
        return false;
    }

    afl::data::Access parsedResult(result);
    if (!parsedResult("result").toInteger()) {
        log().write(LogListener::Error, LOG_NAME, translator().translateString("Login did not succeed; wrong password?"));
        return false;
    }
    acc.setEncoded("api_token", parsedResult("api_token").toString(), false);
    acc.setEncoded("api_user",  parsedResult("username").toString(), false);
    return true;
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
game::pcc::BrowserHandler::getGameList(game::browser::Account& acc)
{
    // Cached?
    if (m_gameList.get() != 0 && m_gameListAccount == buildUrl(acc)) {
        return m_gameList;
    }

    m_gameList.reset();
    m_gameListAccount = buildUrl(acc);

    String_t token;
    String_t user;
    if (!acc.getEncoded("api_token", token) || !acc.getEncoded("api_user", user)) {
        login(acc);
    }
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
game::pcc::BrowserHandler::getDirectoryContent(game::browser::Account& acc, String_t name)
{
    String_t token;
    String_t user;
    if (!acc.getEncoded("api_token", token) || !acc.getEncoded("api_user", user)) {
        login(acc);
    }

    std::auto_ptr<afl::data::Value> result;
    if (acc.getEncoded("api_token", token) && acc.getEncoded("api_user", user)) {
        afl::net::HeaderTable tab;
        tab.set("api_token", token);
        tab.set("dir", name);
        tab.set("action", "ls");
        result = callServer(acc, "file", tab);
    }
    return result;
}

void
game::pcc::BrowserHandler::getFile(game::browser::Account& acc, String_t name, afl::net::http::DownloadListener& listener)
{
    // Build URL to download
    afl::net::Url mainUrl;
    afl::net::Url fileUrl;
    if (!mainUrl.parse(buildUrl(acc)) || !fileUrl.parse(name)) {
        listener.handleFailure(afl::net::http::ClientRequest::UnsupportedProtocol, translator().translateString("Invalid URL"));
        return;
    }
    fileUrl.mergeFrom(mainUrl);
    log().write(LogListener::Trace, LOG_NAME, Format(translator().translateString("Downloading \"%s\"").c_str(), fileUrl.toString()));

    String_t token;
    if (!acc.getEncoded("api_token", token)) {
        login(acc);
    }
    if (acc.getEncoded("api_token", token)) {
        // Attach token to URL
        String_t filePath(fileUrl.getPath());
        afl::net::ParameterEncoder(filePath, '?').handleHeader("api_token", token);
        fileUrl.setPath(filePath);

        // Download the file
        m_manager.getFile(fileUrl, listener);
    } else {
        listener.handleFailure(afl::net::http::ClientRequest::ServerError, translator().translateString("Not logged in"));
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

afl::base::Ref<afl::io::Directory>
game::pcc::BrowserHandler::getDefaultSpecificationDirectory()
{
    return m_defaultSpecificationDirectory;
}

util::ProfileDirectory&
game::pcc::BrowserHandler::profile()
{
    return m_profile;
}

game::v3::RootLoader&
game::pcc::BrowserHandler::loader()
{
    return m_v3Loader;
}
