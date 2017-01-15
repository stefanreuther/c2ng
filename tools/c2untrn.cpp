/**
  *  \file tools/c2untrn.cpp
  *  \brief c2untrn Utility - Turn File Dumper and Manipulator
  */

#include "afl/base/optional.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/defaultcharsetfactory.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/v3/trn/dumper.hpp"
#include "game/v3/trn/filter.hpp"
#include "game/v3/trn/orfilter.hpp"
#include "game/v3/trn/parseexception.hpp"
#include "game/v3/turnfile.hpp"
#include "util/application.hpp"
#include "util/stringparser.hpp"
#include "util/translation.hpp"
#include "version.hpp"

namespace {
    class ConsoleUntrnApplication : public util::Application {
     public:
        ConsoleUntrnApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        virtual void appMain();

     private:
        void showHelp();
        void showVersion();

        void processEdit(game::v3::TurnFile& trn, String_t edit);
        void saveTurn(const game::v3::TurnFile& trn, String_t fileName);
    };
}

void
ConsoleUntrnApplication::appMain()
{
    enum { List, Delete, Rewrite, Nothing } opt_action = List;
    afl::base::Optional<String_t> opt_filename;
    const game::v3::trn::Filter* opt_filter = 0;
    bool opt_kill_taccom = false;
    afl::base::Optional<bool> opt_header;
    afl::base::Optional<bool> opt_trailer;
    bool opt_sort = false;
    bool opt_zap = false;
    bool opt_comments = true;
    afl::base::Deleter deleter;
    std::vector<String_t> edits;
    afl::charset::Charset* charset = 0;

    afl::sys::StandardCommandLineParser cmdl(environment().getCommandLine());
    bool option;
    String_t text;
    while (cmdl.getNext(option, text)) {
        if (option) {
            if (text == "h" || text == "help") {
                showHelp();
            } else if (text == "v" || text == "version") {
                showVersion();
            } else if (text == "p") {
                opt_header = true;
            } else if (text == "P") {
                opt_header = false;
            } else if (text == "t") {
                opt_trailer = true;
            } else if (text == "T") {
                opt_trailer = false;
            } else if (text == "s") {
                opt_sort = true;
            } else if (text == "a") {
                opt_kill_taccom = true;
            } else if (text == "c") {
                opt_comments = false;
            } else if (text == "C") {
                String_t charsetName;
                if (!cmdl.getParameter(charsetName)) {
                    errorExit(_("option \"-C\" needs an argument (the character set)"));
                }

                charset = afl::charset::DefaultCharsetFactory().createCharset(deleter, charsetName);
                if (!charset) {
                    errorExit(_("the specified character set is not known"));
                }
            } else if (text == "f") {
                String_t filterExpr;
                if (!cmdl.getParameter(filterExpr)) {
                    errorExit(_("option \"-f\" needs an argument (the filter expression)"));
                }
                try {
                    const game::v3::trn::Filter& f = game::v3::trn::Filter::parse(filterExpr, deleter);
                    if (opt_filter) {
                        opt_filter = &deleter.addNew(new game::v3::trn::OrFilter(*opt_filter, f));
                    } else {
                        opt_filter = &f;
                    }
                }
                catch (game::v3::trn::ParseException& ex) {
                    errorExit(_("syntax error in filter expression: ") + String_t(ex.what()));
                }
            } else if (text == "e") {
                String_t editCommand;
                if (!cmdl.getParameter(editCommand)) {
                    errorExit(_("option \"-e\" needs an argument (the edit command)"));
                }
                edits.push_back(editCommand);
            } else if (text == "d") {
                opt_action = Delete;
            } else if (text == "n") {
                opt_action = Nothing;
            } else if (text == "r") {
                if (opt_action != Delete) {
                    opt_action = Rewrite;
                }
            } else if (text == "z") {
                opt_zap = true;
            } else {
                errorExit(afl::string::Format(_("invalid option specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
            }
        } else if (!opt_filename.isValid()) {
            opt_filename = text;
        } else {
            errorExit(_("too many arguments"));
        }
    }

    String_t fileName;
    if (!opt_filename.get(fileName)) {
        errorExit(afl::string::Format(_("no file name specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
    }
    if (opt_action == Delete && !opt_filter) {
        errorExit(_("no filter specified. Deleting (\"-d\") needs a filter"));
    }
    if (!edits.empty() && opt_action == List) {
        opt_action = Rewrite;
    }
    if (!charset) {
        charset = &deleter.addNew(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
    }

    const bool showHeader = opt_header.orElse(!opt_filter);
    const bool showTrailer = opt_trailer.orElse(!opt_filter);

    int exitCode = opt_filter ? 2 : 0;

    afl::base::Ptr<afl::io::Stream> file = fileSystem().openFile(fileName, afl::io::FileSystem::OpenRead).asPtr();
    game::v3::TurnFile trn(*charset, *file, !opt_zap);
    file.reset();

    if (opt_sort) {
        trn.sortCommands();
    }
    if (opt_kill_taccom) {
        trn.setFeatures(trn.getFeatures() - trn.TaccomFeature);
        trn.update();       // needed by computeTurnChecksum()
    }
    for (std::vector<String_t>::iterator i = edits.begin(); i != edits.end(); ++i) {
        processEdit(trn, *i);
    }

    afl::io::TextWriter& console = standardOutput();
    switch (opt_action) {
     case List: {
        game::v3::trn::Dumper d(standardOutput());
        d.setShowComments(opt_comments);
        d.setShowHeader(showHeader);
        d.setShowTrailer(showTrailer);
        d.setFilter(opt_filter);
        d.setVerifyTrailerChecksum(!opt_zap);
        if (showHeader) {
            standardOutput().writeLine(afl::string::Format(";==================================================\n"
                                                           "; Listing of \"%s\"\n"
                                                           ";==================================================\n"
                                                           ";",
                                                           fileName));
        }
        if (d.dump(trn)) {
            exitCode = 0;
        } else {
            if (opt_filter) {
                console.writeLine(_("No command matched the filter."));
            }
        }
        break;
     }
     case Delete: {
        bool match = false;
        for (size_t i = 0, n = trn.getNumCommands(); i < n; ++i) {
            if (opt_filter->accept(trn, i)) {
                trn.deleteCommand(i);
                exitCode = 0;
                match = true;
            }
        }
        if (!match) {
            console.writeLine(_("No command matched the filter."));
        }
        trn.update();
        saveTurn(trn, fileName);
        break;
     }
     case Rewrite:
        trn.update();
        saveTurn(trn, fileName);
        break;
     case Nothing:
        break;
    }
    exit(exitCode);
}

void
ConsoleUntrnApplication::showHelp()
{
    // ex game/un-trn.cc:help
    afl::io::TextWriter& w = standardOutput();
    w.writeLine(afl::string::Format(_("Turn File Decompiler v%s - (c) 2001-2017 Stefan Reuther").c_str(), PCC2_VERSION));
    w.writeText(afl::string::Format(_("\n"
                                      "Usage:\n"
                                      "  %s [-h|-v]\n"
                                      "  %0$s [-pPtTsckr] [-d] [-f expr] playerX.trn\n\n"
                                      "Filter expressions:\n"
                                      "  a&b, a|b, !a, (a), command, com*, id, id-id, true, false, #num, #num-num\n\n"
                                      "About:\n"
                                      " -h        this help summary\n"
                                      " -v        show version number\n"
                                      "\n"
                                      "Options:\n"
                                      " -p/-P     show/don't show turn header\n"
                                      " -t/-T     show/don't show turn trailer\n"
                                      " -f expr   show only matching commands\n"
                                      " -e cmd    edit turn file\n"
                                      " -C cset   use specified character set\n"
                                      " -s        sort turn before doing anything\n"
                                      " -c        do not show most comments\n"
                                      " -a        ignore Taccom attachments (delete them if -r used)\n"
                                      " -z        zap commands before doing anything (just parse headers)\n"
                                      "\n"
                                      "Actions:\n"
                                      " -r        re-write turn\n"
                                      " -d        delete matching commands (ignores -pPtT, implies -r)\n"
                                      " -n        no output (ignores -pPtT)\n"
                                      "\n"
                                      "Report bugs to <Streu@gmx.de>\n").c_str(),
                                    environment().getInvocationName()));
    exit(0);
}

void
ConsoleUntrnApplication::showVersion()
{
    // ex game/un-trn.cc:version
    standardOutput().writeLine(afl::string::Format(_("Turn File Decompiler v%s - (c) 2001-2017 Stefan Reuther").c_str(), PCC2_VERSION));
    exit(0);
}

void
ConsoleUntrnApplication::processEdit(game::v3::TurnFile& trn, String_t edit)
{
    // Find command
    util::StringParser p(edit);
    String_t cmd;
    if (!p.parseDelim("=", cmd) || !p.parseChar('=')) {
        errorExit(afl::string::Format(_("invalid edit command \"%s\"").c_str(), edit));
    }

    // Execute command
    if (afl::string::strCaseCompare(cmd, "version") == 0) {
        int n;
        if (!p.parseInt(n) || !p.parseEnd() || n < 0 || n > 99) {
            errorExit(afl::string::Format(_("invalid version number \"%s\"").c_str(), edit));
        }
        trn.setVersion(n);
    } else if (afl::string::strCaseCompare(cmd, "time") == 0 || afl::string::strCaseCompare(cmd, "timestamp") == 0) {
        // Convert to timestamp
        String_t time = trn.charset().encode(afl::string::toMemory(p.getRemainder()));
        char timeBuffer[game::Timestamp::SIZE];
        if (time.size() != sizeof timeBuffer) {
            errorExit(_("timestamp must be 18 characters in length"));
        }
        afl::string::StringMemory_t(timeBuffer).copyFrom(afl::string::toMemory(time));

        // Set it
        trn.setTimestamp(timeBuffer);
    } else {
        errorExit(afl::string::Format(_("unknown edit command \"%s\"").c_str(), edit));
    }
}

void
ConsoleUntrnApplication::saveTurn(const game::v3::TurnFile& trn, String_t fileName)
{
    // ex game/un-trn.cc:saveTurn
    afl::base::Ref<afl::io::Stream> file = fileSystem().openFile(fileName, afl::io::FileSystem::Create);
    trn.write(*file);
    standardOutput().writeLine(afl::string::Format(_("Wrote turn file with %d command%!1{s%}").c_str(), trn.getNumCommands()));
}

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return ConsoleUntrnApplication(env, fs).run();
}
