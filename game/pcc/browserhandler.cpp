/**
  *  \file game/pcc/browserhandler.cpp
  *  \brief Class game::pcc::BrowserHandler
  */

#include "game/pcc/browserhandler.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/defaultvaluefactory.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/json/parser.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/net/mimebuilder.hpp"
#include "afl/net/parameterencoder.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/posixfilenames.hpp"
#include "game/browser/account.hpp"
#include "game/browser/usercallback.hpp"
#include "game/pcc/accountfolder.hpp"
#include "game/pcc/serverdirectory.hpp"
#include "game/pcc/turnloader.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/resultloader.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/v3/stringverifier.hpp"
#include "game/v3/utils.hpp"

using afl::base::Ref;
using afl::io::Directory;
using afl::string::Format;
using afl::sys::LogListener;
using game::browser::UserCallback;
using afl::net::http::SimpleDownloadListener;

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

    game::PlayerSet_t getAvailablePlayers(afl::data::Access a)
    {
        afl::data::StringList_t keys;
        a.getHashKeys(keys);

        game::PlayerSet_t result;
        for (size_t i = 0, n = keys.size(); i < n; ++i) {
            int playerNr = 0;
            if (afl::string::strToInteger(keys[i], playerNr) && playerNr > 0 && playerNr <= game::v3::structures::NUM_PLAYERS) {
                result += playerNr;
            }
        }
        return result;
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
      m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_profile(profile),
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
    SimpleDownloadListener listener;
    m_manager.postFile(parsedUrl, query, "application/x-www-form-urlencoded; charset=UTF-8", listener);
    return processResult(url, listener);
}

std::auto_ptr<afl::data::Value>
game::pcc::BrowserHandler::callServerWithFile(game::browser::Account& acc, String_t endpoint, const afl::net::HeaderTable& args, String_t fileParam, String_t fileName, afl::base::ConstBytes_t fileContent)
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
    afl::net::MimeBuilder builder("");
    builder.addFormFields(args);
    builder.addFormFile(fileParam, fileName);
    builder.addHeader("Content-Type", "application/octet-stream");
    builder.addRawData(fileContent);
    builder.addBoundary();
    String_t boundary = builder.finish();
    builder.removeInitialHeaders();

    // Serialize
    afl::io::InternalSink query;
    builder.write(query, false);

    // Call it
    SimpleDownloadListener listener;
    m_manager.postFile(parsedUrl, afl::string::fromBytes(query.getContent()), "multipart/form-data; boundary=" + boundary, listener);
    return processResult(url, listener);
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
    if (acc.getEncoded("api_token").get(token) && acc.getEncoded("api_user").get(user)) {
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
    if (acc.getEncoded("api_token").get(token)) {
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
    if (acc.getEncoded("api_token").get(token)) {
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

std::auto_ptr<afl::data::Value>
game::pcc::BrowserHandler::putFilePreAuthenticated(game::browser::Account& acc, String_t fileName, afl::base::ConstBytes_t content)
{
    String_t token;
    if (acc.getEncoded("api_token").get(token)) {
        afl::net::HeaderTable tab;
        tab.set("api_token", token);
        tab.set("action", "put");
        tab.set("file", fileName);

        return callServerWithFile(acc, "file", tab, "data", afl::string::PosixFileNames().getFileName(fileName), content);
    } else {
        return std::auto_ptr<afl::data::Value>();
    }
}

std::auto_ptr<afl::data::Value>
game::pcc::BrowserHandler::uploadTurnPreAuthenticated(game::browser::Account& acc, int32_t hostGameNumber, int slot, afl::base::ConstBytes_t content)
{
    String_t token;
    if (acc.getEncoded("api_token").get(token)) {
        afl::net::HeaderTable tab;
        tab.set("api_token", token);
        tab.set("action", "trn");
        tab.set("gid", Format("%d", hostGameNumber));
        tab.set("slot", Format("%d", slot));

        return callServerWithFile(acc, "host", tab, "data", Format("player%d.trn", slot), content);
    } else {
        return std::auto_ptr<afl::data::Value>();
    }
}

void
game::pcc::BrowserHandler::markTurnTemporaryPreAuthenticated(game::browser::Account& acc, int32_t hostGameNumber, int slot, int flag)
{
    String_t token;
    if (acc.getEncoded("api_token").get(token)) {
        afl::net::HeaderTable tab;
        tab.set("api_token", token);
        tab.set("action", "trnmarktemp");
        tab.set("gid", Format("%d", hostGameNumber));
        tab.set("slot", Format("%d", slot));
        tab.set("istemp", Format("%d", flag));
        callServer(acc, "host", tab);
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

game::browser::UserCallback&
game::pcc::BrowserHandler::callback()
{
    return m_browser.callback();
}

afl::base::Ptr<game::Root>
game::pcc::BrowserHandler::loadRoot(game::browser::Account& account, afl::data::Access gameListEntry, const game::config::UserConfiguration& config)
{
    afl::string::Translator& tx = m_browser.translator();
    afl::sys::LogListener& log = m_browser.log();
    afl::charset::CodepageCharset charset(afl::charset::g_codepageLatin1);

    // gameListEntry:
    //    {
    //      conflict: [],
    //      finished: 0,
    //      game: 2,
    //      hosttime: 0,
    //      hostversion: "PHost 4.1h",
    //      missing: [],
    //      name: "My Other New Game",
    //      path: "u/streu/games/2-my-other-new-game",
    //      races: {
    //        1: "The Solar Federation",
    //        2: "The Lizard Alliance",
    //        3: "The Empire of the Birds",
    //        11: "The Missing Colonies of Man"
    //      }
    //    },

    PlayerSet_t availablePlayers = getAvailablePlayers(gameListEntry("races"));

    afl::base::Ptr<Root> result;
    if (!availablePlayers.empty()) {
        // Server directory
        Ref<ServerDirectory> serverDirectory(*new ServerDirectory(*this, account, gameListEntry("path").toString()));

        // Local directory
        Ref<Directory> localDirectory(afl::io::InternalDirectory::create("<internal>"));

        // Specification directory
        Ref<afl::io::MultiDirectory> spec = afl::io::MultiDirectory::create();
        spec->addDirectory(serverDirectory);
        spec->addDirectory(m_defaultSpecificationDirectory);

        // Registration key: load from server
        std::auto_ptr<game::v3::RegistrationKey> key(new game::v3::RegistrationKey(std::auto_ptr<afl::charset::Charset>(charset.clone())));
        key->initFromDirectory(*serverDirectory, log, tx);

        // Specification loader: load from spec (server, then default)
        Ref<game::v3::SpecificationLoader> specLoader(*new game::v3::SpecificationLoader(spec, std::auto_ptr<afl::charset::Charset>(charset.clone()), tx, log));

        // Actions
        Root::Actions_t actions;
        actions += Root::aLoadEditable;         // TODO...
        actions += Root::aConfigureCharset;
        actions += Root::aConfigureFinished;
        actions += Root::aConfigureReadOnly;
        actions += Root::aSweep;

        // Host version: default to PHost 4.0
        HostVersion host(HostVersion::PHost, MKVERSION(4, 0, 0));
        host.fromString(gameListEntry("hostversion").toString());

        // Produce result
        result = new Root(localDirectory, specLoader, host,
                          std::auto_ptr<game::RegistrationKey>(key),
                          std::auto_ptr<game::StringVerifier>(new game::v3::StringVerifier(std::auto_ptr<afl::charset::Charset>(charset.clone()))),
                          std::auto_ptr<afl::charset::Charset>(charset.clone()),
                          Root::Actions_t(actions));

        // Configuration: load from server
        game::v3::Loader(charset, tx, log).loadConfiguration(*result, *serverDirectory);

        // Race names: load from spec (server, then default)
        game::v3::loadRaceNames(result->playerList(), *spec, charset);

        // Preferences: load from profile
        result->userConfiguration().loadUserConfiguration(m_profile, log, tx);
        result->userConfiguration().merge(config);

        // Turn loader
        result->setTurnLoader(new TurnLoader(localDirectory,
                                             m_defaultSpecificationDirectory,
                                             serverDirectory,
                                             gameListEntry("game").toInteger(),
                                             std::auto_ptr<afl::charset::Charset>(charset.clone()),
                                             tx, log,
                                             availablePlayers,
                                             m_profile));
    }
    return result;
}

std::auto_ptr<afl::data::Value>
game::pcc::BrowserHandler::processResult(const String_t& url, afl::net::http::SimpleDownloadListener& listener)
{
    switch (listener.wait()) {
     case SimpleDownloadListener::Succeeded:
        break;
     case SimpleDownloadListener::Failed:
        log().write(LogListener::Error, LOG_NAME, Format(translator()("%s: network access failed (%s)"), url, toString(listener.getFailureReason())));
        return std::auto_ptr<afl::data::Value>();
     case SimpleDownloadListener::TimedOut:
        log().write(LogListener::Error, LOG_NAME, Format(translator()("%s: network access timed out"), url));
        return std::auto_ptr<afl::data::Value>();
     case SimpleDownloadListener::LimitExceeded:
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
