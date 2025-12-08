/**
  *  \file server/file/clientapplication.cpp
  *  \brief Class server::file::ClientApplication
  */

#include <algorithm>
#include "server/file/clientapplication.hpp"
#include "afl/net/http/pagedispatcher.hpp"
#include "afl/net/http/protocolhandler.hpp"
#include "afl/net/name.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "server/file/ca/garbagecollector.hpp"
#include "server/file/ca/root.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/directoryhandlerfactory.hpp"
#include "server/file/directorypage.hpp"
#include "server/file/filesystemhandler.hpp"
#include "server/file/utils.hpp"
#include "version.hpp"

// For debugging/memory optimisation
// #define MALLOC_STATS
#ifdef MALLOC_STATS
# include <malloc.h>
#endif

void
server::file::ClientApplication::appMain()
{
    // Parse args
    afl::string::Translator& tx = translator();
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    afl::base::Optional<String_t> arg_command;
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "h" || p == "help") {
                help();
            } else if (p == "proxy") {
                m_networkStack.add(commandLine.getRequiredParameter(p));
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            arg_command = p;
            break;
        }
    }

    // Now, the commandLine sits at the first argument for the command. Dispatch on command.
    const String_t* pCommand = arg_command.get();
    if (!pCommand) {
        errorExit(afl::string::Format(tx("no command specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }
    if (*pCommand == "help") {
        help();
    } else if (*pCommand == "ls") {
        doList(commandLine);
    } else if (*pCommand == "cp") {
        doCopy(commandLine);
    } else if (*pCommand == "clear") {
        doClear(commandLine);
    } else if (*pCommand == "sync") {
        doSync(commandLine);
    } else if (*pCommand == "serve") {
        doServe(commandLine);
    } else if (*pCommand == "gc") {
        doGC(commandLine);
    } else if (*pCommand == "snapshot") {
        doSnapshot(commandLine);
    } else {
        errorExit(afl::string::Format(tx("invalid command '%s'. Use '%s -h' for help.").c_str(), *pCommand, environment().getInvocationName()));
    }
}

void
server::file::ClientApplication::doCopy(afl::sys::CommandLineParser& cmdl)
{
    afl::string::Translator& tx = translator();
    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    ReadOnlyDirectoryHandler* in = 0;
    DirectoryHandler* out = 0;
    CopyFlags_t flags;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                flags += CopyRecursively;
            } else if (p == "x") {
                flags += CopyExpandTarballs;
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            if (in == 0) {
                in = &dhf.createDirectoryHandler(p, log());
            } else if (out == 0) {
                out = &dhf.createDirectoryHandler(p, log());
            } else {
                errorExit(afl::string::Format(tx("too many directory names specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        }
    }

    if (out == 0) {
        errorExit(afl::string::Format(tx("need two directory names (source, destination). Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    copyDirectory(*out, *in, flags);
}

void
server::file::ClientApplication::doSync(afl::sys::CommandLineParser& cmdl)
{
    afl::string::Translator& tx = translator();
    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    ReadOnlyDirectoryHandler* in = 0;
    DirectoryHandler* out = 0;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                // ignore for symmetry with 'cp'
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            if (in == 0) {
                in = &dhf.createDirectoryHandler(p, log());
            } else if (out == 0) {
                out = &dhf.createDirectoryHandler(p, log());
            } else {
                errorExit(afl::string::Format(tx("too many directory names specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        }
    }

    if (out == 0) {
        errorExit(afl::string::Format(tx("need two directory names (source, destination). Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    synchronizeDirectories(*out, *in);
}

void
server::file::ClientApplication::doList(afl::sys::CommandLineParser& cmdl)
{
    afl::string::Translator& tx = translator();
    bool opt_recursive = false;
    bool opt_long = false;
    std::vector<String_t> args;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                opt_recursive = true;
            } else if (p == "l") {
                opt_long = true;
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            args.push_back(p);
        }
    }

    if (args.empty()) {
        errorExit(afl::string::Format(tx("missing directory name to list. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    bool withHeader = (args.size() > 1 || opt_recursive);
    for (size_t i = 0, n = args.size(); i < n; ++i) {
        doList(dhf.createDirectoryHandler(args[i], log()), args[i], opt_recursive, opt_long, withHeader);
    }
}

void
server::file::ClientApplication::doList(ReadOnlyDirectoryHandler& in, String_t name, bool recursive, bool longFormat, bool withHeader)
{
    afl::io::TextWriter& out = standardOutput();
    if (withHeader) {
        out.writeLine(name + ":");
    }

    // Read content
    InfoVector_t children;
    listDirectory(children, in);

    // Display content
    for (size_t i = 0, n = children.size(); i < n; ++i) {
        const DirectoryHandler::Info& ch = children[i];
        if (longFormat) {
            String_t size = "-";
            if (const int32_t* p = ch.size.get()) {
                size = afl::string::Format("%d", *p);
            }
            String_t type = "?";
            switch (ch.type) {
             case DirectoryHandler::IsUnknown:   type = "UNK";  break;
             case DirectoryHandler::IsDirectory: type = "DIR";  break;
             case DirectoryHandler::IsFile:      type = "FILE"; break;
            }
            out.writeLine(afl::string::Format("%-4s %-40s %10s  %s", type, ch.contentId.orElse("-"), size, ch.name));
        } else {
            out.writeLine(ch.name);
        }
    }
    if (withHeader) {
        out.writeLine();
    }

    if (recursive) {
        for (size_t i = 0, n = children.size(); i < n; ++i) {
            const DirectoryHandler::Info& ch = children[i];
            if (ch.type == DirectoryHandler::IsDirectory) {
                std::auto_ptr<ReadOnlyDirectoryHandler> sub(in.getDirectory(ch));
                doList(*sub, DirectoryHandlerFactory::makePathName(name, ch.name), recursive, longFormat, withHeader);
            }
        }
    }
}

void
server::file::ClientApplication::doClear(afl::sys::CommandLineParser& cmdl)
{
    afl::string::Translator& tx = translator();
    std::vector<String_t> args;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                // Ignore for consistency; we are always recursive
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            args.push_back(p);
        }
    }

    if (args.empty()) {
        errorExit(afl::string::Format(tx("missing directory name to clear. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    for (size_t i = 0, n = args.size(); i < n; ++i) {
        removeDirectoryContent(dhf.createDirectoryHandler(args[i], log()));
    }
}

void
server::file::ClientApplication::doServe(afl::sys::CommandLineParser& cmdl)
{
    // Parse parameters
    afl::string::Translator& tx = translator();
    afl::base::Optional<String_t> source;
    afl::base::Optional<String_t> address;

    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
        } else if (!source.isValid()) {
            source = p;
        } else if (!address.isValid()) {
            address = p;
        } else {
            errorExit(afl::string::Format(tx("too many parameters. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
        }
    }

    const String_t* pSource = source.get();
    const String_t* pAddress = address.get();
    if (pSource == 0 || pAddress == 0) {
        errorExit(afl::string::Format(tx("too few parameters. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    // ProtocolHandlerFactory
    class MyProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        MyProtocolHandlerFactory(afl::net::http::Dispatcher& disp)
            : m_dispatcher(disp)
            { }
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::http::ProtocolHandler(m_dispatcher); }
     private:
        afl::net::http::Dispatcher& m_dispatcher;
    };

    // Set up
    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    DirectoryHandler& dh(dhf.createDirectoryHandler(*pSource, log()));
    afl::net::http::PageDispatcher disp;
    disp.addNewPage("", new DirectoryPage(dh));
    MyProtocolHandlerFactory phf(disp);
    afl::net::Server server(m_serverNetworkStack.listen(afl::net::Name::parse(*pAddress, "8080"), 10), phf);
    server.run();
}

void
server::file::ClientApplication::doGC(afl::sys::CommandLineParser& cmdl)
{
    // Parse parameters
    afl::string::Translator& tx = translator();
    afl::base::Optional<String_t> dir;

    String_t p;
    bool opt;
    bool dryRun = false;
    bool force = false;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "n") {
                dryRun = true;
            } else if (p == "f") {
                force = true;
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else if (!dir.isValid()) {
            dir = p;
        } else {
            errorExit(afl::string::Format(tx("too many parameters. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
        }
    }

    const String_t* pDir = dir.get();
    if (pDir == 0) {
        errorExit(afl::string::Format(tx("too few parameters. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    // Objects
    // (Intentionally do not use DirectoryHandlerFactory; we don't want to use 'ca:DIR' here.)
    FileSystemHandler handler(fileSystem(), *pDir);
    server::file::ca::Root root(handler);
    server::file::ca::GarbageCollector gc(root.objectStore(), log());

    // Do it!
#ifdef MALLOC_STATS
    int prevMem = mallinfo().arena;
#endif
    std::vector<server::file::ca::ObjectId> roots;
    root.listRoots(roots);
    for (size_t i = 0; i < roots.size(); ++i) {
        gc.addCommit(roots[i]);
    }

    size_t n = 0;
    while (gc.checkObject()) {
        if (++n % 512 == 0) {
            standardOutput().writeLine(afl::string::Format("... to check: %d, reachable: %d", gc.getNumObjectsToCheck(), gc.getNumObjectsToKeep()));
            standardOutput().flush();
        }
    }
    standardOutput().writeLine(afl::string::Format("Total reachable objects: %d", gc.getNumObjectsToKeep()));
    standardOutput().flush();
#ifdef MALLOC_STATS
    int postMem = mallinfo().arena;
    standardOutput().writeLine(afl::string::Format("malloc: %d to %d = %d [%dk]", prevMem, postMem, (postMem-prevMem), (postMem-prevMem) / 1024));
    standardOutput().flush();
#endif
    if (gc.getNumErrors() != 0 && !force) {
        errorOutput().writeLine(afl::string::Format("%d error%!1{s%} found, aborted (use \"-f\" to force)", gc.getNumErrors()));
        exit(1);
    }

    // Remove
    if (!dryRun) {
        while (gc.removeGarbageObjects())
            ;
        standardOutput().writeLine(afl::string::Format("Total objects removed: %d", gc.getNumObjectsRemoved()));
    }
}

void
server::file::ClientApplication::doSnapshot(afl::sys::CommandLineParser& cmdl)
{
    // Parse parameters
    afl::string::Translator& tx = translator();
    std::vector<String_t> args;

    String_t p;
    bool opt;
    bool longFormat = false;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "l") {
                longFormat = true;
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help."), environment().getInvocationName()));
            }
        } else {
            args.push_back(p);
        }
    }

    if (args.size() < 2) {
        errorExit(afl::string::Format(tx("too few parameters. Use '%s -h' for help."), environment().getInvocationName()));
    }

    // Objects
    // (Intentionally do not use DirectoryHandlerFactory; we don't want to use 'ca:DIR' here.)
    FileSystemHandler handler(fileSystem(), args[0]);
    server::file::ca::Root root(handler);

    if (args[1] == "ls") {
        afl::data::StringList_t list;
        root.listSnapshots(list);
        std::sort(list.begin(), list.end());
        for (size_t i = 0; i < list.size(); ++i) {
            if (longFormat) {
                afl::base::Optional<server::file::ca::ObjectId> objId = root.getSnapshotCommitId(list[i]);
                String_t objName = objId.isValid() ? objId.get()->toHex() : "-";
                standardOutput().writeLine(afl::string::Format("%-4s %-40s %10s  %s", "SNAP", objName, "-", list[i]));
            } else {
                standardOutput().writeLine(list[i]);
            }
        }
    } else if (args[1] == "add" || args[1] == "create") {
        if (args.size() < 3) {
            errorExit(afl::string::Format(tx("too few parameters. Use '%s -h' for help."), environment().getInvocationName()));
        }
        for (size_t i = 2; i < args.size(); ++i) {
            root.setSnapshotCommitId(args[i], root.getMasterCommitId());
        }
    } else if (args[1] == "rm" || args[1] == "delete") {
        if (args.size() < 3) {
            errorExit(afl::string::Format(tx("too few parameters. Use '%s -h' for help."), environment().getInvocationName()));
        }
        for (size_t i = 2; i < args.size(); ++i) {
            root.removeSnapshot(args[i]);
        }
    } else if (args[1] == "cp" || args[1] == "copy") {
        if (args.size() < 4) {
            errorExit(afl::string::Format(tx("too few parameters. Use '%s -h' for help."), environment().getInvocationName()));
        }
        server::file::ca::ObjectId objId = resolveObjectId(root, args[2]);
        for (size_t i = 3; i < args.size(); ++i) {
            root.setSnapshotCommitId(args[i], objId);
        }
    } else if (args[1] == "restore") {
        if (args.size() < 3) {
            errorExit(afl::string::Format(tx("too few parameters. Use '%s -h' for help."), environment().getInvocationName()));
        }
        if (args.size() > 3) {
            errorExit(afl::string::Format(tx("too many parameters. Use '%s -h' for help."), environment().getInvocationName()));
        }
        root.setMasterCommitId(resolveObjectId(root, args[2]));
    } else {
        errorExit(afl::string::Format(tx("invalid command '%s'. Use '%s -h' for help."), args[1], environment().getInvocationName()));
    }
}

void
server::file::ClientApplication::help()
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(afl::string::Format(tx("PCC2 File Client v%s - (c) 2017-2025 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(afl::string::Format(tx("Usage:\n"
                                         "  %s [-h]\n"
                                         "  %$0s [--proxy=URL] COMMAND...\n"
                                         "\n"
                                         "Commands:\n"
                                         "  %$0s cp [-r] [-x] SOURCE DEST\n"
                                         "                      Copy everything from SOURCE to DEST\n"
                                         "  %$0s ls [-r] [-l] DIR...\n"
                                         "                      List content of the DIRs\n"
                                         "  %$0s sync SOURCE DEST\n"
                                         "                      Make DEST contain the same content as SOURCE\n"
                                         "  %$0s clear DIR...\n"
                                         "                      Remove content of DIRs\n"
                                         "  %$0s serve SOURCE HOST:PORT\n"
                                         "                      Serve SOURCE via HTTP for testing\n"
                                         "  %$0s gc [-n] [-f] PATH\n"
                                         "                      Garbage-collect a CA file system\n"
                                         "  %$0s snapshot PATH ls [-l]\n"
                                         "                      List snapshots (tags) on CA file system\n"
                                         "  %$0s snapshot PATH add NAME...\n"
                                         "                      Create snapshots (tags) on CA file system\n"
                                         "  %$0s snapshot PATH rm NAME...\n"
                                         "                      Remove snapshots (tags) on CA file system\n"
                                         "  %$0s snapshot PATH cp OLD NEW...\n"
                                         "                      Copy snapshots (tags) on CA file system\n"
                                         "  %$0s snapshot PATH restore NAME\n"
                                         "                      Restore from snapshot (tag) on CA file system\n"
                                         "\n"
                                         "Command Options:\n"
                                         "  -f                  Force garbage-collection even on error\n"
                                         "  -l                  Long format\n"
                                         "  -n                  Dry run (do not delete anything)\n"
                                         "  -r                  Recursive\n"
                                         "  -x                  Expand *.tgz/*.tar.gz files\n"
                                         "\n"
                                         "File specifications:\n"
                                         "  PATH                Access files within unmanaged file system\n"
                                         "  [PATH@]ca:SPEC      Access files within unmanaged content-addressable file system\n"
                                         "  [PATH@]snapshot:NAME:SPEC\n"
                                         "                      Access files from a CA file system snapshot (read-only)\n"
                                         "  [PATH@]int:[UNIQ]   Internal (RAM, not persistent) file space\n"
                                         "  c2file://[USER@]HOST:PORT/PATH\n"
                                         "                      Access in a remote managed file system (c2file server)\n"
                                         "\n"
                                         "Report bugs to <Streu@gmx.de>\n").c_str(),
                                      environment().getInvocationName()));
    out.flush();
    exit(0);
}

server::file::ca::ObjectId
server::file::ClientApplication::resolveObjectId(server::file::ca::Root& root, const String_t& name)
{
    afl::base::Optional<server::file::ca::ObjectId> objId = root.getSnapshotCommitId(name);
    server::file::ca::ObjectId* p = objId.get();
    if (p == 0) {
        errorExit(afl::string::Format(translator()("unable to resolve snapshot Id '%s'"), name));
    }
    return *p;
}
