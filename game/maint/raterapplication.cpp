/**
  *  \file game/maint/raterapplication.cpp
  */

#include "game/maint/raterapplication.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/maint/difficultyrater.hpp"
#include "util/math.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::string::Format;
using game::maint::DifficultyRater;

namespace {
    void showRating(afl::io::TextWriter& out, DifficultyRater& rater, const char* ttl, DifficultyRater::Rating which)
    {
        double result = rater.getRating(which);
        out.writeLine(Format("%-30s %6.2f%s", ttl, 100*result, rater.isRatingKnown(which) ? "" : " (default)"));
    }
}

void
game::maint::RaterApplication::appMain()
{
    DifficultyRater rater;
    enum { Report, OneOnly, TotalOnly } output = Report;
    DifficultyRater::Rating output_one = DifficultyRater::ShiplistRating;

    afl::io::TextWriter& out = standardOutput();
    afl::string::Translator& tx = translator();

    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "h" || p == "help") {
                help(out);
                exit(0);
            } else if (p == "D") {
                // direct assignment
                String_t arg = commandLine.getRequiredParameter(p);
                String_t::size_type i = arg.find('=');
                if (i == String_t::npos) {
                    errorExit(tx("\"-D\" option must have format \"-DNAME=VALUE\""));
                }
                rater.addConfigurationValue(arg.substr(0, i), arg.substr(i+1));
            } else if (p == "total") {
                // report only total value
                output = TotalOnly;
            } else if (p == "value") {
                // report only one sub-rating
                output = OneOnly;

                // FIXME: original accepts abbreviations
                String_t arg = commandLine.getRequiredParameter(p);
                if (arg == "shiplist") {
                    output_one = DifficultyRater::ShiplistRating;
                } else if (arg == "minerals") {
                    output_one = DifficultyRater::MineralRating;
                } else if (arg == "natives") {
                    output_one = DifficultyRater::NativeRating;
                } else if (arg == "production") {
                    output_one = DifficultyRater::ProductionRating;
                } else {
                    errorExit(tx("invalid argument to \"-value=\""));
                }
            } else if (p == "file") {
                // read a config file
                String_t arg = commandLine.getRequiredParameter(p);
                rater.addConfigurationFile(*fileSystem().openFile(arg, afl::io::FileSystem::OpenRead));
            } else {
                errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
            }
        } else {
            // read a directory
            afl::base::Ref<afl::io::Directory> dir = fileSystem().openDirectory(p);
            rater.addConfigurationDirectory(*dir);
            rater.addShipList(*dir);
        }
    }

    // Result
    switch (output) {
     case Report:
        showRating(out, rater, "Ship list rating",  DifficultyRater::ShiplistRating);
        showRating(out, rater, "Mineral rating",    DifficultyRater::MineralRating);
        showRating(out, rater, "Native rating",     DifficultyRater::NativeRating);
        showRating(out, rater, "Production rating", DifficultyRater::ProductionRating);
        out.writeLine(Format("%-30s %6.2f", ">> Total rating", 100*rater.getTotalRating()));
        break;
     case OneOnly:
        out.writeLine(Format("%d", util::roundToInt(100*rater.getRating(output_one))));
        break;
     case TotalOnly:
        out.writeLine(Format("%d", util::roundToInt(100*rater.getTotalRating())));
        break;
    }

}

void
game::maint::RaterApplication::help(afl::io::TextWriter& out)
{
    afl::string::Translator& tx = translator();
    out.writeLine(Format(tx("PCC2 Game Difficulty Rater v%s - (c) 2012-2019 Stefan Reuther"), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-options...] [gamedir]\n\n"
                            "Options:\n"
                            "%s"
                            "\n"
                            "Default is a report with all sub-ratings and totals.\n"
                            "A game directory is processed as a whole (config files and shiplist).\n"
                            "Report bugs to <Streu@gmx.de>"),
                         environment().getInvocationName(),
                         util::formatOptions(tx("-DSECTION.OPTION=VALUE\tprocess option\n"
                                                "--file=FILE\tprocess config file fragment for options\n"
                                                "--total\tonly show total value as integer\n"
                                                "--value=WHAT\tonly show one sub-rating as integer,\n"
                                                "\tWHAT is shiplist, minerals, natives, production\n"))));
}
