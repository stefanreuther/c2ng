/**
  *  \file util/plugin/dialogapplication.cpp
  *  \brief Class util::plugin::DialogApplication
  */

#include "util/plugin/dialogapplication.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/string/format.hpp"
#include "util/plugin/installer.hpp"
#include "util/plugin/manager.hpp"
#include "util/plugin/plugin.hpp"
#include "util/profiledirectory.hpp"
#include "util/translation.hpp"

util::plugin::DialogApplication::DialogApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::sys::Dialog& dialog)
    : Application(env, fs),
      m_dialog(dialog)
{ }

void
util::plugin::DialogApplication::appMain()
{
    // ex c2pluginw.cc:main
    afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl = environment().getCommandLine();
    std::vector<String_t> items;
    String_t item;
    while (cmdl->getNextElement(item)) {
        items.push_back(item);
    }
    if (items.empty()) {
        die(translator().translateString("Missing plugin file on command line.\n\n"
                                         "You must invoke this program with one or more *.c2p/*.c2z files on the command line. "
                                         "You can also use the plugin installer in the PCC2 GUI, or the c2plugin command line tool."));
    }

    try {
        doAdd(items);
    }
    catch (afl::except::FileProblemException& e) {
        die(afl::string::Format("%s: %s", e.getFileName(), e.what()));
    }
    catch (std::exception& e) {
        die(e.what());
    }
}

bool
util::plugin::DialogApplication::checkPreconditions(Installer& inst)
{
    String_t msg;
    if (inst.checkInstallPreconditions().get(msg)) {
        m_dialog.showError(msg, windowTitle());
        return false;
    }
    return true;
}

void
util::plugin::DialogApplication::doAdd(const std::vector<String_t>& items)
{
    // ex c2pluginw.cc:doAdd
    // Create a profile
    ProfileDirectory profile(environment(), fileSystem());

    // Create plugin directory
    afl::base::Ref<afl::io::DirectoryEntry> pluginDirEntry = profile.open()->getDirectoryEntryByName("plugins");
    if (pluginDirEntry->getFileType() != afl::io::DirectoryEntry::tDirectory) {
        pluginDirEntry->createAsDirectory();
    }
    afl::base::Ref<afl::io::Directory> pluginDir = pluginDirEntry->openDirectory();

    // Create plugin manager and installer
    afl::string::Translator& tx = translator();
    Manager mgr(tx, log());
    mgr.findPlugins(*pluginDir);
    Installer installer(mgr, fileSystem(), *pluginDir);

    // Iterate
    bool err = false;
    for (size_t i = 0, n = items.size(); i < n; ++i) {
        const String_t& name = items[i];
        try {
            Plugin* plug = installer.prepareInstall(name);
            if (!plug) {
                m_dialog.showError(afl::string::Format(tx("File '%s' cannot be installed as a plugin. "
                                                          "A plugin is normally specified with a *.c2p or *.c2z file."), name),
                                   windowTitle());
                err = true;
            } else {
                bool isUpdate = mgr.getPluginById(plug->getId()) != 0;
                if (checkPreconditions(installer)) {
                    const char* tpl = isUpdate
                        ? N_("Do you want to update plugin \"%s\" (%s)?)")
                        : N_("Do you want to install plugin \"%s\" (%s)?)");
                    String_t message = afl::string::Format(tx(tpl), plug->getName(), plug->getId());
                    if (plug->getDescription().size() > 0) {
                        message += "\n\n";
                        message += plug->getDescription();
                    }
                    if (m_dialog.askYesNo(message, windowTitle())) {
                        installer.doInstall(false);

                        const char* tpl = isUpdate
                            ? N_("Plugin '%s' has been updated.")
                            : N_("Plugin '%s' has been installed.");

                        m_dialog.showInfo(afl::string::Format(tx(tpl), plug->getName()), windowTitle());
                    }
                } else {
                    err = true;
                }
            }
        }
        catch (afl::except::FileProblemException& e) {
            m_dialog.showError(afl::string::Format("%s: %s", e.getFileName(), e.what()), windowTitle());
            err = true;
        }
        catch (std::exception& e) {
            m_dialog.showError(e.what(), windowTitle());
            err = true;
        }
    }
    if (err) {
        exit(1);
    }
}

void
util::plugin::DialogApplication::die(const String_t& text)
{
    // ex c2pluginw.cc:die
    m_dialog.showError(text, windowTitle());
    exit(1);
}

String_t
util::plugin::DialogApplication::windowTitle()
{
    return translator().translateString("PCC2 Plugin Installer");
}
