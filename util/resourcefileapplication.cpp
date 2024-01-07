/**
  *  \file util/resourcefileapplication.cpp
  *  \brief Class util::ResourceFileApplication
  *
  *  This class bundles original PCC1 applications:
  *  - rc2.pas
  *  - rx.pas
  *  - rxall.pas
  *  - parts of replace.pas, replaceb.pas
  *
  *  Known differences between rc2 and 'c2restool create':
  *  - different command line. Most notable difference: rc2 defaults to CRLF linefeeds,
  *    '/k' to use LF; we default to LF, '--crlf' to use CRLF;
  *  - in rc2, in a '.text' block, you can do "\foo\" to insert the number of the
  *    member named 'foo' (and "\\" to insert a single backslash).
  *    This is not supported;
  *  - our '.nul' actually creates an empty element;
  *  - we can process multiple files in one go;
  *  - we can produce a dependency file for Make.
  */

#include "util/resourcefileapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/io/bufferedstream.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "util/resourcefilereader.hpp"
#include "util/resourcefilewriter.hpp"
#include "util/stringparser.hpp"
#include "version.hpp"

using afl::base::Optional;
using afl::base::Ptr;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::io::TextFile;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::CommandLineParser;

namespace {
    /*
     *  Generic parameter parsing
     */

    struct Parameters {
        std::vector<String_t> fileNames;
        std::vector<String_t> searchPath;
        Optional<String_t> listFileName;
        Optional<String_t> depFileName;
        String_t listFilePattern;
        bool useCRLF;

        Parameters()
            : fileNames(), searchPath(), listFileName(), depFileName(),
              listFilePattern("%s=%d;"), useCRLF(false)
            { }
    };

    void parseParameters(Parameters& out,
                         afl::sys::CommandLineParser& cmdl,
                         util::ResourceFileApplication& app,
                         bool acceptOptions)
    {
        bool option;
        String_t text;
        while (cmdl.getNext(option, text)) {
            if (option) {
                if (text == "h" || text == "help") {
                    app.help();
                } else if (acceptOptions && text == "crlf") {
                    out.useCRLF = true;
                } else if (acceptOptions && text == "list") {
                    out.listFileName = cmdl.getRequiredParameter(text);
                } else if (acceptOptions && text == "dep") {
                    out.depFileName = cmdl.getRequiredParameter(text);
                } else if (acceptOptions && text == "list-format") {
                    out.listFilePattern = cmdl.getRequiredParameter(text);
                } else if (acceptOptions && text == "L") {
                    out.searchPath.push_back(cmdl.getRequiredParameter(text));
                } else {
                    app.errorExit(Format(app.translator()("invalid option '%s' specified. Use '%s -h' for help."), text, app.environment().getInvocationName()));
                }
            } else {
                out.fileNames.push_back(text);
            }
        }
    }


    /*
     *  Status for "create" command
     */

    struct CreateStatus {
        util::ResourceFileApplication& app;
        Parameters& param;
        util::ResourceFileWriter writer;
        std::vector<String_t> listFileContent;
        std::vector<String_t> depFileContent;

        CreateStatus(util::ResourceFileApplication& app, Parameters& param, Ref<Stream> file)
            : app(app),
              param(param),
              writer(file, app.translator())
            { }
    };

    /* Check for valid identifier in a member name. */
    bool charIsIdentifier(char ch)
    {
        return (ch >= 'A' && ch <= 'Z')
            || (ch >= 'a' && ch <= 'z')
            || (ch >= '0' && ch <= '9')
            || (ch == '_')
            || (ch == '$');
    }

    /* Open input file. Search path if required, and register it in depFileContent. */
    Ref<Stream> openInputFile(CreateStatus& st, String_t fileName)
    {
        FileSystem& fs = st.app.fileSystem();
        bool search = true;
        if (fileName.size() > 2 && fileName[0] == '*' && fs.isPathSeparator(fileName[1])) {
            fileName.erase(0, 2);
        } else {
            for (size_t i = 0; i < fileName.size(); ++i) {
                if (fs.isPathSeparator(fileName[i])) {
                    search = false;
                }
            }
        }

        // Path search
        if (search) {
            for (size_t i = 0, n = st.param.searchPath.size(); i < n; ++i) {
                String_t pathName = fs.makePathName(st.param.searchPath[i], fileName);
                try {
                    Ref<Stream> result = fs.openFile(pathName, FileSystem::OpenRead);
                    st.depFileContent.push_back(pathName);
                    return result;
                }
                catch (...) { }
            }
        }

        // Fall back
        Ref<Stream> result = fs.openFile(fileName, FileSystem::OpenRead);
        st.depFileContent.push_back(fileName);
        return result;
    }

    /* Process a script file. */
    void processFile(CreateStatus& st, const String_t& fileName)
    {
        Translator& tx = st.app.translator();
        Ref<Stream> file(st.app.fileSystem().openFile(fileName, FileSystem::OpenRead));
        TextFile script(*file);

        st.depFileContent.push_back(fileName);

        String_t line;
        Optional<uint16_t> lastId;
        while (script.readLine(line)) {
            // Parser
            String_t tmpStr;
            int32_t tmpInt;
            util::StringParser parser(line);

            // Skip whitespace; check comments
            parser.parseWhile(afl::string::charIsSpace, tmpStr);
            if (parser.parseEnd() || parser.parseCharacter(';')) {
                continue;
            }

            // Determine next ID
            uint16_t nextId = 0;
            if (parser.parseCaseInsensitiveString("next")) {
                if (const uint16_t* p = lastId.get()) {
                    nextId = static_cast<uint16_t>((*p)+1);
                } else {
                    st.app.errorExit(Format(tx("%s:%d: cannot use 'next' on first entry"), fileName, script.getLineNumber()));
                }
            } else if (parser.parseInt(tmpInt) && tmpInt >= 0 && tmpInt <= 0xFFFF) {
                nextId = static_cast<uint16_t>(tmpInt);
            } else {
                st.app.errorExit(Format(tx("%s:%d: invalid ID number"), fileName, script.getLineNumber()));
            }
            lastId = nextId;

            // Alias?
            parser.parseWhile(afl::string::charIsSpace, tmpStr);
            if (parser.parseCharacter('=')) {
                parser.parseWhile(afl::string::charIsSpace, tmpStr);
                parser.parseWhile(charIsIdentifier, tmpStr);
                st.listFileContent.push_back(Format(st.param.listFilePattern, tmpStr, nextId));
                parser.parseWhile(afl::string::charIsSpace, tmpStr);
            }

            // Save possible file name
            const String_t memberName = parser.getRemainder();
            if (parser.parseCaseInsensitiveString("eq")
                && parser.parseWhile(afl::string::charIsSpace, tmpStr)
                && parser.parseInt(tmpInt)
                && (tmpInt >= 0 && tmpInt <= 0xFFFF)
                && (parser.parseWhile(afl::string::charIsSpace, tmpStr),
                    parser.parseEnd()))
            {
                // Hardlink
                if (!st.writer.createHardlink(static_cast<uint16_t>(tmpInt), nextId)) {
                    st.app.errorExit(Format(tx("%s:%d: source ID not defined yet"), fileName, script.getLineNumber()));
                }
            } else {
                // Normal member
                Ref<Stream> member(st.writer.createMember(nextId));
                if (afl::string::strCaseCompare(memberName, ".nul") == 0) {
                    // Just write nothing
                } else if (afl::string::strCaseCompare(memberName, ".text") == 0) {
                    // Embedded text
                    afl::io::BufferedStream bufOut(*member);
                    String_t newline = st.param.useCRLF ? "\r\n" : "\n";
                    while (1) {
                        String_t content;
                        if (!script.readLine(content)) {
                            st.app.errorExit(Format(tx("%s:%d: unexpected EOF in .text section"), fileName, script.getLineNumber()));
                        }
                        if (afl::string::strCaseCompare(content, ".endtext") == 0) {
                            break;
                        }
                        bufOut.write(afl::string::toBytes(content));
                        bufOut.write(afl::string::toBytes(newline));
                    }
                    bufOut.flush();
                } else {
                    // Possible file
                    member->copyFrom(*openInputFile(st, memberName));
                }
            }
        }
    }

    /* Write list file. */
    void writeListFile(const CreateStatus& st, const String_t& fileName)
    {
        Ref<Stream> out(st.app.fileSystem().openFile(fileName, FileSystem::Create));
        TextFile tf(*out);
        for (size_t i = 0, n = st.listFileContent.size(); i < n; ++i) {
            tf.writeLine(st.listFileContent[i]);
        }
        tf.flush();
    }

    /* Write dependency file. */
    void writeDependencyFile(const CreateStatus& st, const String_t& fileName, const String_t& resFileName)
    {
        Ref<Stream> out(st.app.fileSystem().openFile(fileName, FileSystem::Create));
        TextFile tf(*out);

        // Normal dependencies block
        tf.writeText(resFileName + ":");
        for (size_t i = 0, n = st.depFileContent.size(); i < n; ++i) {
            tf.writeLine(" \\");
            tf.writeText("\t" + st.depFileContent[i]);
        }
        tf.writeLine();

        // Pseudo rules to support deletion of inputs
        for (size_t i = 0, n = st.depFileContent.size(); i < n; ++i) {
            tf.writeLine(st.depFileContent[i] + ":");
        }
        tf.flush();
    }
}


/*
 *  ResourceFileApplication
 */

util::ResourceFileApplication::ResourceFileApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : Application(env, fs)
{ }

void
util::ResourceFileApplication::appMain()
{
    // Parse args
    Translator& tx = translator();
    afl::sys::StandardCommandLineParser cmdl(environment().getCommandLine());
    afl::base::Optional<String_t> arg_command;
    String_t text;
    bool option;

    while (cmdl.getNext(option, text)) {
        if (option) {
            if (text == "h" || text == "help") {
                help();
            } else {
                errorExit(Format(tx("invalid option '%s' specified. Use '%s -h' for help."), text, environment().getInvocationName()));
            }
        } else {
            arg_command = text;
            break;
        }
    }

    // Dispatch on command
    const String_t* pCommand = arg_command.get();
    if (!pCommand) {
        errorExit(Format(tx("no command specified. Use '%s -h' for help."), environment().getInvocationName()));
    }

    if (text == "help") {
        help();
    } else if (text == "create") {
        doCreate(cmdl);
    } else if (text == "list" || text == "ls") {
        doList(cmdl);
    } else if (text == "extract" || text == "rx") {
        doExtract(cmdl);
    } else if (text == "extract-all" || text == "rxall") {
        doExtractAll(cmdl);
    } else {
        errorExit(Format(tx("invalid command '%s'. Use '%s -h' for help."), *pCommand, environment().getInvocationName()));
    }
}

void
util::ResourceFileApplication::help()
{
    Translator& tx = translator();
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(Format(tx("PCC2 Resource File Utility v%s - (c) 2023-2024 Stefan Reuther"), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s COMMAND...\n"
                            "\n"
                            "Commands:\n"
                            "  %$0s create [--crlf] [--list=FILE] [-L DIR] FILE.RES FILE.RC...\n"
                            "                      Create resource file from scripts\n"
                            "  %$0s ls FILE.RES...\n"
                            "                      List content of the FILEs (also: list)\n"
                            "  %$0s extract FILE.RES INDEX FILE.OUT\n"
                            "                      Extract single entry (also: rx)\n"
                            "  %$0s extract-all FILE.RES [FILE.RC]\n"
                            "                      Extract all files and create a script (also: rxall)\n"
                            "\n"
                            "Command Options:\n"
                            "  --crlf              (create) Use CR/LF linefeeds for embedded text\n"
                            "  --list=FILE         (create) Create list file of aliases\n"
                            "  --dep=FILE          (create) Create dependency file\n"
                            "  --list-format=FMT   (create) Define format of list file (e.g. \"#define %%s %%d\")\n"
                            "  -L DIR              (create) Search path\n"
                            "\n"
                            "Resource scripts:\n"
                            "  NUM[=ALIAS] SOURCE  Create an entry\n"
                            "    NUM can be number or '.next'\n"
                            "    SOURCE can be file name or '.text' or 'eq NUM'\n"
                            "\n"
                            "Report bugs to <Streu@gmx.de>\n"),
                         environment().getInvocationName()));
    out.flush();
    exit(0);
}

/* "create" command.
   Formerly "rc2 FILE.RC...." (rc2.pas), now with different syntax */
void
util::ResourceFileApplication::doCreate(afl::sys::CommandLineParser& cmdl)
{
    Parameters param;
    parseParameters(param, cmdl, *this, true);

    // Command line validation
    Translator& tx = translator();
    if (param.fileNames.size() < 2) {
        errorExit(tx("command requires at least 2 parameters"));
    }

    // Output file
    const String_t& outFileName = param.fileNames[0];
    CreateStatus status(*this, param, fileSystem().openFile(outFileName, FileSystem::Create));

    // Process input files
    for (size_t i = 1, n = param.fileNames.size(); i < n; ++i) {
        processFile(status, param.fileNames[i]);
    }

    // Finish
    status.writer.finishFile();

    // Write list file
    if (const String_t* p = param.listFileName.get()) {
        writeListFile(status, *p);
    }

    // Write dependency file
    if (const String_t* p = param.depFileName.get()) {
        writeDependencyFile(status, *p, outFileName);
    }
}

/* "list" command.
   Formerly "rx FILE.RES" (rx.pas) */
void
util::ResourceFileApplication::doList(afl::sys::CommandLineParser& cmdl)
{
    Parameters param;
    parseParameters(param, cmdl, *this, false);

    for (size_t i = 0; i < param.fileNames.size(); ++i) {
        Ref<Stream> file = fileSystem().openFile(param.fileNames[i], FileSystem::OpenRead);
        ResourceFileReader rdr(file, translator());
        for (size_t i = 0, n = rdr.getNumMembers(); i < n; ++i) {
            standardOutput().writeLine(Format("%5d %9d", rdr.getMemberIdByIndex(i), rdr.openMemberByIndex(i)->getSize()));
        }
    }
}

/* "extract" command.
   Formerly "rx FILE.RES ID FILE.OUT" (rx.pas) */
void
util::ResourceFileApplication::doExtract(afl::sys::CommandLineParser& cmdl)
{
    Parameters param;
    parseParameters(param, cmdl, *this, false);

    // Command line validation
    Translator& tx = translator();
    if (param.fileNames.size() != 3) {
        errorExit(tx("command requires 3 parameters"));
    }

    // Parse ID
    uint16_t id = 0;
    if (!afl::string::strToInteger(param.fileNames[1], id)) {
        errorExit(tx("resource Id must be a number"));
    }

    // Open input
    Ref<Stream> file = fileSystem().openFile(param.fileNames[0], FileSystem::OpenRead);
    ResourceFileReader rdr(file, translator());
    Ptr<Stream> member = rdr.openMember(id);
    if (member.get() == 0) {
        errorExit(tx("resource Id not found in file"));
    }

    // Create output
    fileSystem().openFile(param.fileNames[2], FileSystem::Create)
        ->copyFrom(*member);
}

/* "extract-all" command.
   Formerly "rxall FILE.RES" (rxall.pas) */
void
util::ResourceFileApplication::doExtractAll(afl::sys::CommandLineParser& cmdl)
{
    Parameters param;
    parseParameters(param, cmdl, *this, false);

    // Command line validation
    Translator& tx = translator();
    if (param.fileNames.size() < 1 || param.fileNames.size() > 2) {
        errorExit(tx("command requires 1 or 2 parameters"));
    }

    // Open input
    Ref<Stream> file = fileSystem().openFile(param.fileNames[0], FileSystem::OpenRead);
    ResourceFileReader rdr(file, translator());

    // Open list file
    Ref<Stream> listFile =
        (param.fileNames.size() < 2
         ? *new afl::io::NullStream()
         : fileSystem().openFile(param.fileNames[1], FileSystem::Create));
    TextFile list(*listFile);

    // Do it
    for (size_t i = 0, n = rdr.getNumMembers(); i < n; ++i) {
        uint16_t thisId = rdr.getMemberIdByIndex(i);
        uint16_t primaryId = rdr.findPrimaryIdByIndex(i);
        if (thisId != primaryId) {
            list.writeLine(Format("%d eq %d", thisId, primaryId));
        } else {
            Ptr<Stream> member = rdr.openMemberByIndex(i);
            if (member.get() == 0) {
                errorExit("<Internal error: open failed>");     // Must not happen
            }

            String_t fileName = Format("%05d.dat", thisId);
            fileSystem().openFile(fileName, FileSystem::Create)
                ->copyFrom(*member);

            list.writeLine(Format("%d %s", thisId, fileName));
        }
    }

    // Finish
    list.flush();
}
