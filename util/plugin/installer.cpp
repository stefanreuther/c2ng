/**
  *  \file util/plugin/installer.cpp
  *  \brief Class util::plugin::Installer
  */

#include "util/plugin/installer.hpp"
#include "afl/io/archive/zipreader.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "util/plugin/manager.hpp"
#include "util/plugin/plugin.hpp"
#include "util/translation.hpp"

using afl::string::Format;

namespace {
    const char LOG_NAME[] = "plugin.install";

    void copyFile(afl::io::Directory& dstDir, afl::io::Directory& srcDir, const String_t fname)
    {
        afl::base::Ref<afl::io::Stream> inFile = srcDir.openFile(fname, afl::io::FileSystem::OpenRead);
        afl::base::Ref<afl::io::Stream> outFile = dstDir.openFile(fname, afl::io::FileSystem::Create);
        outFile->copyFrom(*inFile);
    }

    bool eraseFile(afl::io::Directory& dir, const String_t& name, afl::sys::LogListener& log)
    {
        try {
            dir.erase(name);
            return true;
        }
        catch (std::exception& e) {
            log.write(afl::sys::LogListener::Warn, LOG_NAME, String_t(), e);
            return false;
        }
    }

    util::plugin::Installer::ScanResult scanDirectory(afl::io::Directory& dir, String_t& pluginName)
    {
        using util::plugin::Installer;
        Installer::ScanResult result = Installer::NoPlugin;
        afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > content = dir.getDirectoryEntries();
        afl::base::Ptr<afl::io::DirectoryEntry> elem;
        while (content->getNextElement(elem)) {
            const String_t baseName = elem->getTitle();
            if (baseName.size() > 4 && afl::string::strCaseCompare(baseName.substr(baseName.size() - 4), ".c2p") == 0) {
                if (result == Installer::NoPlugin) {
                    result = Installer::OnePlugin;
                    pluginName = baseName;
                } else {
                    result = Installer::MultiplePlugins;
                }
            }
        }
        return result;
    }
}

// Constructor.
util::plugin::Installer::Installer(Manager& mgr, afl::io::FileSystem& fs, afl::io::Directory& rootDir)
    : manager(mgr),
      m_fileSystem(fs),
      rootDir(rootDir),
      srcDir(),
      srcFile(),
      apPlug()
{
    // ex PluginInstaller::PluginInstaller
}

util::plugin::Installer::~Installer()
{ }

// Prepare installation.
util::plugin::Plugin*
util::plugin::Installer::prepareInstall(String_t fileName, afl::string::Translator& tx)
{
    // ex PluginInstaller::prepareInstall
    // Reset
    apPlug.reset();
    srcDir = 0;
    srcFile = 0;

    // Open plugin
    String_t baseName = m_fileSystem.getFileName(fileName);
    String_t dirName = m_fileSystem.getDirectoryName(fileName);
    srcFile = m_fileSystem.openFile(fileName, afl::io::FileSystem::OpenRead).asPtr();
    srcDir = m_fileSystem.openDirectory(dirName).asPtr();

    // Determine file type
    if (baseName.size() > 4 && afl::string::strCaseCompare(baseName.substr(baseName.size() - 4), ".c2p") == 0) {
        // Regular *.c2p file
        String_t pluginName = baseName.substr(0, baseName.size() - 4);
        apPlug.reset(new Plugin(afl::string::strUCase(pluginName)));
        apPlug->initFromPluginFile(dirName, baseName, *srcFile, manager.log());
        srcFile->setPos(0);
    } else if (baseName.size() > 4 && afl::string::strCaseCompare(baseName.substr(baseName.size() - 4), ".res") == 0) {
        // *.res file with synthetic definition
        String_t pluginName = baseName.substr(0, baseName.size() - 4);
        apPlug.reset(new Plugin(afl::string::strUCase(pluginName)));
        apPlug->initFromResourceFile(dirName, baseName, tx);
        srcFile = 0;
    } else if (baseName.size() > 2 && afl::string::strCaseCompare(baseName.substr(baseName.size() - 2), ".q") == 0) {
        // *.q file with synthetic definition
        String_t pluginName = baseName.substr(0, baseName.size() - 2);
        apPlug.reset(new Plugin(afl::string::strUCase(pluginName)));
        apPlug->initFromScriptFile(dirName, baseName, *srcFile, tx);
        srcFile = 0;
    } else if (baseName.size() > 4
               && (afl::string::strCaseCompare(baseName.substr(baseName.size() - 4), ".zip") == 0
                   || afl::string::strCaseCompare(baseName.substr(baseName.size() - 4), ".c2z") == 0))
    {
        // *.zip/*.c2z: zipped plugin
        afl::base::Ref<afl::io::Directory> zip = afl::io::archive::ZipReader::open(*srcFile, 0);
        String_t pluginFile;
        if (scanDirectory(*zip, pluginFile) == OnePlugin) {
            // Read definition
            afl::base::Ref<afl::io::Stream> file = zip->openFile(pluginFile, afl::io::FileSystem::OpenRead);
            String_t pluginName = pluginFile.substr(0, pluginFile.size() - 4);
            apPlug.reset(new Plugin(afl::string::strUCase(pluginName)));
            apPlug->initFromPluginFile(fileName, pluginFile, *file, manager.log());

            // Set output
            srcFile = zip->openFile(pluginFile, afl::io::FileSystem::OpenRead).asPtr();
            srcDir = zip.asPtr();
        }
    } else {
        // skip
    }

    return apPlug.get();
}

// Check for installation ambiguities.
util::plugin::Installer::ScanResult
util::plugin::Installer::checkInstallAmbiguity(String_t& out)
{
    // ex PluginInstaller::checkInstallAmbiguity
    if (srcFile.get() == 0 && srcDir.get() != 0) {
        // We do not have a source file.
        // this means we are auto-converting and could be subject to ambiguities.
        return scanDirectory(*srcDir, out);
    } else {
        return NoPlugin;
    }
}

// Check preconditions for installation.
afl::base::Optional<String_t>
util::plugin::Installer::checkInstallPreconditions(afl::string::Translator& tx)
{
    // ex c2pluginw.cc:checkPreconditions, c2plugin.cc:checkPreconditions, plugindlg.cc:checkPreconditions
    // Do we have a plugin?
    if (apPlug.get() == 0) {
        return afl::base::Nothing;
    }
    const Plugin& plug = *apPlug;

    // Check for conflicts
    std::vector<Plugin*> plugList;
    manager.enumConflictingPlugins(plug, plugList);
    if (!plugList.empty()) {
        String_t message = Format(tx.translateString("Plugin \"%s\" conflicts with the following plugins:").c_str(), plug.getId());
        for (size_t i = 0, n = plugList.size(); i < n; ++i) {
            message += "\n";
            message += Format("  %s (%s)", plugList[i]->getId(), plugList[i]->getName());
        }
        message += "\n";
        message += tx.translateString("It cannot be installed.");
        return message;
    }

    // Check for preconditions
    Plugin::FeatureSet_t fset;
    manager.enumProvidedFeatures(fset);
    if (!plug.isSatisfiedBy(fset)) {
        String_t message = Format(tx.translateString("Plugin \"%s\" requires the following features:").c_str(), plug.getId());
        Plugin::FeatureSet_t missing;
        plug.enumMissingFeatures(fset, missing);
        for (Plugin::FeatureSet_t::const_iterator it = missing.begin(), e = missing.end(); it != e; ++it) {
            message += "\n  ";
            message += it->first;
            if (!it->second.empty()) {
                message += " ";
                message += it->second;
            }
        }
        message += "\n";
        message += tx.translateString("It cannot be installed.");
        return message;
    }
    return afl::base::Nothing;
}

// Install the prepared plugin.
void
util::plugin::Installer::doInstall(bool dry)
{
    // ex PluginInstaller::doInstall

    // Quick exit if sequence violated
    if (apPlug.get() == 0) {
        return;
    }

    // If there already is a plugin with that name, uninstall it first
    if (Plugin* originalPlugin = manager.getPluginById(apPlug->getId())) {
        doRemove(originalPlugin, dry);
    }

    if (!dry) {
        // Create directory
        afl::base::Ref<afl::io::DirectoryEntry> dirEntry = rootDir.getDirectoryEntryByName(afl::string::strLCase(apPlug->getId()));
        try {
            dirEntry->createAsDirectory();
        }
        catch (...) { }
        afl::base::Ref<afl::io::Directory> dir = dirEntry->openDirectory();

        // OK, do it
        const Plugin::ItemList_t& items = apPlug->getItems();
        for (size_t i = 0, n = items.size(); i != n; ++i) {
            const Plugin::Item& item = items[i];
            switch (item.type) {
             case Plugin::PlainFile:
             case Plugin::ScriptFile:
             case Plugin::ResourceFile:
             case Plugin::HelpFile:
                copyFile(*dir, *srcDir, item.name);
                break;

             case Plugin::Command:
                break;
            }
        }

        // Create c2p file
        afl::base::Ref<afl::io::Stream> c2pFile = rootDir.openFile(afl::string::strLCase(apPlug->getId()) + ".c2p", afl::io::FileSystem::Create);
        if (srcFile.get() != 0) {
            c2pFile->copyFrom(*srcFile);
        } else {
            apPlug->savePluginFile(*c2pFile);
        }

        // Move the plugin definition
        // (This is needed in case the PluginManager is long-lived,
        // so someone actually needs this.)
        apPlug->setBaseDirectory(dir->getDirectoryName());
    }

    // Reset
    manager.addNewPlugin(apPlug.release());
    srcFile = 0;
    srcDir  = 0;
}

// Check preconditions for removal.
afl::base::Optional<String_t>
util::plugin::Installer::checkRemovePreconditions(const Plugin& plug, afl::string::Translator& tx)
{
    std::vector<util::plugin::Plugin*> tmp;
    manager.enumDependingPlugins(plug, tmp);
    if (!tmp.empty()) {
        String_t message = Format(tx.translateString("Plugin \"%s\" is required by the following plugins:").c_str(), plug.getId());
        for (size_t i = 0, n = tmp.size(); i < n; ++i) {
            message += "\n";
            message += Format("  %s (%s)", tmp[i]->getId(), tmp[i]->getName());
        }
        message += "\n";
        message += tx.translateString("It cannot be uninstalled.");
        return message;
    }
    return afl::base::Nothing;
}

// Remove a plugin.
bool
util::plugin::Installer::doRemove(Plugin* pPlug, bool dry)
{
    // ex PluginInstaller::doRemove
    // Remove the plugin from the manager
    std::auto_ptr<Plugin> apPlug(manager.extractPlugin(pPlug));

    // Early exit
    if (apPlug.get() == 0 || dry) {
        return true;
    }

    // Remove
    bool err = false;
    try {
        // Remove the plugin directory content
        afl::base::Ref<afl::io::Directory> dir = m_fileSystem.openDirectory(apPlug->getBaseDirectory());
        const Plugin::ItemList_t& items = apPlug->getItems();
        for (size_t ii = 0, nn = items.size(); ii < nn; ++ii) {
            const Plugin::Item& item = items[ii];
            switch (item.type) {
             case Plugin::PlainFile:
             case Plugin::ScriptFile:
             case Plugin::ResourceFile:
             case Plugin::HelpFile:
                if (!eraseFile(*dir, item.name, manager.log())) {
                    err = true;
                }
                break;

             case Plugin::Command:
                break;
            }
        }

        // Remove the plugin definition file
        const String_t& file = apPlug->getDefinitionFileName();
        if (!file.empty()) {
            if (!eraseFile(rootDir, file, manager.log())) {
                err = true;
            }
        }

        // Remove the plugin directory
        String_t dirName = afl::string::strLCase(apPlug->getId());
        if (!eraseFile(rootDir, dirName, manager.log())) {
            err = true;
        }
    }
    catch (std::exception& e) {
        manager.log().write(afl::sys::LogListener::Error, LOG_NAME, String_t(), e);
        err = true;
    }
    if (err) {
        manager.log().write(afl::sys::LogListener::Warn, LOG_NAME, Format(_("Uninstallation of \"%s\" might be incomplete.").c_str(), apPlug->getId()));
    }
    return !err;
}
