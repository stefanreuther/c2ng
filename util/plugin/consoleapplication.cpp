/**
  *  \file util/plugin/consoleapplication.cpp
  *  \brief Class util::plugin::ConsoleApplication
  */

#include "util/plugin/consoleapplication.hpp"
#include "afl/base/nullenumerator.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/messages.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "util/plugin/installer.hpp"
#include "util/plugin/manager.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using afl::string::Format;

namespace {
    class NullDirectory : public Directory {
     public:
        NullDirectory()
            { }
        ~NullDirectory()
            { }
        virtual Ref<DirectoryEntry> getDirectoryEntryByName(String_t name)
            { return *new Entry(*this, name); }
        virtual Ref<afl::base::Enumerator<Ptr<DirectoryEntry> > > getDirectoryEntries()
            { return *new afl::base::NullEnumerator<Ptr<DirectoryEntry> >(); }
        virtual Ptr<Directory> getParentDirectory()
            { return new NullDirectory(); }
        virtual String_t getDirectoryName()
            { return String_t(); }
        virtual String_t getTitle()
            { return String_t(); }
        virtual void flush()
            { }
     private:
        class Entry : public DirectoryEntry {
         public:
            Entry(Ref<Directory> parent, String_t name)
                : DirectoryEntry(),
                  m_name(name),
                  m_parent(parent)
                { }
            virtual String_t getTitle()
                { return m_name; }
            virtual String_t getPathName()
                { return String_t(); }
            virtual Ref<afl::io::Stream> openFile(afl::io::FileSystem::OpenMode mode)
                {
                    if (mode == afl::io::FileSystem::Create) {
                        return *new afl::io::NullStream();
                    } else {
                        throw afl::except::FileProblemException(m_name, afl::string::Messages::fileNotFound());
                    }
                }
            virtual Ref<Directory> openDirectory()
                { return *new NullDirectory(); }
            virtual Ref<Directory> openContainingDirectory()
                { return m_parent; }
            virtual void updateInfo(uint32_t)
                { }
            virtual void doRename(String_t)
                { }
            virtual void doErase()
                { }
            virtual void doCreateAsDirectory()
                { }
            virtual void doSetFlag(FileFlag /*flag*/, bool /*value*/)
                { }
            virtual void doMoveTo(Directory& /*dir*/, String_t /*name*/)
                { }
         private:
            String_t m_name;
            Ref<Directory> m_parent;
        };
    };

    bool checkRemovePlugin(afl::io::TextWriter& out, util::plugin::Installer& inst, util::plugin::Plugin& plug)
    {
        String_t msg;
        if (inst.checkRemovePreconditions(plug).get(msg)) {
            out.writeLine(msg);
            return false;
        }
        return true;
    }

    bool checkEnablePlugin(afl::io::TextWriter& out, util::plugin::Installer& inst, util::plugin::Plugin& plug)
    {
        String_t msg;
        if (inst.checkEnablePreconditions(plug).get(msg)) {
            out.writeLine(msg);
            return false;
        }
        return true;
    }

    bool checkDisablePlugin(afl::io::TextWriter& out, util::plugin::Installer& inst, util::plugin::Plugin& plug)
    {
        String_t msg;
        if (inst.checkDisablePreconditions(plug).get(msg)) {
            out.writeLine(msg);
            return false;
        }
        return true;
    }

    bool checkPreconditions(afl::io::TextWriter& out, util::plugin::Installer& inst)
    {
        String_t msg;
        if (inst.checkInstallPreconditions().get(msg)) {
            out.writeLine(msg);
            return false;
        }
        return true;
    }
}


struct util::plugin::ConsoleApplication::Command {
    const char* name;
    void (ConsoleApplication::*func)(afl::sys::Environment::CommandLine_t& cmdl);
};

/*
 *  ManagerOperation
 */

class util::plugin::ConsoleApplication::ManagerOperation {
 public:
    void run(ConsoleApplication& app, afl::sys::Environment::CommandLine_t& cmdl)
        {
            // Create plugin manager
            Manager mgr(app.translator(), app.log());

            // FIXME: this relies on us being able to create a Directory instance for a nonexistant directory.
            Ref<Directory> dir = app.profile.open()->openDirectory("plugins");
            mgr.findPlugins(*dir);

            Installer installer(mgr, app.fileSystem(), *dir);

            // Iterate
            bool dry = false;
            bool did = false;
            bool err = false;
            bool force = false;
            afl::string::Translator& tx = app.translator();
            afl::sys::StandardCommandLineParser parser(cmdl);
            String_t text;
            bool option;
            while (parser.getNext(option, text)) {
                if (option) {
                    if (text == "n") {
                        dry = true;
                    } else if (text == "f") {
                        force = true;
                    } else {
                        app.errorExit(Format(tx("Unknown option \"-%s\""), text));
                    }
                } else {
                    did = true;
                    if (Plugin* pPlug = mgr.getPluginById(afl::string::strUCase(text))) {
                        if (!performOperation(app, installer, *pPlug, force, dry)) {
                            err = true;
                        }
                    } else {
                        app.errorOutput().writeLine(Format(tx("Plugin '%s' is not known."), text));
                        err = true;
                    }
                }
            }
            if (!did) {
                app.errorExit(Format(tx("Missing plugin name. '%s -h' for help."), app.environment().getInvocationName()));
            } else {
                finishOperation(dry, mgr, *dir);
            }
            if (err) {
                app.exit(1);
            }
        }

    virtual bool performOperation(ConsoleApplication& app, Installer& installer, Plugin& plug, bool force, bool dry) = 0;
    virtual void finishOperation(bool dry, Manager& mgr, Directory& dir) = 0;
};


/*
 *  Entry
 */

util::plugin::ConsoleApplication::ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : Application(env, fs),
      profile(env, fs)
{
    consoleLogger().setConfiguration("*@-Debug=drop", translator());
}

void
util::plugin::ConsoleApplication::appMain()
{
    // Find command
    Ref<afl::sys::Environment::CommandLine_t> cmdl = environment().getCommandLine();
    String_t command;
    if (!cmdl->getNextElement(command)) {
        errorExit(translator()("No command specified. Use 'c2plugin -h' for help."));
    } else if (const Command* cmd = findCommand(command)) {
        (this->*(cmd->func))(*cmdl);
        exit(0);
    } else {
        errorExit(translator()("Invalid command specified. Use 'c2plugin -h' for help."));
    }
}

void
util::plugin::ConsoleApplication::doList(afl::sys::Environment::CommandLine_t& cmdl)
{
    // We do not take arguments
    enum { Default, Long, Short } f = Default;
    bool ordered = false;
    bool showEnabled = true;
    bool showDisabled = true;

    afl::string::Translator& tx = translator();
    afl::sys::StandardCommandLineParser parser(cmdl);
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (!option) {
            errorExit(tx("This command does not take positional parameters"));
        }
        if (text == "l") {
            f = Long;
        } else if (text == "b") {
            f = Short;
        } else if (text == "e") {
            showEnabled = true; showDisabled = false;
        } else if (text == "d") {
            showEnabled = false; showDisabled = true;
        } else if (text == "o") {
            ordered = true;
        } else {
            errorExit(Format(tx("Unknown option \"-%s\""), text));
        }
    }

    // Create plugin manager
    Manager mgr(translator(), log());
    mgr.findPlugins(*profile.open()->openDirectory("plugins"));

    // List them
    std::vector<Plugin*> them;
    mgr.enumPlugins(them, ordered);
    if (f != Short) {
        standardOutput().writeLine(Format(tx("%d plugin%!1{s%} installed."), them.size()));
    }
    for (size_t i = 0, n = them.size(); i < n; ++i) {
        const Plugin& p = *them[i];
        if (p.isDisabled() ? showDisabled : showEnabled) {
            if (f == Short) {
                standardOutput().writeLine(p.getId());
            } else {
                standardOutput().writeLine("--------");

                // Title line
                String_t line = Format(tx("Plugin '%s': %s"), p.getId(), p.getName());
                if (p.isDisabled()) {
                    line += tx(" (disabled)");
                }
                standardOutput().writeLine(line);

                // Description
                if (!p.getDescription().empty()) {
                    standardOutput().writeLine();
                    standardOutput().writeLine(p.getDescription());
                }

                // Extra info
                if (f == Long) {
                    const Plugin::ItemList_t& items = p.getItems();
                    bool did = false;
                    for (size_t ii = 0, nn = items.size(); ii < nn; ++ii) {
                        if (items[ii].type != Plugin::Command) {
                            if (!did) {
                                standardOutput().writeLine();
                                standardOutput().writeLine(Format(tx("Files (in '%s'):"), p.getBaseDirectory()));
                                did = true;
                            }
                            standardOutput().writeLine("  " + items[ii].name);
                        }
                    }
                }
            }
        }
    }
}

void
util::plugin::ConsoleApplication::doAdd(afl::sys::Environment::CommandLine_t& cmdl)
{
    // Create plugin directory
    Ref<DirectoryEntry> pluginDirEntry = profile.open()->getDirectoryEntryByName("plugins");
    if (pluginDirEntry->getFileType() != DirectoryEntry::tDirectory) {
        pluginDirEntry->createAsDirectory();
    }
    Ref<Directory> pluginDir = pluginDirEntry->openDirectory();

    // Create plugin manager
    Manager mgr(translator(), log());
    mgr.findPlugins(*pluginDir);

    Installer installer(mgr, fileSystem(), *pluginDir);

    // Iterate
    bool dry = false;
    bool did = false;
    bool err = false;
    bool force = false;
    afl::string::Translator& tx = translator();
    afl::sys::StandardCommandLineParser parser(cmdl);
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (text == "n") {
                dry = true;
            } else if (text == "-f") {
                force = true;
            } else {
                errorExit(Format(tx("Unknown option \"-%s\""), text));
            }
        } else {
            did = true;
            try {
                Plugin* plug = installer.prepareInstall(text);
                if (!plug) {
                    errorOutput().writeLine(Format(tx("%s: Unknown file type"), text));
                    err = true;
                } else {
                    if (mgr.getPluginById(plug->getId())) {
                        standardOutput().writeLine(Format(tx("Updating plugin '%s'..."), plug->getId()));
                    } else {
                        standardOutput().writeLine(Format(tx("Installing plugin '%s'..."), plug->getId()));
                    }
                    if (force || checkPreconditions(errorOutput(), installer)) {
                        installer.doInstall(dry);
                    } else {
                        err = true;
                    }
                }
            }
            catch (afl::except::FileProblemException& e) {
                errorOutput().writeLine(Format("%s: %s", e.getFileName(), e.what()));
                err = true;
            }
            catch (std::exception& e) {
                errorOutput().writeLine(e.what());
                err = true;
            }
        }
    }
    if (!did) {
        errorExit(Format(tx("Missing name plugin or file to install. '%s -h' for help."), environment().getInvocationName()));
    }
    if (err) {
        exit(1);
    }
}

void
util::plugin::ConsoleApplication::doRemove(afl::sys::Environment::CommandLine_t& cmdl)
{
    class Remove : public ManagerOperation {
     public:
        virtual bool performOperation(ConsoleApplication& app, Installer& installer, Plugin& plug, bool force, bool dry)
            {
                afl::string::Translator& tx = app.translator();
                if (force || checkRemovePlugin(app.errorOutput(), installer, plug)) {
                    app.standardOutput().writeLine(Format(tx("Removing plugin '%s'..."), plug.getId()));
                    installer.doRemove(&plug, dry);
                    return true;
                } else {
                    return false;
                }
            }
        virtual void finishOperation(bool /*dry*/, Manager& /*mgr*/, Directory& /*dir*/)
            { }
    };
    Remove().run(*this, cmdl);
}

void
util::plugin::ConsoleApplication::doEnable(afl::sys::Environment::CommandLine_t& cmdl)
{
    class Enable : public ManagerOperation {
     public:
        virtual bool performOperation(ConsoleApplication& app, Installer& installer, Plugin& plug, bool force, bool /*dry*/)
            {
                if (force || checkEnablePlugin(app.errorOutput(), installer, plug)) {
                    plug.setDisabled(false);
                    return true;
                } else {
                    return false;
                }
            }
        virtual void finishOperation(bool dry, Manager& mgr, Directory& dir)
            {
                if (!dry) {
                    mgr.saveConfiguration(dir);
                }
            }
    };
    Enable().run(*this, cmdl);
}

void
util::plugin::ConsoleApplication::doDisable(afl::sys::Environment::CommandLine_t& cmdl)
{
    class Disable : public ManagerOperation {
     public:
        virtual bool performOperation(ConsoleApplication& app, Installer& installer, Plugin& plug, bool force, bool /*dry*/)
            {
                if (force || checkDisablePlugin(app.errorOutput(), installer, plug)) {
                    plug.setDisabled(true);
                    return true;
                } else {
                    return false;
                }
            }
        virtual void finishOperation(bool dry, Manager& mgr, Directory& dir)
            {
                if (!dry) {
                    mgr.saveConfiguration(dir);
                }
            }
    };
    Disable().run(*this, cmdl);
}

void
util::plugin::ConsoleApplication::doTest(afl::sys::Environment::CommandLine_t& cmdl)
{
    // Dummy path
    Ref<Directory> dir = *new NullDirectory();

    // Iterate
    bool did = false;
    bool err = false;
    bool verbose = false;
    afl::string::Translator& tx = translator();
    afl::sys::StandardCommandLineParser parser(cmdl);
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (text == "v") {
                verbose = true;
            } else {
                errorExit(Format(tx("Unknown option \"-%s\""), text));
            }
        } else {
            // Create plugin manager and installer separately for each item
            Manager mgr(translator(), log());
            Installer installer(mgr, fileSystem(), *dir);
            did = true;

            try {
                Plugin* plug = installer.prepareInstall(text);
                if (!plug) {
                    errorOutput().writeLine(Format(tx("%s: Unknown file type"), text));
                    err = true;
                } else {
                    installer.doInstall(false);
                    if (verbose) {
                        standardOutput().writeLine(Format(tx("%s: Plugin '%s' (%s) tested successfully."), text, plug->getName(), plug->getId()));
                    }
                }
            }
            catch (afl::except::FileProblemException& e) {
                if (text == e.getFileName()) {
                    errorOutput().writeLine(Format("%s: %s", text, e.what()));
                } else {
                    errorOutput().writeLine(Format("%s: %s: %s", text, e.getFileName(), e.what()));
                }
                err = true;
            }
            catch (std::exception& e) {
                errorOutput().writeLine(Format("%s: %s", text, e.what()));
                err = true;
            }
        }
    }
    if (!did) {
        errorExit(Format(tx("Missing name plugin or file to test. '%s -h' for help."), environment().getInvocationName()));
    }
    if (err) {
        exit(1);
    }
}

void
util::plugin::ConsoleApplication::doHelp(afl::sys::Environment::CommandLine_t& /*cmdl*/)
{
    afl::string::Translator& tx = translator();
    standardOutput().writeText(Format(tx("PCC2 Plugin Manager v%s - (c) 2015-2026 Stefan Reuther\n"), PCC2_VERSION));
    standardOutput().writeText(Format(tx("\n"
                                         "Usage:\n"
                                         "  %s -h|help\n"
                                         "                  This help message\n"
                                         "  %$0s list|ls [-l|-b] [-e|-d] [-o]\n"
                                         "                  List installed plugins\n"
                                         "  %$0s add|install [-n] [-f] FILE.c2p...\n"
                                         "                  Install given plugins\n"
                                         "  %$0s remove|rm|uninstall [-n] [-f] ID...\n"
                                         "                  Remove given plugins\n"
                                         "  %$0s enable|disable [-n] [-f] ID...\n"
                                         "                  Enable/disable given plugins\n"
                                         "  %$0s test [-v] FILE.c2p...\n"
                                         "                  Test given plugins\n"
                                         "\n"
                                         "Options:\n"
                                         "%s\n"
                                         "Report bugs to <Streu@gmx.de>\n"),
                                      environment().getInvocationName(),
                                      util::formatOptions(tx("-l\tList more details\n"
                                                             "-b\tList fewer details\n"
                                                             "-e\tShow only enabled plugins\n"
                                                             "-d\tShow only disabled plutins\n"
                                                             "-o\tList in load order (default: alphabetical)\n"
                                                             "-n\tDry run (don't modify anything, just check)\n"
                                                             "-f\tIgnore dependencies/requirements\n"
                                                             "-v\tVerbose\n"))));
    exit(0);
}

const util::plugin::ConsoleApplication::Command*
util::plugin::ConsoleApplication::findCommand(const String_t& name)
{
    static const struct Command commands[] = {
        { "ls",        &ConsoleApplication::doList },
        { "list",      &ConsoleApplication::doList },
        { "add",       &ConsoleApplication::doAdd },
        { "install",   &ConsoleApplication::doAdd },
        { "rm",        &ConsoleApplication::doRemove },
        { "remove",    &ConsoleApplication::doRemove },
        { "uninstall", &ConsoleApplication::doRemove },
        { "enable",    &ConsoleApplication::doEnable },
        { "disable",   &ConsoleApplication::doDisable },
        { "test",      &ConsoleApplication::doTest },
        { "-h",        &ConsoleApplication::doHelp },
        { "--help",    &ConsoleApplication::doHelp },
        { "-help",     &ConsoleApplication::doHelp },
        { "help",      &ConsoleApplication::doHelp },
    };

    afl::base::Memory<const Command> c(commands);
    while (const Command* p = c.eat()) {
        if (p->name == name) {
            return p;
        }
    }
    return 0;
}
