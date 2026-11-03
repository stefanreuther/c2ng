/**
  *  \file game/proxy/pluginmanagerproxy.cpp
  *  \brief Class game::proxy::PluginManagerProxy
  */

#include "game/proxy/pluginmanagerproxy.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
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
    void doModify(const String_t& id, ModifyResult& result, Operation op);
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
game::proxy::PluginManagerProxy::Trampoline::doModify(const String_t& id, ModifyResult& result, Operation op)
{
    try {
        // Assume error
        result.isValid = false;

        // Create installer
        createInstaller();
        if (m_installer.get() == 0) {
            return;
        }

        // Find plugin to modify
        Plugin* p = m_session.plugins().getPluginById(id);
        if (p == 0) {
            return;
        }

        // Do it
        afl::base::Optional<String_t> errors;
        bool success = true;
        switch (op) {
         case PrepareRemove:
            errors = m_installer->checkRemovePreconditions(*p);
            break;

         case DoRemove:
            success = m_installer->doRemove(p, false);
            break;

         case PrepareEnable:
            errors = m_installer->checkEnablePreconditions(*p);
            break;

         case DoEnable:
            p->setDisabled(false);
            m_installer->saveConfiguration();
            break;

         case PrepareDisable:
            errors = m_installer->checkDisablePreconditions(*p);
            break;

         case DoDisable:
            p->setDisabled(true);
            m_installer->saveConfiguration();
            break;
        }

        // Did we produce an error?
        if (const String_t* err = errors.get()) {
            result.errorMessage = *err;
            success = false;
        }

        // Success
        result.isValid = success;

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
game::proxy::PluginManagerProxy::prepareInstall(util::WaitIndicator& ind, const String_t& fileName)
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
game::proxy::PluginManagerProxy::doInstall(util::WaitIndicator& ind)
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
game::proxy::PluginManagerProxy::ModifyResult
game::proxy::PluginManagerProxy::prepareRemove(util::WaitIndicator& ind, const String_t& id)
{
    return sendModifyRequest(ind, id, PrepareRemove);
}

// Perform uninstallation.
game::proxy::PluginManagerProxy::ModifyResult
game::proxy::PluginManagerProxy::doRemove(util::WaitIndicator& ind, const String_t& id)
{
    return sendModifyRequest(ind, id, DoRemove);
}

//  Prepare enabling of a plugin.
game::proxy::PluginManagerProxy::ModifyResult
game::proxy::PluginManagerProxy::prepareEnable(util::WaitIndicator& ind, const String_t& id)
{
    return sendModifyRequest(ind, id, PrepareEnable);
}

// Enable plugin.
game::proxy::PluginManagerProxy::ModifyResult
game::proxy::PluginManagerProxy::doEnable(util::WaitIndicator& ind, const String_t& id)
{
    return sendModifyRequest(ind, id, DoEnable);
}

// Prepare disabling of a plugin.
game::proxy::PluginManagerProxy::ModifyResult
game::proxy::PluginManagerProxy::prepareDisable(util::WaitIndicator& ind, const String_t& id)
{
    return sendModifyRequest(ind, id, PrepareDisable);
}

// Disable plugin.
game::proxy::PluginManagerProxy::ModifyResult
game::proxy::PluginManagerProxy::doDisable(util::WaitIndicator& ind, const String_t& id)
{
    return sendModifyRequest(ind, id, DoDisable);
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

game::proxy::PluginManagerProxy::ModifyResult
game::proxy::PluginManagerProxy::sendModifyRequest(util::WaitIndicator& ind, const String_t& id, Operation op)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const String_t& id, ModifyResult& result, Operation op)
            : m_id(id), m_result(result), m_op(op)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.doModify(m_id, m_result, m_op); }
     private:
        String_t m_id;
        ModifyResult& m_result;
        Operation m_op;
    };

    ModifyResult result;
    Task t(id, result, op);
    ind.call(m_request, t);
    return result;
}
