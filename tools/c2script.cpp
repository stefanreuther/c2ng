/**
  *  \file tools/c2script.cpp
  *  \brief c2script Utility - Script-Related Actions
  *
  *  PCC1 "ccs":
  *     /k CMD   execute command in game context
  *     /kk CMD  execute command before load
  *     /n       don't auto-load game
  *     /s       assume shareware key
  *     /p PWD   password
  *     /!       ignore default options
  *     /a       ignore plugins
  *
  *     /u       action: "unpack"
  *     /uc      action: "unpack + decompile turn"
  *     /r       action: "record statistics"
  *     /c       action: "maketurn"
  *
  *     /b       system: black/white
  *     /h       system: use UMBs
  *     /m       system: don't use XMS
  *     /t       system: skip splashscreen
  *     /w       system: Windows compatibility
  *     /x       system: 256 colors
  *     /d       system: no double-buffering
  *     /v       system: 16 colors
  *     /z       system: something something something mouse
  *     /$       system: swapfile size
  *
  *  c2script:
  *
  *     c2script [--action] [-flags] file.q....
  *
  *     Actions:
  *       --exec (default)   Execute
  *       --compile, -c      Produce "*.qc" files            [C compiler option]
  *       --disassemble, -S  Produce "*.qs" files            [C compiler option]
  *       --dump             Dump
  *
  *     Flags:
  *       -g                 Enable debug info               [C compiler option]
  *       -s                 Disable debug info              [C compiler option]
  *       -o file.qc/qs/dmp  Output file                     [C compiler option]
  *       --game/-G DIR      Game directory
  *       --root/-R DIR      Root direcory
  *       --charset/-C CS    Game charset                    [PCC2 option]
  *       -O LVL             Optimisation level              [C compiler option]
  *       -k                 Execute commands, not files     [PCC1 option]
  */

#include <stdexcept>
#include <vector>
#include "afl/base/optional.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/io/textfile.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/game.hpp"
#include "game/interface/loadcontext.hpp"
#include "game/session.hpp"
#include "game/specificationloader.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "game/v3/rootloader.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/vmio/assemblersavecontext.hpp"
#include "interpreter/vmio/filesavecontext.hpp"
#include "interpreter/vmio/objectloader.hpp"
#include "util/application.hpp"
#include "util/charsetfactory.hpp"
#include "util/consolelogger.hpp"
#include "util/profiledirectory.hpp"
#include "util/string.hpp"
#include "util/translation.hpp"
#include "version.hpp"

namespace {
    const char LOG_NAME[] = "script";

    enum Mode {
        ExecMode,          //   --exec (default)   Execute
        CompileMode,       //   --compile, -c      Produce "*.qc" files
        DisassembleMode,   //   --disassemble, -S  Produce "*.qs" files
        DumpMode           //   --dump             Dump
    };

    struct Parameters {
        afl::base::Optional<String_t> arg_gamedir;  // -G
        afl::base::Optional<String_t> arg_rootdir;  // -R
        afl::base::Optional<String_t> arg_output;   // -o
        bool opt_debug;                             // -g/-s
        bool opt_commands;                          // -k
        bool opt_preexecLoad;                       // -fpreexec-load
        bool opt_readonly;                          // --readonly
        Mode mode;
        std::auto_ptr<afl::charset::Charset> gameCharset;
        std::vector<String_t> loadPath;             // -I
        std::vector<String_t> job;
        int optimisationLevel;
        int playerNumber;

        Parameters()
            : arg_gamedir(),
              arg_rootdir(),
              arg_output(),
              opt_debug(true),
              opt_commands(false),
              opt_preexecLoad(false),
              opt_readonly(false),
              mode(ExecMode),
              gameCharset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1)),
              loadPath(),
              job(),
              optimisationLevel(1),
              playerNumber(0)
            { }
    };

    void warnIgnoredOptions(afl::sys::LogListener& log, const Parameters& params)
    {
        if (params.arg_gamedir.isValid()) {
            log.write(log.Warn, LOG_NAME, _("Game directory ('-G') name has been ignored"));
        }
        if (params.arg_rootdir.isValid()) {
            log.write(log.Warn, LOG_NAME, _("Root directory ('-R') name has been ignored"));
        }
        if (params.playerNumber > 0) {
            log.write(log.Warn, LOG_NAME, _("Player number ('-P') has been ignored"));
        }
    }

    String_t getFileNameExtension(afl::io::FileSystem& fs, String_t input)
    {
        String_t fileName = fs.getFileName(input);
        String_t::size_type dot = fileName.rfind('.');
        if (dot != String_t::npos && dot != 0) {
            return fileName.substr(dot+1);
        } else {
            return String_t();
        }
    }

    void doCompile(game::Session& session, const Parameters& params, std::vector<interpreter::BCOPtr_t>& result)
    {
        // Commands or files?
        if (params.opt_commands) {
            // Commands: compile everything into one single BCO
            interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
            interpreter::MemoryCommandSource cs;
            for (size_t i = 0, n = params.job.size(); i < n; ++i) {
                cs.addLine(params.job[i]);
            }

            bco->setIsProcedure(true);
            interpreter::StatementCompiler sc(cs);
            interpreter::DefaultStatementCompilationContext scc(session.world());
            scc.withFlag(scc.ExpressionsAreStatements);
            scc.withFlag(scc.LinearExecution);
            scc.withFlag(scc.LocalContext);
            sc.setOptimisationLevel(params.optimisationLevel);
            if (params.opt_preexecLoad) {
                scc.withFlag(scc.PreexecuteLoad);
            }
            sc.compileList(*bco, scc);
            sc.finishBCO(*bco, scc);
            result.push_back(bco.asPtr());
            session.log().write(afl::sys::Log::Trace, LOG_NAME, afl::string::Format(_("Compiled %d command%!1{s%}.").c_str(), params.job.size()));
        } else {
            // Files: compile files into individual BCOs
            for (size_t i = 0, n = params.job.size(); i < n; ++i) {
                // FIXME: when given .qs files, assemble. When given .qc files, load.
                afl::io::FileSystem& fs = session.world().fileSystem();
                String_t ext = getFileNameExtension(fs, params.job[i]);
                afl::base::Ref<afl::io::Stream> stream(fs.openFile(params.job[i], afl::io::FileSystem::OpenRead));
                if (ext == "qc") {
                    // Load header FIXME q&d
                    uint8_t header[14];
                    stream->fullRead(header);
                    if (std::memcmp(header, "CCobj\32\144\0\4\0", 10) != 0) {
                        throw afl::except::FileFormatException(*stream, _("Invalid file header"));
                    }
                    uint32_t bcoId = header[10] + 256*(header[11] + 256*(header[12] + 256*header[13]));
                    
                    game::interface::LoadContext lc(session);
                    interpreter::vmio::ObjectLoader loader(*params.gameCharset, lc);
                    loader.load(stream);
                    result.push_back(loader.getBCO(bcoId).asPtr());
                } else {
                    interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
                    afl::io::TextFile tf(*stream);
                    interpreter::FileCommandSource cs(tf);
                    bco->setFileName(params.job[i]);

                    try {
                        bco->setIsProcedure(true);
                        interpreter::StatementCompiler sc(cs);
                        interpreter::DefaultStatementCompilationContext scc(session.world());
                        scc.withFlag(scc.ExpressionsAreStatements);
                        scc.withFlag(scc.LinearExecution);
                        if (params.opt_preexecLoad) {
                            scc.withFlag(scc.PreexecuteLoad);
                        }
                        sc.setOptimisationLevel(params.optimisationLevel);
                        sc.compileList(*bco, scc);
                        sc.finishBCO(*bco, scc);
                        result.push_back(bco.asPtr());
                    }
                    catch (interpreter::Error& e) {
                        // Compiler error. Convert this exception to a FileProblemException.
                        // This causes the report to look like
                        //     c2export: file.q: line NN: Whatever
                        //     loaded by 'file2.q', line N
                        String_t msg = afl::string::Format(_("line %d: %s").c_str(), cs.getLineNumber(), e.what());
                        String_t trace = e.getTrace();
                        if (!trace.empty()) {
                            msg += "\n";
                            msg += trace;
                        }
                        throw afl::except::FileProblemException(tf.getName(), msg);
                    }
                }
            }
            session.log().write(afl::sys::Log::Trace, LOG_NAME, afl::string::Format(_("Compiled %d file%!1{s%}.").c_str(), params.job.size()));
        }
    }

    interpreter::BCORef_t mergeResult(const std::vector<interpreter::BCOPtr_t>& result)
    {
        if (result.size() == 1) {
            return *result[0];
        } else {
            interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
            for (size_t i = 0, n = result.size(); i < n; ++i) {
                // pushlit BCO
                // callind 0
                interpreter::SubroutineValue sv(*result[i]);
                bco->addPushLiteral(&sv);
                bco->addInstruction(interpreter::Opcode::maIndirect, interpreter::Opcode::miIMCall, 0);
            }
            return bco;
        }
    }

    String_t getOutputFileName(afl::io::FileSystem& fs, String_t input, const char* ext)
    {
        String_t dirName = fs.getDirectoryName(input);
        String_t fileName = fs.getFileName(input);
        String_t::size_type dot = fileName.rfind('.');
        if (dot != String_t::npos &&
            dot != 0 &&
            (afl::string::strCaseCompare(fileName.c_str()+dot, ".q") == 0
             || afl::string::strCaseCompare(fileName.c_str()+dot, ".qc") == 0
             || afl::string::strCaseCompare(fileName.c_str()+dot, ".qs") == 0))
        {
            fileName.erase(dot);
        }
        fileName += ext;
        return fs.makePathName(dirName, fileName);
    }

    void saveObjectFile(afl::sys::LogListener& log, afl::io::FileSystem& fs, String_t fileName, interpreter::BCORef_t bco, const Parameters& params)
    {
        // Prepare save
        interpreter::vmio::FileSaveContext fsc(*params.gameCharset);
        fsc.setDebugInformation(params.opt_debug);
        uint32_t bcoID = fsc.addBCO(*bco);
        log.write(log.Trace, LOG_NAME, afl::string::Format(_("Writing '%s', %d object%!1{s%}...").c_str(), fileName, fsc.getNumPreparedObjects()));

        // Create output file
        afl::base::Ref<afl::io::Stream> file = fs.openFile(fileName, afl::io::FileSystem::Create);
        uint8_t header[] = { 'C', 'C', 'o', 'b', 'j', 26, 100, 0, 4, 0,
                             uint8_t(bcoID),
                             uint8_t(bcoID>>8),
                             uint8_t(bcoID>>16),
                             uint8_t(bcoID>>24) };

        file->fullWrite(header);
        fsc.save(*file);
    }


    void saveAssemblerSource(afl::io::TextWriter& out, interpreter::BCORef_t bco, const Parameters& params)
    {
        interpreter::vmio::AssemblerSaveContext asc;
        asc.setDebugInformation(params.opt_debug);
        asc.addBCO(*bco);
        asc.save(out);
    }


    /*
     *  Execute Mode
     */
    int doExecMode(game::Session& session, const Parameters& params,
                   afl::sys::Environment& env, util::ProfileDirectory& profile)
    {
        // Compile
        std::vector<interpreter::BCOPtr_t> result;
        doCompile(session, params, result);
        interpreter::BCORef_t bco = mergeResult(result);

        // Set up game directories
        afl::io::FileSystem& fs = session.world().fileSystem();
        afl::string::Translator& tx = session.translator();

        String_t defaultRoot = fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs");
        game::v3::RootLoader loader(fs.openDirectory(params.arg_rootdir.orElse(defaultRoot)), profile, tx, session.log(), fs);

        // Check game data
        // FIXME: load correct config!
        const game::config::UserConfiguration uc;
        afl::base::Ptr<game::Root> root = loader.load(fs.openDirectory(params.arg_gamedir.orElse(".")), *params.gameCharset, uc, false);
        if (root.get() == 0 || root->getTurnLoader().get() == 0) {
            session.log().write(afl::sys::Log::Error, LOG_NAME, tx.translateString("no game data found"));
            return 1;
        }

        // Check player number
        int arg_race = params.playerNumber;
        if (arg_race != 0) {
            String_t extra;
            if (!root->getTurnLoader()->getPlayerStatus(arg_race, extra, tx).contains(game::TurnLoader::Available)) {
                session.log().write(afl::sys::Log::Error, LOG_NAME, afl::string::Format(tx.translateString("no game data available for player %d").c_str(), arg_race));
                return 1;
            }
        } else {
            arg_race = root->getTurnLoader()->getDefaultPlayer(root->playerList().getAllPlayers());
            if (arg_race == 0) {
                session.log().write(afl::sys::Log::Error, LOG_NAME, tx.translateString("please specify the player number"));
                return 1;
            }
        }

        // Make a session and load it
        session.setGame(new game::Game());
        session.setRoot(root);
        session.setShipList(new game::spec::ShipList());
        root->specificationLoader().loadShipList(*session.getShipList(), *root);

        root->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), arg_race, *root, session);
        session.getGame()->currentTurn().universe().postprocess(game::PlayerSet_t(arg_race), game::PlayerSet_t(arg_race), game::map::Object::Playable,
                                                                root->hostVersion(), root->hostConfiguration(),
                                                                session.getGame()->currentTurn().getTurnNumber(),
                                                                tx, session.log());
        session.getGame()->setViewpointPlayer(arg_race);

        // Execute the process
        interpreter::ProcessList& processList = session.world().processList();
        interpreter::Process& proc = processList.create(session.world(), tx.translateString("Console"));

        proc.pushFrame(bco, false);
        uint32_t pgid = processList.allocateProcessGroup();
        processList.resumeProcess(proc, pgid);
        processList.startProcessGroup(pgid);
        processList.run();

        int returnCode = 0;
        if (proc.getState() == interpreter::Process::Failed) {
            // Log exception
            session.logError(proc.getError());
            returnCode = 1;
        }
        processList.removeTerminatedProcesses();

        // FIXME: save stuff etc.
        // Check "readonly" option.
        return returnCode;
    }

    /*
     *  Compile Mode
     */
    int doCompileMode(game::Session& session, const Parameters& params)
    {
        // Environment
        afl::sys::LogListener& log = session.log();
        afl::io::FileSystem& fs = session.world().fileSystem();

        warnIgnoredOptions(log, params);

        // Compile
        std::vector<interpreter::BCOPtr_t> result;
        doCompile(session, params, result);

        // Produce output
        String_t output;
        if (params.arg_output.get(output)) {
            // Single output file given. If we have multiple BCO's, merge.
            interpreter::BCORef_t bco = mergeResult(result);
            saveObjectFile(log, fs, output, bco, params);
            return 0;
        } else if (params.opt_commands) {
            // No output file given, input is commands
            log.write(afl::sys::Log::Error, LOG_NAME, session.translator().translateString("must specify an output file ('-o FILE') if input is commands"));
            return 1;
        } else {
            // No output file given, input is files. Generate output file names.
            for (size_t i = 0; i < result.size() && i < params.job.size(); ++i) {
                saveObjectFile(log, fs, getOutputFileName(fs, params.job[i], ".qc"), *result[i], params);
            }
            return 0;
        }
    }

    /*
     *  Disassemble Mode
     */
    int doDisassembleMode(game::Session& session, const Parameters& params, afl::io::TextWriter& standardOutput)
    {
        warnIgnoredOptions(session.log(), params);

        // Compile
        std::vector<interpreter::BCOPtr_t> result;
        doCompile(session, params, result);

        // Merge everything
        interpreter::BCORef_t bco = mergeResult(result);

        // Produce output
        String_t output;
        if (params.arg_output.get(output)) {
            // Save to file
            afl::base::Ref<afl::io::Stream> file = session.world().fileSystem().openFile(output, afl::io::FileSystem::Create);
            afl::io::TextFile text(*file);
            saveAssemblerSource(text, bco, params);
        } else {
            // Send to console
            saveAssemblerSource(standardOutput, bco, params);
        }
        return 0;
    }

    class ConsoleScriptApplication : public util::Application {
     public:
        ConsoleScriptApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            {
                consoleLogger().setConfiguration("*=raw");
            }

        virtual void appMain();

     private:
        String_t fetchArg(const char* opt, afl::sys::CommandLineParser& parser);
        void help();
    };
}

void
ConsoleScriptApplication::appMain()
{
    util::ProfileDirectory profile(environment(), fileSystem(), translator(), log());

    // Parameters
    Parameters params;

    // Parser
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "exec" || p == "run") {
                params.mode = ExecMode;
            } else if (p == "compile" || p == "c") {
                params.mode = CompileMode;
            } else if (p == "disassemble" || p == "S") {
                params.mode = DisassembleMode;
            } else if (p == "dump") {
                params.mode = DumpMode;
            } else if (p == "g") {
                params.opt_debug = true;
            } else if (p == "s") {
                params.opt_debug = false;
            } else if (p == "f") {
                String_t arg = fetchArg("-f", commandLine);
                if (arg == "preexec-load") {
                    params.opt_preexecLoad = true;
                } else {
                    errorExit(afl::string::Format(_("invalid option '%s' specified. Use '%s -h' for help.").c_str(), "-f " + arg, environment().getInvocationName()));
                }
            } else if (p == "I") {
                params.loadPath.push_back(fetchArg("-I", commandLine));
            } else if (p == "o") {
                params.arg_output = fetchArg("-o", commandLine);
            } else if (p == "G" || p == "game") {
                params.arg_gamedir = fetchArg(p.c_str(), commandLine);
            } else if (p == "R" || p == "root") {
                params.arg_rootdir = fetchArg(p.c_str(), commandLine);
            } else if (p == "P" || p == "player") {
                String_t arg;
                int value = 0;
                if (!commandLine.getParameter(arg)
                    || !afl::string::strToInteger(arg, value)
                    || value <= 0
                    || value > game::MAX_PLAYERS)
                {
                    errorExit(_("option '-P' needs a player number as parameter").c_str());
                }
                params.playerNumber = value;
            } else if (p == "C") {
                if (afl::charset::Charset* cs = util::CharsetFactory().createCharset(fetchArg("-C", commandLine))) {
                    params.gameCharset.reset(cs);
                } else {
                    errorExit(_("the specified character set is not known"));
                }
            } else if (p == "O") {
                String_t arg;
                int value = 0;
                if (!commandLine.getParameter(arg)
                    || !afl::string::strToInteger(arg, value)
                    || value < interpreter::StatementCompiler::MIN_OPTIMISATION_LEVEL
                    || value > interpreter::StatementCompiler::MAX_OPTIMISATION_LEVEL)
                {
                    errorExit(afl::string::Format(_("option '-O' needs a number between %d and %d as parameter").c_str(),
                                                  interpreter::StatementCompiler::MIN_OPTIMISATION_LEVEL,
                                                  interpreter::StatementCompiler::MAX_OPTIMISATION_LEVEL));
                }
                params.optimisationLevel = value;
            } else if (p == "k") {
                params.opt_commands = true;
            } else if (p == "log") {
                try {
                    consoleLogger().setConfiguration(fetchArg("--log", commandLine));
                }
                catch (std::exception& e) {
                    errorExit(_("parameter to '--log' is not valid"));
                }
            } else if (p == "readonly" || p == "read-only") {
                params.opt_readonly = true;
            } else if (p == "q") {
                consoleLogger().setConfiguration("script*@Info+=raw:*=hide");
            } else if (p == "h" || p == "help") {
                help();
            } else {
                errorExit(afl::string::Format(_("invalid option '%s' specified. Use '%s -h' for help.").c_str(), p, environment().getInvocationName()));
            }
        } else {
            params.job.push_back(p);
        }
    }

    if (params.job.empty()) {
        if (params.opt_commands) {
            errorExit(afl::string::Format(_("no commands specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
        } else {
            errorExit(afl::string::Format(_("no input files specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
        }
    }

    // Make a game session.
    // Making a session means we can re-use the Session's initialisation of special commands.
    // Also, interpreter objects are not intended to outlive a session.
    afl::io::FileSystem& fs = fileSystem();
    game::Session session(translator(), fs);
    session.log().addListener(log());
    if (!params.loadPath.empty()) {
        if (params.loadPath.size() == 1U) {
            session.world().setSystemLoadDirectory(fs.openDirectory(params.loadPath[0]).asPtr());
        } else {
            afl::base::Ref<afl::io::MultiDirectory> dir = afl::io::MultiDirectory::create();
            for (size_t i = 0, n = params.loadPath.size(); i < n; ++i) {
                dir->addDirectory(fs.openDirectory(params.loadPath[i]));
            }
            session.world().setSystemLoadDirectory(dir.asPtr());
        }
    }
    int result = 0;
    switch (params.mode) {
     case ExecMode:
        result = doExecMode(session, params, environment(), profile);
        break;
     case CompileMode:
        result = doCompileMode(session, params);
        break;
     case DisassembleMode:
        consoleLogger().setConfiguration("*@Warn+=raw:*=drop");
        result = doDisassembleMode(session, params, standardOutput());
        break;
     case DumpMode:
        errorExit("FIXME: NOT IMPLEMENTED");
        break;
    }
    exit(result);
}

String_t
ConsoleScriptApplication::fetchArg(const char* opt, afl::sys::CommandLineParser& parser)
{
    String_t result;
    if (!parser.getParameter(result)) {
        errorExit(afl::string::Format(_("option '%s' needs an argument").c_str(), opt));
    }
    return result;
}

/** Exit with help message. */
void
ConsoleScriptApplication::help()
{
    const String_t options =
        util::formatOptions(_("Actions:\n"
                              "--exec, --run\tExecute (default)\n"
                              "--compile, -c\tCompile to \"*.qc\" files\n"
                              "--disassemble, -S\tDisassemble to \"*.qs\" files\n"
                              // "--dump\tDump data\n"
                              "\n"
                              "Options:\n"
                              "-g\tEnable debug info (default)\n"
                              "-s\tDisable debug info\n"
                              "-o FILE\tOutput file\n"
                              "--game/-G DIR\tGame directory\n"
                              "--root/-R DIR\tRoot direcory\n"
                              "--player/-P NUM\tPlayer number\n"
                              "--readonly\tOpen game data read-only\n"
                              "-I DIR\tInclude (load) directory\n"
                              "--charset/-C CS\tSet game character set\n"
                              "-O LVL\tOptimisation level\n"
                              "-k\tExecute commands, not files\n"
                              "--log CONFIG\tConfigure log output\n"
                              "-q\tQuiet; show only script output (predefined log config)\n"
                              "\n"
                              "Expert Options:\n"
                              "-f preexec-load\tPre-execute \"Load\" statements\n"));
    
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(afl::string::Format(_("PCC2 Script Engine v%s - (c) 2017-2018 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(afl::string::Format(_("Usage:\n"
                                        "  %s [-h]\n"
                                        "  %$0s [-ACTION] [-OPTIONS] FILE...\n"
                                        "  %$0s [-ACTION] [-OPTIONS] -k COMMAND...\n\n"
                                        "%s"
                                        "\n"
                                        "Report bugs to <Streu@gmx.de>").c_str(),
                                      environment().getInvocationName(),
                                      options));
    exit(0);
}

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return ConsoleScriptApplication(env, fs).run();
}
