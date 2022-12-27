/**
  *  \file interpreter/consoleapplication.cpp
  *  \brief Class interpreter::ConsoleApplication
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
#include "interpreter/consoleapplication.hpp"
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
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/interface/consolecommands.hpp"
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
#include "version.hpp"

using afl::base::Optional;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::Environment;
using afl::sys::LogListener;
using interpreter::BCOPtr_t;
using interpreter::BCORef_t;
using interpreter::ConsoleApplication;

struct interpreter::ConsoleApplication::Parameters {
    enum Mode {
        ExecMode,          //   --exec (default)   Execute
        CompileMode,       //   --compile, -c      Produce "*.qc" files
        DisassembleMode    //   --disassemble, -S  Produce "*.qs" files
    };

    Optional<String_t> arg_gamedir;                    // -G
    Optional<String_t> arg_rootdir;                    // -R
    Optional<String_t> arg_output;                     // -o
    bool opt_debug;                                    // -g/-s
    bool opt_commands;                                 // -k
    bool opt_preexecLoad;                              // -fpreexec-load
    bool opt_readonly;                                 // --readonly
    bool opt_nostdlib;                                 // --nostdlib
    Mode mode;
    std::auto_ptr<afl::charset::Charset> gameCharset;  // -C
    std::vector<String_t> loadPath;                    // -I
    std::vector<String_t> job;                         // list of files/commands
    int optimisationLevel;                             // -O
    int playerNumber;                                  // -P

    Parameters()
        : arg_gamedir(),
          arg_rootdir(),
          arg_output(),
          opt_debug(true),
          opt_commands(false),
          opt_preexecLoad(false),
          opt_readonly(false),
          opt_nostdlib(false),
          mode(ExecMode),
          gameCharset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1)),
          loadPath(),
          job(),
          optimisationLevel(1),
          playerNumber(0)
        { }
};

namespace {
    const char LOG_NAME[] = "script";

    /* Warn for ignored options.
       In compile or disassemble mode, we ignore a specified game. */
    void warnIgnoredOptions(LogListener& log, const ConsoleApplication::Parameters& params, Translator& tx)
    {
        if (params.arg_gamedir.isValid()) {
            log.write(log.Warn, LOG_NAME, tx("Game directory ('-G') name has been ignored"));
        }
        if (params.arg_rootdir.isValid()) {
            log.write(log.Warn, LOG_NAME, tx("Root directory ('-R') name has been ignored"));
        }
        if (params.playerNumber > 0) {
            log.write(log.Warn, LOG_NAME, tx("Player number ('-P') has been ignored"));
        }
    }

    /* Get file name extension for a file name */
    String_t getFileNameExtension(FileSystem& fs, String_t input)
    {
        String_t fileName = fs.getFileName(input);
        String_t::size_type dot = fileName.rfind('.');
        if (dot != String_t::npos && dot != 0) {
            return fileName.substr(dot+1);
        } else {
            return String_t();
        }
    }

    /* Compile the given job into a list of BCOs. */
    void doCompile(game::Session& session, const ConsoleApplication::Parameters& params, std::vector<BCOPtr_t>& result)
    {
        // Commands or files?
        if (params.opt_commands) {
            // Commands: compile everything into one single BCO
            BCORef_t bco = interpreter::BytecodeObject::create(true);
            interpreter::MemoryCommandSource cs;
            for (size_t i = 0, n = params.job.size(); i < n; ++i) {
                cs.addLine(params.job[i]);
            }

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
            session.log().write(LogListener::Trace, LOG_NAME, Format(session.translator()("Compiled %d command%!1{s%}.").c_str(), params.job.size()));
        } else {
            // Files: compile files into individual BCOs
            for (size_t i = 0, n = params.job.size(); i < n; ++i) {
                FileSystem& fs = session.world().fileSystem();
                String_t ext = getFileNameExtension(fs, params.job[i]);
                Ref<afl::io::Stream> stream(fs.openFile(params.job[i], FileSystem::OpenRead));
                if (ext == "qc") {
                    // Load object file
                    game::interface::LoadContext lc(session);
                    interpreter::vmio::ObjectLoader loader(*params.gameCharset, session.translator(), lc);
                    result.push_back(loader.loadObjectFile(stream).asPtr());
                } else {
                    // Compile source file
                    BCORef_t bco = interpreter::BytecodeObject::create(true);
                    afl::io::TextFile tf(*stream);
                    interpreter::FileCommandSource cs(tf);
                    bco->setFileName(params.job[i]);

                    try {
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
                        // Compiler error. Convert this exception to a FileProblemException; framework will log it in "prog: file: line: msg" format.
                        // For a normal error in the file-given-on-command-line, this will log
                        //     c2script: file-given-on-command-line.q: line NN: Whatever
                        // which is what we want. For an included file (-fpreexec-load), this causes the report to look like
                        //     c2script: file-given-on-command-line.q: line NN: Whatever
                        //     in file 'loaded-file.q', line N
                        // This is suboptimal, but the best we can do for now.
                        String_t msg = Format(session.translator()("line %d: %s").c_str(), cs.getLineNumber(), e.what());
                        String_t trace = e.getTrace();
                        if (!trace.empty()) {
                            msg += "\n";
                            msg += trace;
                        }
                        throw afl::except::FileProblemException(tf.getName(), msg);
                    }
                }
            }
            session.log().write(LogListener::Trace, LOG_NAME, Format(session.translator()("Compiled %d file%!1{s%}.").c_str(), params.job.size()));
        }
    }

    /* Merge a list-of-BCOs into a single BCO: if multiple have been generated, make a single BCO that calls them all. */
    BCORef_t mergeResult(const std::vector<BCOPtr_t>& result)
    {
        if (result.size() == 1) {
            return *result[0];
        } else {
            BCORef_t bco = interpreter::BytecodeObject::create(true);
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

    /* Generate output file name, given input file name. */
    String_t getOutputFileName(FileSystem& fs, String_t input, const char* ext)
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

    /* Save an object file, starting with a given BCO.
       Saves the transitive closure of that BCO. */
    void saveObjectFile(LogListener& log, FileSystem& fs, String_t fileName, BCORef_t bco, const ConsoleApplication::Parameters& params, Translator& tx)
    {
        // Prepare save
        interpreter::vmio::FileSaveContext fsc(*params.gameCharset);
        fsc.setDebugInformation(params.opt_debug);
        uint32_t bcoID = fsc.addBCO(*bco);
        log.write(log.Trace, LOG_NAME, Format(tx("Writing '%s', %d object%!1{s%}...").c_str(), fileName, fsc.getNumPreparedObjects()));

        // Create output file
        Ref<afl::io::Stream> file = fs.openFile(fileName, FileSystem::Create);
        fsc.saveObjectFile(*file, bcoID);
    }

    /* Save assembler source, starting with a given BCO.
       Saves the transitive closure of that BCO. */
    void saveAssemblerSource(afl::io::TextWriter& out, BCORef_t bco, const ConsoleApplication::Parameters& params)
    {
        interpreter::vmio::AssemblerSaveContext asc;
        asc.setDebugInformation(params.opt_debug);
        asc.addBCO(*bco);
        asc.save(out);
    }


    /*
     *  Execute Mode
     */
    int doExecMode(game::Session& session, const ConsoleApplication::Parameters& params, Environment& env, util::ProfileDirectory& profile)
    {
        // Compile
        std::vector<BCOPtr_t> result;
        doCompile(session, params, result);
        BCORef_t bco = mergeResult(result);

        // Set up game directories
        FileSystem& fs = session.world().fileSystem();
        Translator& tx = session.translator();

        String_t defaultRoot = fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs");
        game::v3::RootLoader loader(fs.openDirectory(params.arg_rootdir.orElse(defaultRoot)), &profile, 0 /* FIXME: pass proper callback? */, tx, session.log(), fs);

        // Check game data
        // FIXME: load correct config!
        const game::config::UserConfiguration uc;
        afl::base::Ptr<game::Root> root = loader.load(fs.openDirectory(fs.getAbsolutePathName(params.arg_gamedir.orElse("."))), *params.gameCharset, uc, false);
        if (root.get() == 0 || root->getTurnLoader().get() == 0) {
            session.log().write(LogListener::Error, LOG_NAME, tx.translateString("no game data found"));
            return 1;
        }

        // Check player number
        int arg_race = params.playerNumber;
        if (arg_race != 0) {
            String_t extra;
            if (!root->getTurnLoader()->getPlayerStatus(arg_race, extra, tx).contains(game::TurnLoader::Available)) {
                session.log().write(LogListener::Error, LOG_NAME, Format(tx.translateString("no game data available for player %d").c_str(), arg_race));
                return 1;
            }
        } else {
            arg_race = root->getTurnLoader()->getDefaultPlayer(root->playerList().getAllPlayers());
            if (arg_race == 0) {
                session.log().write(LogListener::Error, LOG_NAME, tx.translateString("please specify the player number"));
                return 1;
            }
        }

        // Make a session and load it
        bool ok = false;
        session.setGame(new game::Game());
        session.setRoot(root);
        session.setShipList(new game::spec::ShipList());
        root->specificationLoader().loadShipList(*session.getShipList(), *root, game::makeResultTask(ok))->call();
        if (!ok) {
            throw game::Exception(tx("unable to load ship list"));
        }

        ok = false;
        root->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), arg_race, *root, session, game::makeResultTask(ok))->call();
        if (!ok) {
            throw game::Exception(tx("unable to load turn"));
        }

        session.postprocessTurn(session.getGame()->currentTurn(), game::PlayerSet_t(arg_race), game::PlayerSet_t(arg_race), game::map::Object::Playable);
        session.getGame()->setViewpointPlayer(arg_race);

        // Execute the process
        interpreter::ProcessList& processList = session.processList();
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
    int doCompileMode(game::Session& session, const ConsoleApplication::Parameters& params)
    {
        // Environment
        LogListener& log = session.log();
        FileSystem& fs = session.world().fileSystem();

        warnIgnoredOptions(log, params, session.translator());

        // Compile
        std::vector<BCOPtr_t> result;
        doCompile(session, params, result);

        // Produce output
        String_t output;
        if (params.arg_output.get(output)) {
            // Single output file given. If we have multiple BCO's, merge.
            BCORef_t bco = mergeResult(result);
            saveObjectFile(log, fs, output, bco, params, session.translator());
            return 0;
        } else if (params.opt_commands) {
            // No output file given, input is commands
            log.write(LogListener::Error, LOG_NAME, session.translator()("must specify an output file ('-o FILE') if input is commands"));
            return 1;
        } else {
            // No output file given, input is files. Generate output file names.
            for (size_t i = 0; i < result.size() && i < params.job.size(); ++i) {
                saveObjectFile(log, fs, getOutputFileName(fs, params.job[i], ".qc"), *result[i], params, session.translator());
            }
            return 0;
        }
    }

    /*
     *  Disassemble Mode
     */
    int doDisassembleMode(game::Session& session, const ConsoleApplication::Parameters& params, afl::io::TextWriter& standardOutput)
    {
        warnIgnoredOptions(session.log(), params, session.translator());

        // Compile
        std::vector<BCOPtr_t> result;
        doCompile(session, params, result);

        // Merge everything
        BCORef_t bco = mergeResult(result);

        // Produce output
        String_t output;
        if (params.arg_output.get(output)) {
            // Save to file
            Ref<afl::io::Stream> file = session.world().fileSystem().openFile(output, FileSystem::Create);
            afl::io::TextFile text(*file);
            saveAssemblerSource(text, bco, params);
        } else {
            // Send to console
            saveAssemblerSource(standardOutput, bco, params);
        }
        return 0;
    }
}


/*
 *  ConsoleApplication
 */

interpreter::ConsoleApplication::ConsoleApplication(afl::sys::Environment& env, FileSystem& fs)
    : Application(env, fs)
{
    consoleLogger().setConfiguration("*=raw", translator());
}

void
interpreter::ConsoleApplication::appMain()
{
    util::ProfileDirectory profile(environment(), fileSystem(), translator(), log());
    Translator& tx = translator();

    // Parameters
    Parameters params;
    parseParameters(params);
    if (params.job.empty()) {
        if (params.opt_commands) {
            errorExit(Format(tx("no commands specified. Use '%s -h' for help."), environment().getInvocationName()));
        } else {
            errorExit(Format(tx("no input files specified. Use '%s -h' for help."), environment().getInvocationName()));
        }
    }

    // Make a game session.
    // Making a session means we can re-use the Session's initialisation of special commands.
    // Also, interpreter objects are not intended to outlive a session.
    FileSystem& fs = fileSystem();
    game::Session session(tx, fs);
    session.log().addListener(log());
    session.sig_runRequest.add(&session.processList(), &ProcessList::run);

    // If we are in exec mode, inject core.q
    if (!params.opt_nostdlib && params.mode == Parameters::ExecMode) {
        String_t corePath = fs.makePathName(fs.makePathName(environment().getInstallationDirectoryName(), "share"), "resource");
        params.loadPath.insert(params.loadPath.begin(), corePath);
        if (params.opt_commands) {
            params.job.insert(params.job.begin(), "Load 'core.q'");
        } else {
            params.job.insert(params.job.begin(), fs.makePathName(corePath, "core.q"));
        }
    }

    // Register console commands.
    // If attachXXX throws, we're caught by the main loop.
    game::interface::registerConsoleCommands(session, environment().attachTextReader(Environment::Input), environment().attachTextWriter(Environment::Output));

    // Build load path
    if (!params.loadPath.empty()) {
        if (params.loadPath.size() == 1U) {
            session.world().setSystemLoadDirectory(fs.openDirectory(params.loadPath[0]).asPtr());
        } else {
            Ref<afl::io::MultiDirectory> dir = afl::io::MultiDirectory::create();
            for (size_t i = 0, n = params.loadPath.size(); i < n; ++i) {
                dir->addDirectory(fs.openDirectory(params.loadPath[i]));
            }
            session.world().setSystemLoadDirectory(dir.asPtr());
        }
    }
    int result = 0;
    switch (params.mode) {
     case Parameters::ExecMode:
        result = doExecMode(session, params, environment(), profile);
        break;
     case Parameters::CompileMode:
        result = doCompileMode(session, params);
        break;
     case Parameters::DisassembleMode:
        consoleLogger().setConfiguration("*@Warn+=raw:*=drop", translator());
        result = doDisassembleMode(session, params, standardOutput());
        break;
    }
    exit(result);
}

/** Parse parameters.
    @params [out] Parameters */
void
interpreter::ConsoleApplication::parseParameters(Parameters& params)
{
    Translator& tx = translator();
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "exec" || p == "run") {
                params.mode = Parameters::ExecMode;
            } else if (p == "compile" || p == "c") {
                params.mode = Parameters::CompileMode;
            } else if (p == "disassemble" || p == "S") {
                params.mode = Parameters::DisassembleMode;
            } else if (p == "g") {
                params.opt_debug = true;
            } else if (p == "s") {
                params.opt_debug = false;
            } else if (p == "f") {
                String_t arg = commandLine.getRequiredParameter(p);
                if (arg == "preexec-load") {
                    params.opt_preexecLoad = true;
                } else {
                    errorExit(Format(tx("invalid option '%s' specified. Use '%s -h' for help."), "-f " + arg, environment().getInvocationName()));
                }
            } else if (p == "I") {
                params.loadPath.push_back(commandLine.getRequiredParameter(p));
            } else if (p == "o") {
                params.arg_output = commandLine.getRequiredParameter(p);
            } else if (p == "nostdlib") {
                params.opt_nostdlib = true;
            } else if (p == "G" || p == "game") {
                params.arg_gamedir = commandLine.getRequiredParameter(p);
            } else if (p == "R" || p == "root") {
                params.arg_rootdir = commandLine.getRequiredParameter(p);
            } else if (p == "P" || p == "player") {
                String_t arg;
                int value = 0;
                if (!commandLine.getParameter(arg)
                    || !afl::string::strToInteger(arg, value)
                    || value <= 0
                    || value > game::MAX_PLAYERS)
                {
                    errorExit(tx("option '-P' needs a player number as parameter"));
                }
                params.playerNumber = value;
            } else if (p == "C") {
                if (afl::charset::Charset* cs = util::CharsetFactory().createCharset(commandLine.getRequiredParameter(p))) {
                    params.gameCharset.reset(cs);
                } else {
                    errorExit(tx("the specified character set is not known"));
                }
            } else if (p == "O") {
                String_t arg;
                int value = 0;
                if (!commandLine.getParameter(arg)
                    || !afl::string::strToInteger(arg, value)
                    || value < StatementCompiler::MIN_OPTIMISATION_LEVEL
                    || value > StatementCompiler::MAX_OPTIMISATION_LEVEL)
                {
                    errorExit(Format(tx("option '-O' needs a number between %d and %d as parameter"),
                                     StatementCompiler::MIN_OPTIMISATION_LEVEL,
                                     StatementCompiler::MAX_OPTIMISATION_LEVEL));
                }
                params.optimisationLevel = value;
            } else if (p == "k") {
                params.opt_commands = true;
            } else if (p == "log") {
                try {
                    consoleLogger().setConfiguration(commandLine.getRequiredParameter(p), tx);
                }
                catch (std::exception& e) {
                    errorExit(tx("parameter to '--log' is not valid"));
                }
            } else if (p == "readonly" || p == "read-only") {
                params.opt_readonly = true;
            } else if (p == "q") {
                consoleLogger().setConfiguration("script*@Info+=raw:*=hide", tx);
            } else if (p == "h" || p == "help") {
                help();
            } else {
                errorExit(Format(tx("invalid option '%s' specified. Use '%s -h' for help."), p, environment().getInvocationName()));
            }
        } else {
            params.job.push_back(p);
        }
    }
}

/** Exit with help message. */
void
interpreter::ConsoleApplication::help()
{
    Translator& tx = translator();
    const String_t options =
        util::formatOptions(tx("Actions:\n"
                               "--exec, --run\tExecute (default)\n"
                               "--compile, -c\tCompile to \"*.qc\" files\n"
                               "--disassemble, -S\tDisassemble to \"*.qs\" files\n"
                               "\n"
                               "Options:\n"
                               "-g\tEnable debug info (default)\n"
                               "-s\tDisable debug info\n"
                               "-o FILE\tOutput file\n"
                               "--game/-G DIR\tGame directory\n"
                               "--root/-R DIR\tRoot direcory\n"
                               "--player/-P NUM\tPlayer number\n"
                               "--readonly\tOpen game data read-only\n"
                               "--nostdlib\tDo not load standard library (exec mode only)\n"
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
    out.writeLine(Format(tx("PCC2 Script Engine v%s - (c) 2017-2022 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
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
