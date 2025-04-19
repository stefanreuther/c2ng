/**
  *  \file game/interface/scriptapplication.cpp
  *  \brief Class game::interface::ScriptApplication
  */

#include <stdexcept>
#include <vector>
#include "game/interface/scriptapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
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
#include "interpreter/vmio/objectloader.hpp"
#include "util/charsetfactory.hpp"
#include "util/consolelogger.hpp"
#include "util/io.hpp"
#include "util/profiledirectory.hpp"
#include "util/string.hpp"
#include "version.hpp"
#include "interpreter/coveragerecorder.hpp"

using afl::base::Optional;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::Environment;
using afl::sys::LogListener;
using interpreter::BCOPtr_t;
using interpreter::BCORef_t;
using game::interface::ScriptApplication;

struct game::interface::ScriptApplication::Parameters {
    Optional<String_t> arg_gamedir;                    // -G
    Optional<String_t> arg_rootdir;                    // -R
    bool opt_commands;                                 // -k
    bool opt_readonly;                                 // --readonly
    bool opt_nostdlib;                                 // --nostdlib
    std::auto_ptr<afl::charset::Charset> gameCharset;  // -C
    std::vector<String_t> loadPath;                    // -I
    std::vector<String_t> job;                         // list of files/commands
    int optimisationLevel;                             // -O
    int playerNumber;                                  // -P
    Optional<String_t> coverageFile;                   // --coverage
    String_t coverageTestName;                         // --coverage-test-name

    Parameters()
        : arg_gamedir(),
          arg_rootdir(),
          opt_commands(false),
          opt_readonly(false),
          opt_nostdlib(false),
          gameCharset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1)),
          loadPath(),
          job(),
          optimisationLevel(1),
          playerNumber(0),
          coverageFile(),
          coverageTestName()
        { }
};

namespace {
    const char LOG_NAME[] = "script";

    class CoverageRunner : public afl::base::Closure<void()> {
     public:
        CoverageRunner(game::Session& session, interpreter::CoverageRecorder& rec)
            : m_session(session), m_recorder(rec)
            { }

        void call()
            { m_session.processList().run(&m_recorder); }
     private:
        game::Session& m_session;
        interpreter::CoverageRecorder& m_recorder;
    };

    /* Compile the given job into a list of BCOs. */
    void doCompile(game::Session& session, const ScriptApplication::Parameters& params, std::vector<BCOPtr_t>& result)
    {
        // Default parameters
        interpreter::DefaultStatementCompilationContext scc(session.world());
        scc.withFlag(scc.ExpressionsAreStatements);
        scc.withFlag(scc.LinearExecution);
        scc.withFlag(scc.LocalContext);

        // Commands or files?
        if (params.opt_commands) {
            // Commands: compile everything into one single BCO
            BCORef_t bco = interpreter::BytecodeObject::create(true);
            interpreter::MemoryCommandSource cs;
            for (size_t i = 0, n = params.job.size(); i < n; ++i) {
                cs.addLine(params.job[i]);
            }

            interpreter::StatementCompiler sc(cs);
            sc.setOptimisationLevel(params.optimisationLevel);
            sc.compileList(*bco, scc);
            sc.finishBCO(*bco, scc);
            result.push_back(bco.asPtr());
            session.log().write(LogListener::Debug, LOG_NAME, Format(session.translator()("Compiled %d command%!1{s%}.").c_str(), params.job.size()));
        } else {
            // Files: compile files into individual BCOs
            for (size_t i = 0, n = params.job.size(); i < n; ++i) {
                FileSystem& fs = session.world().fileSystem();
                String_t ext = util::getFileNameExtension(fs, params.job[i]);
                Ref<afl::io::Stream> stream(fs.openFile(params.job[i], FileSystem::OpenRead));
                if (ext == ".qc") {
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
            session.log().write(LogListener::Debug, LOG_NAME, Format(session.translator()("Compiled %d file%!1{s%}.").c_str(), params.job.size()));
        }
    }


    /*
     *  Execute Mode
     */
    int doExecMode(game::Session& session, const ScriptApplication::Parameters& params, Environment& env, util::ProfileDirectory& profile)
    {
        // Compile
        std::vector<BCOPtr_t> result;
        doCompile(session, params, result);
        BCORef_t bco = mergeByteCodeObjects(result);

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

        // Coverage?
        std::auto_ptr<interpreter::CoverageRecorder> pCoverage;
        if (params.coverageFile.isValid()) {
            pCoverage.reset(new interpreter::CoverageRecorder());
            pCoverage->addBCO(*bco);
            session.setNewScriptRunner(new CoverageRunner(session, *pCoverage));
        }

        // Execute the process
        interpreter::ProcessList& processList = session.processList();
        interpreter::Process& proc = processList.create(session.world(), tx.translateString("Console"));

        proc.pushFrame(bco, false);
        uint32_t pgid = processList.allocateProcessGroup();
        processList.resumeProcess(proc, pgid);
        processList.startProcessGroup(pgid);
        session.runScripts();

        int returnCode = 0;
        if (proc.getState() == interpreter::Process::Failed) {
            // Log exception
            session.logError(proc.getError());
            returnCode = 1;
        }
        processList.removeTerminatedProcesses();
        session.setNewScriptRunner(0);

        // Save coverage
        if (pCoverage.get() != 0) {
            afl::base::Ref<afl::io::Stream> out = session.world().fileSystem().openFile(params.coverageFile.orElse(""), afl::io::FileSystem::Create);
            pCoverage->save(*out, params.coverageTestName);
        }

        // FIXME: save stuff etc.
        // Check "readonly" option.
        return returnCode;
    }
}


/*
 *  ScriptApplication
 */

game::interface::ScriptApplication::ScriptApplication(afl::sys::Environment& env, FileSystem& fs)
    : Application(env, fs)
{
    consoleLogger().setConfiguration("*=raw", translator());
}

void
game::interface::ScriptApplication::appMain()
{
    util::ProfileDirectory profile(environment(), fileSystem());
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
    Session session(tx, fs);
    session.log().addListener(log());

    // If we are in exec mode, inject core.q
    if (!params.opt_nostdlib) {
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
    registerConsoleCommands(session, environment().attachTextReader(Environment::Input), environment().attachTextWriter(Environment::Output));

    // Build load path
    session.world().setSystemLoadDirectory(util::makeSearchDirectory(fs, params.loadPath).asPtr());
    int result = doExecMode(session, params, environment(), profile);
    exit(result);
}

/** Parse parameters.
    @param [out] params Parameters */
void
game::interface::ScriptApplication::parseParameters(Parameters& params)
{
    Translator& tx = translator();
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "I") {
                params.loadPath.push_back(commandLine.getRequiredParameter(p));
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
                    || value > MAX_PLAYERS)
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
                    || value < interpreter::StatementCompiler::MIN_OPTIMISATION_LEVEL
                    || value > interpreter::StatementCompiler::MAX_OPTIMISATION_LEVEL)
                {
                    errorExit(Format(tx("option '-O' needs a number between %d and %d as parameter"),
                                     interpreter::StatementCompiler::MIN_OPTIMISATION_LEVEL,
                                     interpreter::StatementCompiler::MAX_OPTIMISATION_LEVEL));
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
            } else if (p == "coverage") {
                params.coverageFile = commandLine.getRequiredParameter(p);
            } else if (p == "coverage-test-name") {
                params.coverageTestName = commandLine.getRequiredParameter(p);
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
game::interface::ScriptApplication::help()
{
    Translator& tx = translator();
    const String_t options =
        util::formatOptions(tx("Options:\n"
                               "--game/-G DIR\tGame directory\n"
                               "--root/-R DIR\tRoot direcory\n"
                               "--player/-P NUM\tPlayer number\n"
                               "--readonly\tOpen game data read-only\n"
                               "--nostdlib\tDo not load standard library (core.q)\n"
                               "-I DIR\tInclude (load) directory\n"
                               "--charset/-C CS\tSet game character set\n"
                               "--coverage FILE.info\tProduce coverage report\n"
                               "--coverage-test-name NAME\tTest name to write to coverage report\n"
                               "-O LVL\tOptimisation level\n"
                               "-k\tExecute commands, not files\n"
                               "--log CONFIG\tConfigure log output\n"
                               "-q\tQuiet; show only script output (predefined log config)\n"));

    afl::io::TextWriter& out = standardOutput();
    out.writeLine(Format(tx("PCC2 Script Engine v%s - (c) 2017-2024 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s [-OPTIONS] FILE...\n"
                            "  %$0s [-OPTIONS] -k COMMAND...\n\n"
                            "%s"
                            "\n"
                            "Report bugs to <Streu@gmx.de>").c_str(),
                         environment().getInvocationName(),
                         options));
    exit(0);
}
