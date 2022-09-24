/**
  *  \file game/proxy/pluginmanagerproxy.cpp
  *  \brief Class game::proxy::PluginManagerProxy
  */

#include "game/proxy/pluginmanagerproxy.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
#include "game/proxy/waitindicator.hpp"
#include "util/io.hpp"

using afl::string::Format;
using util::plugin::Installer;
using util::plugin::Plugin;

/*
 *  Trampoline
 */

class game::proxy::PluginManagerProxy::Trampoline {
 public:
    Trampoline(util::RequestSender<PluginManagerProxy> reply, Session& session);

    void requestList();
    void requestDetails(String_t id);
    void prepareInstall(const String_t& fileName, InstallInfo& result);
    void doInstall(InstallResult& result);
    void prepareRemove(const String_t& id, RemoveResult& result);
    void doRemove(const String_t& id, RemoveResult& result);
    void cancelInstallation();

 private:
    void sendList();
    void createInstaller();

    util::RequestSender<PluginManagerProxy> m_reply;
    Session& m_session;
    afl::base::Ptr<afl::io::Directory> m_pluginDirectory;
    std::auto_ptr<Installer> m_installer;
    Plugin* m_installingPlugin;
};

game::proxy::PluginManagerProxy::Trampoline::Trampoline(util::RequestSender<PluginManagerProxy> reply, Session& session)
    : m_reply(reply),
      m_session(session),
      m_pluginDirectory(),
      m_installer(),
      m_installingPlugin()
{ }

void
game::proxy::PluginManagerProxy::Trampoline::requestList()
{
    sendList();
}

void
game::proxy::PluginManagerProxy::Trampoline::requestDetails(String_t id)
{
    class Task : public util::Request<PluginManagerProxy> {
     public:
        Task(const util::plugin::Manager& mgr, const String_t& id)
            : m_result(mgr.describePlugin(mgr.getPluginById(id)))
            { }
        virtual void handle(PluginManagerProxy& proxy)
            { proxy.handleDetails(m_result); }
     private:
        Details_t m_result;
    };
    m_reply.postNewRequest(new Task(m_session.plugins(), id));
}

void
game::proxy::PluginManagerProxy::Trampoline::prepareInstall(const String_t& fileName, InstallInfo& result)
{
    afl::io::FileSystem& fs = m_session.world().fileSystem();
    try {
        // Assume error result
        result.isValid = false;

        // Create installer
        createInstaller();
        if (m_installer.get() == 0) {
            return;
        }

        // Create the plugin
        m_installingPlugin = m_installer->prepareInstall(fileName);
        if (!m_installingPlugin) {
            return;
        }
        result.fileName = fileName;
        result.fileTitle = fs.getFileName(fileName);

        // Check ambiguities
        String_t altTitle;
        result.ambiguity = m_installer->checkInstallAmbiguity(altTitle);
        switch (result.ambiguity) {
         case Installer::NoPlugin:
            break;

         case Installer::OnePlugin:
            result.altName = fs.makePathName(fs.getDirectoryName(fileName), altTitle);
            result.altTitle = altTitle;

         case Installer::MultiplePlugins:
            break;
        }

        // Conflicts
        result.conflicts = m_installer->checkInstallPreconditions();

        // Update flag and details
        result.isUpdate = m_session.plugins().getPluginById(m_installingPlugin->getId()) != 0;
        result.pluginId = m_installingPlugin->getId();
        result.pluginName = m_installingPlugin->getName();
        result.pluginDescription = afl::string::strFirst(m_installingPlugin->getDescription(), "\n");

        // When we're here, it's a success
        result.isValid = true;
    }
    catch (afl::except::FileProblemException& e) {
        result.errorMessage = Format("%s: %s", e.getFileName(), e.what());
    }
    catch (std::exception& e) {
        result.errorMessage = e.what();
    }
}

void
game::proxy::PluginManagerProxy::Trampoline::doInstall(InstallResult& result)
{
    try {
        // Check sequence
        result.isValid = false;
        if (m_installer.get() == 0) {
            return;
        }

        // Do it
        m_installer->doInstall(false);
        result.pluginId = m_installingPlugin->getId();
        result.isValid = true;

        // Reset state
        cancelInstallation();
    }
    catch (afl::except::FileProblemException& e) {
        result.errorMessage = Format("%s: %s", e.getFileName(), e.what());
    }
    catch (std::exception& e) {
        result.errorMessage = e.what();
    }
}

void
game::proxy::PluginManagerProxy::Trampoline::prepareRemove(const String_t& id, RemoveResult& result)
{
    try {
        // Assume error
        result.isValid = false;

        // Create installer
        createInstaller();
        if (m_installer.get() == 0) {
            return;
        }

        // Find plugin to remove
        Plugin* p = m_session.plugins().getPluginById(id);
        if (p == 0) {
            return;
        }

        // Check preconditions
        afl::base::Optional<String_t> errors = m_installer->checkRemovePreconditions(*p);
        if (const String_t* err = errors.get()) {
            result.errorMessage = *err;
            return;
        }

        // Success
        result.isValid = true;

        // Reset state
        cancelInstallation();
    }
    catch (afl::except::FileProblemException& e) {
        result.errorMessage = Format("%s: %s", e.getFileName(), e.what());
    }
    catch (std::exception& e) {
        result.errorMessage = e.what();
    }
}

void
game::proxy::PluginManagerProxy::Trampoline::doRemove(const String_t& id, RemoveResult& result)
{
    try {
        // Assume error
        result.isValid = false;

        // Create installer
        createInstaller();
        if (m_installer.get() == 0) {
            return;
        }

        // Find plugin to remove
        Plugin* p = m_session.plugins().getPluginById(id);
        if (p == 0) {
            return;
        }

        // Check preconditions
        if (!m_installer->doRemove(p, false)) {
            return;
        }

        // Success
        result.isValid = true;

        // Reset state
        cancelInstallation();
    }
    catch (afl::except::FileProblemException& e) {
        result.errorMessage = Format("%s: %s", e.getFileName(), e.what());
    }
    catch (std::exception& e) {
        result.errorMessage = e.what();
    }
}

void
game::proxy::PluginManagerProxy::Trampoline::cancelInstallation()
{
    m_installer.reset();
    m_installingPlugin = 0;
    m_pluginDirectory = 0;
}

void
game::proxy::PluginManagerProxy::Trampoline::sendList()
{
    class Task : public util::Request<PluginManagerProxy> {
     public:
        Task(const util::plugin::Manager& mgr)
            : m_result()
            { mgr.enumPluginInfo(m_result); }
        virtual void handle(PluginManagerProxy& proxy)
            { proxy.sig_list.raise(m_result); }
     private:
        Infos_t m_result;
    };
    m_reply.postNewRequest(new Task(m_session.plugins()));
}

void
game::proxy::PluginManagerProxy::Trampoline::createInstaller()
{
    afl::io::FileSystem& fs = m_session.world().fileSystem();

    // Release previous instance
    cancelInstallation();

    // Try to open target directory and verify that it exists
    String_t pluginDirName = m_session.getPluginDirectoryName();
    if (pluginDirName.empty()) {
        return;
    }
    util::createDirectoryTree(fs, pluginDirName);
    m_pluginDirectory = fs.openDirectory(pluginDirName).asPtr();
    m_pluginDirectory->getDirectoryEntries();

    // Create an installer
    m_installer.reset(new Installer(m_session.plugins(), fs, *m_pluginDirectory));
}


/*
 *  TrampolineFromSession
 */

class game::proxy::PluginManagerProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(game::Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<PluginManagerProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(Session& session)
        { return new Trampoline(m_reply, session); }
 private:
    util::RequestSender<PluginManagerProxy> m_reply;
};


/*
 *  PluginManagerProxy
 */

// Constructor.
game::proxy::PluginManagerProxy::PluginManagerProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply)
    : m_reply(reply, *this),
      m_request(gameSender.makeTemporary(new TrampolineFromSession(m_reply.getSender()))),
      m_detailsRequested(false),
      m_detailRequest()
{ }

// Destructor.
game::proxy::PluginManagerProxy::~PluginManagerProxy()
{ }

// Request list of plugins.
void
game::proxy::PluginManagerProxy::requestList()
{
    m_request.postRequest(&Trampoline::requestList);
}

// Request details about a plugin.
void
game::proxy::PluginManagerProxy::requestDetails(const String_t& id)
{
    m_detailRequest = id;
    sendDetailRequest();
}

// Prepare installation.
game::proxy::PluginManagerProxy::InstallInfo
game::proxy::PluginManagerProxy::prepareInstall(WaitIndicator& ind, const String_t& fileName)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const String_t& fileName, InstallInfo& result)
            : m_fileName(fileName), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.prepareInstall(m_fileName, m_result); }
     private:
        String_t m_fileName;
        InstallInfo& m_result;
    };

    InstallInfo result;
    Task t(fileName, result);
    ind.call(m_request, t);
    return result;
}

// Perform installation.
game::proxy::PluginManagerProxy::InstallResult
game::proxy::PluginManagerProxy::doInstall(WaitIndicator& ind)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(InstallResult& result)
            : m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.doInstall(m_result); }
     private:
        InstallResult& m_result;
    };

    InstallResult result;
    Task t(result);
    ind.call(m_request, t);
    return result;
}

// Prepare uninstallation.
game::proxy::PluginManagerProxy::RemoveResult
game::proxy::PluginManagerProxy::prepareRemove(WaitIndicator& ind, const String_t& id)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const String_t& id, RemoveResult& result)
            : m_id(id), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.prepareRemove(m_id, m_result); }
     private:
        String_t m_id;
        RemoveResult& m_result;
    };

    RemoveResult result;
    Task t(id, result);
    ind.call(m_request, t);
    return result;
}

// Perform uninstallation.
game::proxy::PluginManagerProxy::RemoveResult
game::proxy::PluginManagerProxy::doRemove(WaitIndicator& ind, const String_t& id)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const String_t& id, RemoveResult& result)
            : m_id(id), m_result(result)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.doRemove(m_id, m_result); }
     private:
        String_t m_id;
        RemoveResult& m_result;
    };

    RemoveResult result;
    Task t(id, result);
    ind.call(m_request, t);
    return result;
}

// Cancel installation.
void
game::proxy::PluginManagerProxy::cancelInstallation()
{
    m_request.postRequest(&Trampoline::cancelInstallation);
}

// Send request for details to game thread if there isn't one currently running.
void
game::proxy::PluginManagerProxy::sendDetailRequest()
{
    if (!m_detailsRequested) {
        if (const String_t* p = m_detailRequest.get()) {
            String_t id = *p;
            m_detailRequest.clear();
            m_detailsRequested = true;
            m_request.postRequest(&Trampoline::requestDetails, id);
        }
    }
}

// Handle detail response: publish or send next request.
void
game::proxy::PluginManagerProxy::handleDetails(const Details_t& d)
{
    m_detailsRequested = false;
    if (m_detailRequest.isValid()) {
        // New request, send that
        sendDetailRequest();
    } else {
        // No new request, publish to user
        sig_details.raise(d);
    }
}
