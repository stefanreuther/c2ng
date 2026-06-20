/**
  *  \file game/maint/configurationapplication.cpp
  *  \brief Class game::maint::ConfigurationApplication
  */

#include <memory>
#include "game/maint/configurationapplication.hpp"
#include "afl/base/countof.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/config/configuration.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/v3/hconfig.hpp"
#include "game/v3/structures.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"
#include "version.hpp"

using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::io::TextFile;
using afl::string::Format;
using afl::sys::LogListener;
using game::config::Configuration;
using game::config::ConfigurationOption;
using game::config::HostConfiguration;
using game::v3::structures::NUM_HULLS_PER_PLAYER;
using game::v3::structures::NUM_PLAYERS;
using util::ConfigurationFile;

namespace {
    /*
     *  shuffle() ignorelist processing
     */

    const char*const IGNORELIST[] = {
        // Ignore pcontrol; command lines might contain commas
        "PCONTROL.",

        // Experience options (indexed by experience level, not race)
        ".EMODBAYRECHARGEBONUS",
        ".EMODBAYRECHARGERATE",
        ".EMODBEAMHITBONUS",
        ".EMODBEAMHITFIGHTERCHARGE",
        ".EMODBEAMHITODDS",
        ".EMODBEAMRECHARGEBONUS",
        ".EMODBEAMRECHARGERATE",
        ".EMODCREWKILLSCALING",
        ".EMODENGINESHIELDBONUSRATE",
        ".EMODEXTRAFIGHTERBAYS",
        ".EMODFIGHTERBEAMEXPLOSIVE",
        ".EMODFIGHTERBEAMKILL",
        ".EMODFIGHTERMOVEMENTSPEED",
        ".EMODHULLDAMAGESCALING",
        ".EMODMAXFIGHTERSLAUNCHED",
        ".EMODMINEHITODDSBONUS",
        ".EMODPLANETARYTORPSPERTUBE",
        ".EMODSHIELDDAMAGESCALING",
        ".EMODSHIELDKILLSCALING",
        ".EMODSTRIKESPERFIGHTER",
        ".EMODTORPHITBONUS",
        ".EMODTORPHITODDS",
        ".EMODTUBERECHARGEBONUS",
        ".EMODTUBERECHARGERATE",
        ".EPCOMBATBOOSTLEVEL",
        ".EPCOMBATBOOSTRATE",
        ".EXPERIENCELEVELNAMES",
        ".EXPERIENCELEVELS",

        // Game name may contain commas
        ".GAMENAME",

        // Language (includes host language)
        ".LANGUAGE",

        // Ranges, not indexed by race
        ".LARGEMETEORORERANGES",
        ".METEORSHOWERORERANGES",
        ".NATIVECLANSRANGE",
        ".NATIVEGOVFREQUENCIES",
        ".NATIVETYPEFREQUENCIES",
        ".NEWNATIVESGOVERNMENTRATE",
        ".NEWNATIVESPOPULATIONRANGE",
        ".NEWNATIVESRACERATE",
        ".WRAPAROUNDRECTANGLE",
    };

    /*
     *  ConfigurationReference
     */

    class ConfigurationReference {
     public:
        ConfigurationReference()
            : m_p()
            { }

        ConfigurationFile& operator()()
            {
                if (m_p.get() == 0) {
                    m_p.reset(new ConfigurationFile());
                }
                return *m_p;
            }

        void replaceOrMerge(std::auto_ptr<ConfigurationFile> other)
            {
                if (m_p.get() == 0) {
                    m_p = other;
                } else {
                    m_p->merge(*other);
                }
            }

        ConfigurationFile* get()
            { return m_p.get(); }

     private:
        std::auto_ptr<ConfigurationFile> m_p;
    };

    /*
     *  Utilities
     */

    String_t limit11(String_t in)
    {
        size_t i = 0;
        int n = 0;
        while (i < in.size()) {
            if (in[i] == ',') {
                if (++n == game::v3::structures::NUM_PLAYERS) {
                    in.erase(i);
                    break;
                }
            }
            ++i;
        }
        return in;
    }

    std::vector<int> parsePermutation(const String_t& perm, afl::string::Translator& tx)
        {
            std::vector<int> result;
            util::StringParser p(perm);
            String_t tmp;
            int tmpInt;
            while (1) {
                p.parseWhile(afl::string::charIsSpace, tmp);
                if (!p.parseInt(tmpInt)) {
                    throw afl::except::CommandLineException(Format(tx("expecting number, found \"%s\""),
                                                                   p.getRemainder().substr(0, 20)));
                }
                result.push_back(tmpInt);
                p.parseWhile(afl::string::charIsSpace, tmp);
                if (p.parseEnd()) {
                    break;
                }
                if (!p.parseCharacter(',')) {
                    throw afl::except::CommandLineException(Format(tx("expecting \",\", found \"%s\""),
                                                                   p.getRemainder().substr()));
                }
            }
            return result;
        }

    bool endsWith(const String_t& str, const char* end)
    {
        size_t n = std::strlen(end);
        return str.size() >= n
            && afl::string::strCaseCompare(str.substr(str.size()-n), end) == 0;
    }

    bool startsWith(const String_t& str, const char* beg)
    {
        size_t n = std::strlen(beg);
        return str.size() >= n
            && afl::string::strCaseCompare(str.substr(0, n), beg) == 0;
    }
}

game::maint::ConfigurationApplication::ConfigurationApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : Application(env, fs)
{ }

void
game::maint::ConfigurationApplication::appMain()
{
    afl::string::Translator& tx = translator();
    ConfigurationReference subject;
    afl::sys::StandardCommandLineParser cmdl(environment().getCommandLine());
    bool hadAction = false;
    bool whitespaceIsSignificant = false;
    bool option;
    String_t text;
    while (cmdl.getNext(option, text)) {
        if (option) {
            if (text == "help" || text == "h") {
                showHelp();
            } else if (text == "empty") {
                // --empty
                subject();
            } else if (text == "load-hconfig") {
                // --load-hconfig=FILE
                String_t fileName = cmdl.getRequiredParameter(text);
                Ref<Stream> thisStream(fileSystem().openFile(fileName, FileSystem::OpenRead));
                loadHConfig(subject(), *thisStream);
            } else if (text == "load-truehull") {
                // --load-truehull=FILE
                String_t fileName = cmdl.getRequiredParameter(text);
                Ref<Stream> thisStream(fileSystem().openFile(fileName, FileSystem::OpenRead));
                loadTruehull(subject(), *thisStream);
            } else if (text == "D") {
                // -D KEY=VALUE
                String_t kv = cmdl.getRequiredParameter(text);
                String_t::size_type eq = kv.find('=');
                if (eq == String_t::npos) {
                    errorExit(tx("expecting \"KEY=VALUE\" for option \"-D\""));
                }
                subject().set(kv.substr(0, eq), kv.substr(eq+1));
            } else if (text == "A") {
                // -A KEY=VALUE
                String_t kv = cmdl.getRequiredParameter(text);
                String_t::size_type eq = kv.find('=');
                if (eq == String_t::npos) {
                    errorExit(tx("expecting \"KEY=VALUE\" for option \"-A\""));
                }
                subject().add(kv.substr(0, eq), kv.substr(eq+1));
            } else if (text == "U") {
                // -U KEY
                String_t key = cmdl.getRequiredParameter(text);
                while (subject().remove(key)) {
                    // nix
                }
            } else if (text == "o") {
                // -o FILE
                String_t fileName = cmdl.getRequiredParameter(text);
                Ref<Stream> thisStream(fileSystem().openFile(fileName, FileSystem::Create));
                TextFile thisText(*thisStream);
                subject().save(thisText);
                thisText.flush();
                hadAction = true;
            } else if (text == "stdout") {
                // --stdout
                subject().save(standardOutput());
                hadAction = true;
            } else if (text == "get") {
                // --get=KEY
                String_t key = afl::string::strUCase(cmdl.getRequiredParameter(text));
                if (const ConfigurationFile::Element* ele = subject().findElement(ConfigurationFile::Assignment, key)) {
                    standardOutput().writeLine(ele->value);
                } else {
                    standardOutput().writeLine(String_t());
                }
                hadAction = true;
            } else if (text == "save-hconfig") {
                // --save-hconfig=FILE
                String_t fileName = cmdl.getRequiredParameter(text);
                Ref<Stream> thisStream(fileSystem().openFile(fileName, FileSystem::Create));
                saveHConfig(subject(), *thisStream);
                hadAction = true;
            } else if (text == "save-truehull") {
                // --save-truehull=FILE
                String_t fileName = cmdl.getRequiredParameter(text);
                Ref<Stream> thisStream(fileSystem().openFile(fileName, FileSystem::Create));
                saveTruehull(subject(), *thisStream);
                hadAction = true;
            } else if (text == "shuffle") {
                // --shuffle=A,B,C,...
                shuffle(subject(), cmdl.getRequiredParameter(text));
            } else if (text == "w") {
                // -w
                whitespaceIsSignificant = true;
                if (ConfigurationFile* p = subject.get()) {
                    p->setWhitespaceIsSignificant(whitespaceIsSignificant);
                }
            } else {
                errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
            }
        } else {
            // Just a file name: load it
            Ref<Stream> thisStream(fileSystem().openFile(text, FileSystem::OpenRead));
            TextFile thisText(*thisStream);
            std::auto_ptr<ConfigurationFile> thisConfig(new ConfigurationFile());
            thisConfig->setWhitespaceIsSignificant(whitespaceIsSignificant);
            thisConfig->load(thisText);
            subject.replaceOrMerge(thisConfig);
        }
    }

    if (!hadAction) {
        errorOutput().writeLine(translator().translateString("warning: no action specified"));
    }
}

void
game::maint::ConfigurationApplication::showHelp()
{
    afl::string::Translator& tx = translator();
    afl::io::TextWriter& w = standardOutput();
    w.writeLine(Format(tx("Configuration Tool v%s - (c) 2018-2025 Stefan Reuther").c_str(), PCC2_VERSION));
    w.writeText(Format(tx("\n"
                          "Usage:\n"
                          "  %s [-OPTIONS|FILES...]\n"
                          "\n"
                          "%s"
                          "\n"
                          "Report bugs to <Streu@gmx.de>\n").c_str(),
                       environment().getInvocationName(),
                       util::formatOptions(tx("General:\n"
                                              "--help\tshow help\n"
                                              "-w\twhitespace is significant in values\n"
                                              "\n"
                                              "Load/Modify:\n"
                                              "FILE\tload text file\n"
                                              "--empty\tload empty file\n"
                                              "--load-hconfig=FILE\tload binary HConfig file\n"
                                              "--load-truehull=FILE\tload truehull file\n"
                                              "-DKEY=VALUE\tset value\n"
                                              "-AKEY=VALUE\tadd value\n"
                                              "-UKEY\tunset value\n"
                                              "--shuffle=A,B,C\tshuffle player-specific settings\n"
                                              "\n"
                                              "Actions:\n"
                                              "-o FILE\tsave result to file\n"
                                              "--stdout\tsend result to stdout\n"
                                              "--get=OPTION\tget option value\n"
                                              "--save-hconfig=FILE\tsave binary HConfig file\n"
                                              "--save-truehull=FILE\tsave truehull file\n"))));
    exit(0);
}

void
game::maint::ConfigurationApplication::loadHConfig(util::ConfigurationFile& out, afl::io::Stream& in)
{
    // Load file
    game::v3::structures::HConfig data;
    size_t size = in.read(afl::base::fromObject(data));

    // Convert to internal format
    Ref<HostConfiguration> config = HostConfiguration::create();
    game::v3::unpackHConfig(data, size, *config, ConfigurationOption::User);

    // Convert that into result
    // Internal options have 31 (NUM_PLAYERS) slots; we want pconfig to be limited to 11.
    Ref<Configuration::Enumerator_t> e(config->getOptions());
    Configuration::OptionInfo_t oi;
    while (e->getNextElement(oi)) {
        if (oi.second != 0 && oi.second->getSource() == ConfigurationOption::User) {
            out.set("PHOST", oi.first, limit11(oi.second->toString()));
        }
    }
}

void
game::maint::ConfigurationApplication::saveHConfig(const util::ConfigurationFile& in, afl::io::Stream& out)
{
    // Convert to internal format
    Ref<HostConfiguration> config = HostConfiguration::create();
    for (size_t i = 0, n = in.getNumElements(); i < n; ++i) {
        if (const ConfigurationFile::Element* pElem = in.getElementByIndex(i)) {
            if (pElem->key.compare(0, 6, "PHOST.", 6) == 0) {
                try {
                    config->setOption(pElem->key.substr(6), pElem->value, ConfigurationOption::User);
                }
                catch (std::exception& e) {
                    log().write(LogListener::Warn, "config", pElem->key, e);
                }
            }
        }
    }

    // Convert to hconfig format
    game::v3::structures::HConfig data;
    game::v3::packHConfig(data, *config);

    // Write file
    out.fullWrite(afl::base::fromObject(data));
}

void
game::maint::ConfigurationApplication::loadTruehull(util::ConfigurationFile& out, afl::io::Stream& in)
{
    // Load file
    game::v3::structures::Truehull data;
    in.fullRead(afl::base::fromObject(data));

    // Convert into options
    for (int slot = 0; slot < NUM_HULLS_PER_PLAYER; ++slot) {
        String_t value = Format("%d", int(data.hulls[0][slot]));
        for (int player = 1; player < NUM_PLAYERS; ++player) {
            value += Format(",%d", int(data.hulls[player][slot]));
        }
        out.set("TRUEHULL", Format("Slot%d", slot+1), value);
    }
}

void
game::maint::ConfigurationApplication::saveTruehull(const util::ConfigurationFile& in, afl::io::Stream& out)
{
    // Load file
    game::v3::structures::Truehull data;

    // Read each option
    for (int slot = 0; slot < NUM_HULLS_PER_PLAYER; ++slot) {
        int32_t row[NUM_PLAYERS];
        afl::base::Memory<int32_t>(row).fill(0);

        if (const ConfigurationFile::Element* ele = in.findElement(ConfigurationFile::Assignment, Format("TRUEHULL.Slot%d", slot+1))) {
            game::config::IntegerValueParser::instance.parseArray(ele->value, row);
        }

        for (int player = 0; player < NUM_PLAYERS; ++player) {
            data.hulls[player][slot] = static_cast<int16_t>(row[player]);
        }
    }

    out.fullWrite(afl::base::fromObject(data));
}

void
game::maint::ConfigurationApplication::shuffle(util::ConfigurationFile& config, const String_t& perm)
{
    std::vector<int> parsedPerm = parsePermutation(perm, translator());
    class AcceptorImpl : public util::ConfigurationFile::Acceptor {
     public:
        virtual bool accept(const String_t& key)
            {
                for (size_t i = 0; i < countof(IGNORELIST); ++i) {
                    if (IGNORELIST[i][0] == '.') {
                        if (endsWith(key, IGNORELIST[i])) {
                            return false;
                        }
                    } else {
                        if (startsWith(key, IGNORELIST[i])) {
                            return false;
                        }
                    }
                }
                return true;
            }
    };
    AcceptorImpl a;
    config.shuffle(a, parsedPerm);
}
