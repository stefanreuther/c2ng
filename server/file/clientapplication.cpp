/**
  *  \file server/file/clientapplication.cpp
  *  \brief Class server::file::ClientApplication
  */

#include "server/file/clientapplication.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"
#include "server/file/directoryhandlerfactory.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/utils.hpp"
#include "version.hpp"

void
server::file::ClientApplication::appMain()
{
    // Parse args
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    afl::base::Optional<String_t> arg_command;
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "h" || p == "help") {
                help();
            } else {
                errorExit(afl::string::Format(_("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            arg_command = p;
            break;
        }
    }

    // Now, the commandLine sits at the first argument for the command. Dispatch on command.
    const String_t* pCommand = arg_command.get();
    if (!pCommand) {
        errorExit(afl::string::Format(_("no command specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
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
    } else {
        errorExit(afl::string::Format(_("invalid command '%s'. Use '%s -h' for help.").c_str(), *pCommand, environment().getInvocationName()));
    }
}

void
server::file::ClientApplication::doCopy(afl::sys::CommandLineParser& cmdl)
{
    bool opt_recursive = false;
    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    DirectoryHandler* in = 0;
    DirectoryHandler* out = 0;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                opt_recursive = true;
            } else {
                errorExit(afl::string::Format(_("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            if (in == 0) {
                in = &dhf.createDirectoryHandler(p);
            } else if (out == 0) {
                out = &dhf.createDirectoryHandler(p);
            } else {
                errorExit(afl::string::Format(_("too many directory names specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }                
        }
    }

    if (out == 0) {
        errorExit(afl::string::Format(_("need two directory names (source, destination). Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    copyDirectory(*out, *in, opt_recursive);
}

void
server::file::ClientApplication::doSync(afl::sys::CommandLineParser& cmdl)
{
    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    DirectoryHandler* in = 0;
    DirectoryHandler* out = 0;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                // ignore for symmetry with 'cp'
            } else {
                errorExit(afl::string::Format(_("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            if (in == 0) {
                in = &dhf.createDirectoryHandler(p);
            } else if (out == 0) {
                out = &dhf.createDirectoryHandler(p);
            } else {
                errorExit(afl::string::Format(_("too many directory names specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }                
        }
    }

    if (out == 0) {
        errorExit(afl::string::Format(_("need two directory names (source, destination). Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    synchronizeDirectories(*out, *in);
}

void
server::file::ClientApplication::doList(afl::sys::CommandLineParser& cmdl)
{
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
                errorExit(afl::string::Format(_("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            args.push_back(p);
        }
    }

    if (args.empty()) {
        errorExit(afl::string::Format(_("missing directory name to list. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    bool withHeader = (args.size() > 1 || opt_recursive);
    for (size_t i = 0, n = args.size(); i < n; ++i) {
        doList(dhf.createDirectoryHandler(args[i]), args[i], opt_recursive, opt_long, withHeader);
    }
}

void
server::file::ClientApplication::doList(DirectoryHandler& in, String_t name, bool recursive, bool longFormat, bool withHeader)
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
                std::auto_ptr<DirectoryHandler> sub(in.getDirectory(ch));
                doList(*sub, DirectoryHandlerFactory::makePathName(name, ch.name), recursive, longFormat, withHeader);
            }
        }
    }
}

void
server::file::ClientApplication::doClear(afl::sys::CommandLineParser& cmdl)
{
    std::vector<String_t> args;
    String_t p;
    bool opt;
    while (cmdl.getNext(opt, p)) {
        if (opt) {
            if (p == "r") {
                // Ignore for consistency; we are always recursive
            } else {
                errorExit(afl::string::Format(_("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            args.push_back(p);
        }
    }

    if (args.empty()) {
        errorExit(afl::string::Format(_("missing directory name to clear. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
    }

    DirectoryHandlerFactory dhf(fileSystem(), networkStack());
    for (size_t i = 0, n = args.size(); i < n; ++i) {
        removeDirectoryContent(dhf.createDirectoryHandler(args[i]));
    }
}

void
server::file::ClientApplication::help()
{
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(afl::string::Format(_("PCC2 File Client v%s - (c) 2017-2018 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(afl::string::Format(_("Usage:\n"
                                        "  %s [-h]\n"
                                        "  %$0s cp [-r] SOURCE DEST\n"
                                        "  %$0s ls [-r] [-l] DIR\n"
                                        "  %$0s sync SOURCE DEST\n"
                                        "  %$0s clear DIR\n"
                                        "\n"
                                        "Options:\n"
                                        "  -r                  Recursive\n"
                                        "  -l                  Long format\n"
                                        "\n"
                                        "File specifications:\n"
                                        "  PATH                Access files within unmanaged file system\n"
                                        "  [PATH@]ca:SPEC      Access files within unmanaged content-addressable file system\n"
                                        // "  [PATH@]c2file:SPEC  Access files within managed file system\n"
                                        // "  [PATH@]ro:SPEC      Prevent write access\n"
                                        "  [PATH@]int:[UNIQ]   Internal (RAM, not persistent) file space\n"
                                        "  c2file://[USER@]HOST:PORT/PATH\n"
                                        "                      Access in a remote managed file system (c2file server)\n"
                                        "\n"
                                        "Report bugs to <Streu@gmx.de>\n").c_str(),
                                      environment().getInvocationName()));
    out.flush();
    exit(0);
}
