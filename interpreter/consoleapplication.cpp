/**
  *  \file interpreter/consoleapplication.cpp
  *  \brief Class interpreter::ConsoleApplication
  */

#include <stdexcept>
#include <vector>
#include "interpreter/consoleapplication.hpp"
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
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/vmio/assemblersavecontext.hpp"
#include "interpreter/vmio/filesavecontext.hpp"
#include "interpreter/vmio/nullloadcontext.hpp"
#include "interpreter/vmio/objectloader.hpp"
#include "interpreter/world.hpp"
#include "util/application.hpp"
#include "util/charsetfactory.hpp"
#include "util/consolelogger.hpp"
#include "util/io.hpp"
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
        CompileMode,       //   --compile, -c      Produce "*.qc" files
        DisassembleMode    //   --disassemble, -S  Produce "*.qs" files
    };

    Optional<String_t> arg_output;                     // -o
    bool opt_debug;                                    // -g/-s
    bool opt_commands;                                 // -k
    bool opt_preexecLoad;                              // -fpreexec-load
    Mode mode;
    std::auto_ptr<afl::charset::Charset> gameCharset;  // -C
    std::vector<String_t> loadPath;                    // -I
    std::vector<String_t> job;                         // list of files/commands
    int optimisationLevel;                             // -O

    Parameters()
        : arg_output(),
          opt_debug(true),
          opt_commands(false),
          opt_preexecLoad(false),
          mode(CompileMode),
          gameCharset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1)),
          loadPath(),
          job(),
          optimisationLevel(1)
        { }
};

namespace {
    const char LOG_NAME[] = "script";

    /* Compile the given job into a list of BCOs. */
    void doCompile(interpreter::World& world, const ConsoleApplication::Parameters& params, std::vector<BCOPtr_t>& result)
    {
        // Default parameters
        interpreter::DefaultStatementCompilationContext scc(world);
        scc.withFlag(scc.ExpressionsAreStatements);
        scc.withFlag(scc.LinearExecution);
        scc.withFlag(scc.LocalContext);
        if (params.opt_preexecLoad) {
            scc.withFlag(scc.PreexecuteLoad);
        }

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
            world.logListener().write(LogListener::Trace, LOG_NAME, Format(world.translator()("Compiled %d command%!1{s%}.").c_str(), params.job.size()));
        } else {
            // Files: compile files into individual BCOs
            for (size_t i = 0, n = params.job.size(); i < n; ++i) {
                FileSystem& fs = world.fileSystem();
                String_t ext = util::getFileNameExtension(fs, params.job[i]);
                Ref<afl::io::Stream> stream(fs.openFile(params.job[i], FileSystem::OpenRead));
                if (ext == ".qc") {
                    // Load object file
                    interpreter::vmio::NullLoadContext lc;
                    interpreter::vmio::ObjectLoader loader(*params.gameCharset, world.translator(), lc);
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
                        String_t msg = Format(world.translator()("line %d: %s").c_str(), cs.getLineNumber(), e.what());
                        String_t trace = e.getTrace();
                        if (!trace.empty()) {
                            msg += "\n";
                            msg += trace;
                        }
                        throw afl::except::FileProblemException(tf.getName(), msg);
                    }
                }
            }
            world.logListener().write(LogListener::Trace, LOG_NAME, Format(world.translator()("Compiled %d file%!1{s%}.").c_str(), params.job.size()));
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
     *  Compile Mode
     */
    int doCompileMode(interpreter::World& world, const ConsoleApplication::Parameters& params)
    {
        // Environment
        LogListener& log = world.logListener();
        FileSystem& fs = world.fileSystem();

        // Compile
        std::vector<BCOPtr_t> result;
        doCompile(world, params, result);

        // Produce output
        String_t output;
        if (params.arg_output.get(output)) {
            // Single output file given. If we have multiple BCO's, merge.
            BCORef_t bco = mergeByteCodeObjects(result);
            saveObjectFile(log, fs, output, bco, params, world.translator());
            return 0;
        } else if (params.opt_commands) {
            // No output file given, input is commands
            log.write(LogListener::Error, LOG_NAME, world.translator()("must specify an output file ('-o FILE') if input is commands"));
            return 1;
        } else {
            // No output file given, input is files. Generate output file names.
            for (size_t i = 0; i < result.size() && i < params.job.size(); ++i) {
                saveObjectFile(log, fs, getOutputFileName(fs, params.job[i], ".qc"), *result[i], params, world.translator());
            }
            return 0;
        }
    }

    /*
     *  Disassemble Mode
     */
    int doDisassembleMode(interpreter::World& world, const ConsoleApplication::Parameters& params, afl::io::TextWriter& standardOutput)
    {
        // Compile
        std::vector<BCOPtr_t> result;
        doCompile(world, params, result);

        // Merge everything
        BCORef_t bco = mergeByteCodeObjects(result);

        // Produce output
        String_t output;
        if (params.arg_output.get(output)) {
            // Save to file
            Ref<afl::io::Stream> file = world.fileSystem().openFile(output, FileSystem::Create);
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
    Translator& tx = translator();
    FileSystem& fs = fileSystem();

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

    // Make a World.
    World world(log(), tx, fs);

    // Build load path
    world.setSystemLoadDirectory(util::makeSearchDirectory(fs, params.loadPath).asPtr());

    int result = 0;
    switch (params.mode) {
     case Parameters::CompileMode:
        result = doCompileMode(world, params);
        break;
     case Parameters::DisassembleMode:
        consoleLogger().setConfiguration("*@Warn+=raw:*=drop", translator());
        result = doDisassembleMode(world, params, standardOutput());
        break;
    }
    exit(result);
}

/** Parse parameters.
    @param params [out] Parameters */
void
interpreter::ConsoleApplication::parseParameters(Parameters& params)
{
    Translator& tx = translator();
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "compile" || p == "c") {
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
                               "--compile, -c\tCompile to \"*.qc\" files (default)\n"
                               "--disassemble, -S\tDisassemble to \"*.qs\" files\n"
                               "\n"
                               "Options:\n"
                               "-g\tEnable debug info (default)\n"
                               "-s\tDisable debug info\n"
                               "-o FILE\tOutput file\n"
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
    out.writeLine(Format(tx("PCC2 Script Compiler v%s - (c) 2017-2023 Stefan Reuther").c_str(), PCC2_VERSION));
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
