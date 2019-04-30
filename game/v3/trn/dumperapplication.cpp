/**
  *  \file game/v3/trn/dumperapplication.cpp
  */

#include "game/v3/trn/dumperapplication.hpp"
#include "afl/base/deleter.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/v3/trn/dumper.hpp"
#include "game/v3/trn/filter.hpp"
#include "game/v3/trn/orfilter.hpp"
#include "game/v3/trn/parseexception.hpp"
#include "util/charsetfactory.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"
#include "version.hpp"

using afl::base::Optional;
using afl::string::Format;

void
game::v3::trn::DumperApplication::appMain()
{
    enum { List, Delete, Rewrite, Nothing } opt_action = List;
    Optional<String_t> opt_filename;
    const Filter* opt_filter = 0;
    bool opt_kill_taccom = false;
    Optional<bool> opt_header;
    Optional<bool> opt_trailer;
    bool opt_sort = false;
    bool opt_zap = false;
    bool opt_comments = true;
    afl::base::Deleter deleter;
    std::vector<String_t> edits;
    std::auto_ptr<afl::charset::Charset> charset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));

    afl::string::Translator& tx = translator();

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
                const String_t charsetName = cmdl.getRequiredParameter(text);
                if (afl::charset::Charset* cs = util::CharsetFactory().createCharset(charsetName)) {
                    charset.reset(cs);
                } else {
                    errorExit(tx("the specified character set is not known"));
                }
            } else if (text == "f") {
                const String_t filterExpr = cmdl.getRequiredParameter(text);
                try {
                    const Filter& f = Filter::parse(filterExpr, deleter);
                    if (opt_filter) {
                        opt_filter = &deleter.addNew(new OrFilter(*opt_filter, f));
                    } else {
                        opt_filter = &f;
                    }
                }
                catch (ParseException& ex) {
                    errorExit(tx("syntax error in filter expression: ") + String_t(ex.what()));
                }
            } else if (text == "e") {
                edits.push_back(cmdl.getRequiredParameter(text));
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
                errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
            }
        } else if (!opt_filename.isValid()) {
            opt_filename = text;
        } else {
            errorExit(tx("too many arguments"));
        }
    }

    String_t fileName;
    if (!opt_filename.get(fileName)) {
        errorExit(Format(tx("no file name specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
    }
    if (opt_action == Delete && !opt_filter) {
        errorExit(tx("no filter specified. Deleting (\"-d\") needs a filter"));
    }
    if (!edits.empty() && opt_action == List) {
        opt_action = Rewrite;
    }

    const bool showHeader = opt_header.orElse(!opt_filter);
    const bool showTrailer = opt_trailer.orElse(!opt_filter);

    int exitCode = opt_filter ? 2 : 0;

    afl::base::Ptr<afl::io::Stream> file = fileSystem().openFile(fileName, afl::io::FileSystem::OpenRead).asPtr();
    TurnFile trn(*charset, *file, !opt_zap);
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
        Dumper d(standardOutput());
        d.setShowComments(opt_comments);
        d.setShowHeader(showHeader);
        d.setShowTrailer(showTrailer);
        d.setFilter(opt_filter);
        d.setVerifyTrailerChecksum(!opt_zap);
        if (showHeader) {
            console.writeLine(Format(";==================================================\n"
                                     "; Listing of \"%s\"\n"
                                     ";==================================================\n"
                                     ";",
                                     fileName));
        }
        if (d.dump(trn)) {
            exitCode = 0;
        } else {
            if (opt_filter) {
                console.writeLine(tx("No command matched the filter."));
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
            console.writeLine(tx("No command matched the filter."));
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
game::v3::trn::DumperApplication::showHelp()
{
    // ex game/un-trn.cc:help
    afl::io::TextWriter& w = standardOutput();
    afl::string::Translator& tx = translator();
    w.writeLine(Format(tx("Turn File Decompiler v%s - (c) 2001-2019 Stefan Reuther").c_str(), PCC2_VERSION));
    w.writeText(Format(tx("\n"
                          "Usage:\n"
                          "  %s [-h|-v]\n"
                          "  %0$s [-pPtTsckr] [-d] [-f expr] playerX.trn\n\n"
                          "Filter expressions:\n"
                          "  a&b, a|b, !a, (a), command, com*, id, id-id, true, false, #num, #num-num\n\n"
                          "%s"
                          "\n"
                          "Report bugs to <Streu@gmx.de>\n").c_str(),
                       environment().getInvocationName(),
                       util::formatOptions(tx("About:\n"
                                              "-h\tthis help summary\n"
                                              "-v\tshow version number\n"
                                              "\n"
                                              "Options:\n"
                                              "-p/-P\tshow/don't show turn header\n"
                                              "-t/-T\tshow/don't show turn trailer\n"
                                              "-f expr\tshow only matching commands\n"
                                              "-e cmd\tedit turn file\n"
                                              "-C cset\tuse specified character set\n"
                                              "-s\tsort turn before doing anything\n"
                                              "-c\tdo not show most comments\n"
                                              "-a\tignore Taccom attachments (delete them if -r used)\n"
                                              "-z\tzap commands before doing anything (just parse headers)\n"
                                              "\n"
                                              "Actions:\n"
                                              "-r\tre-write turn\n"
                                              "-d\tdelete matching commands (ignores -pPtT, implies -r)\n"
                                              "-n\tno output (ignores -pPtT)\n"))));
    exit(0);
}

void
game::v3::trn::DumperApplication::showVersion()
{
    // ex game/un-trn.cc:version
    standardOutput().writeLine(Format(translator()("Turn File Decompiler v%s - (c) 2001-2019 Stefan Reuther").c_str(), PCC2_VERSION));
    exit(0);
}

void
game::v3::trn::DumperApplication::processEdit(game::v3::TurnFile& trn, String_t edit)
{
    // Find command
    afl::string::Translator& tx = translator();
    util::StringParser p(edit);
    String_t cmd;
    if (!p.parseDelim("=", cmd) || !p.parseCharacter('=')) {
        errorExit(Format(tx("invalid edit command \"%s\""), edit));
    }

    // Execute command
    if (afl::string::strCaseCompare(cmd, "version") == 0) {
        int n;
        if (!p.parseInt(n) || !p.parseEnd() || n < 0 || n > 99) {
            errorExit(Format(tx("invalid version number \"%s\""), edit));
        }
        trn.setVersion(n);
    } else if (afl::string::strCaseCompare(cmd, "time") == 0 || afl::string::strCaseCompare(cmd, "timestamp") == 0) {
        // Convert to timestamp
        afl::base::GrowableBytes_t time = trn.charset().encode(afl::string::toMemory(p.getRemainder()));
        uint8_t timeBuffer[game::Timestamp::SIZE];
        if (time.size() != sizeof timeBuffer) {
            errorExit(tx("timestamp must be 18 characters in length"));
        }
        afl::base::Bytes_t(timeBuffer).copyFrom(time);

        // Set it
        trn.setTimestamp(timeBuffer);
    } else {
        errorExit(Format(tx("unknown edit command \"%s\""), edit));
    }
}

void
game::v3::trn::DumperApplication::saveTurn(const game::v3::TurnFile& trn, String_t fileName)
{
    // ex game/un-trn.cc:saveTurn
    afl::base::Ref<afl::io::Stream> file = fileSystem().openFile(fileName, afl::io::FileSystem::Create);
    trn.write(*file);
    standardOutput().writeLine(Format(translator()("Wrote turn file with %d command%!1{s%}"), trn.getNumCommands()));
}
