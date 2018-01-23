/**
  *  \file tools/c2rater.cpp
  *  \brief "Rater" utility - Main Function
  */

#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/maint/difficultyrater.hpp"
#include "util/application.hpp"
#include "util/math.hpp"
#include "util/translation.hpp"
#include "version.hpp"

using game::maint::DifficultyRater;

namespace {
    class ConsoleRaterApplication : public util::Application {
     public:
        ConsoleRaterApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        void appMain();

     private:
        void help(afl::io::TextWriter& out);
    };

    void showRating(afl::io::TextWriter& out, DifficultyRater& rater, const char* ttl, DifficultyRater::Rating which)
    {
        double result = rater.getRating(which);
        out.writeLine(afl::string::Format("%-30s %6.2f%s", ttl, 100*result, rater.isRatingKnown(which) ? "" : " (default)"));
    }
}

void
ConsoleRaterApplication::appMain()
{
    DifficultyRater rater;
    enum { Report, OneOnly, TotalOnly } output = Report;
    DifficultyRater::Rating output_one = DifficultyRater::ShiplistRating;

    afl::io::TextWriter& out = standardOutput();

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
                    errorExit(_("\"-D\" option must have format \"-DNAME=VALUE\""));
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
                    errorExit(_("invalid argument to \"-value=\""));
                }
            } else if (p == "file") {
                // read a config file
                String_t arg = commandLine.getRequiredParameter(p);
                rater.addConfigurationFile(*fileSystem().openFile(arg, afl::io::FileSystem::OpenRead));
            } else {
                errorExit(afl::string::Format(_("invalid option specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
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
        out.writeLine(afl::string::Format("%-30s %6.2f", ">> Total rating", 100*rater.getTotalRating()));
        break;
     case OneOnly:
        out.writeLine(afl::string::Format("%d", util::roundToInt(100*rater.getRating(output_one))));
        break;
     case TotalOnly:
        out.writeLine(afl::string::Format("%d", util::roundToInt(100*rater.getTotalRating())));
        break;
    }

}

void
ConsoleRaterApplication::help(afl::io::TextWriter& out)
{
    out.writeLine(afl::string::Format(_("PCC2 Game Difficulty Rater v%s - (c) 2012-2018 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(afl::string::Format(_("Usage:\n"
                                        "  %s [-options...] [gamedir]\n\n"
                                        "Options:\n"
                                        " -DSECTION.OPTION=VALUE    process option\n"
                                        " --file=FILE               process config file fragment for options\n"
                                        " --total                   only show total value as integer\n"
                                        " --value=WHAT              only show one sub-rating as integer,\n"
                                        "                           WHAT is shiplist, minerals, natives, production\n"
                                        "\n"
                                        "Default is a report with all sub-ratings and totals.\n"
                                        "A game directory is processed as a whole (config files and shiplist).\n"
                                        "Report bugs to <Streu@gmx.de>").c_str(), environment().getInvocationName()));
}

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return ConsoleRaterApplication(env, fs).run();
}
