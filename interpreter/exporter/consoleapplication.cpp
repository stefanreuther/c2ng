/**
  *  \file interpreter/exporter/consoleapplication.cpp
  */

#include "interpreter/exporter/consoleapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/limits.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/specificationloader.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "game/v3/rootloader.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/exporter/configuration.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplecontext.hpp"
#include "interpreter/values.hpp"
#include "util/charsetfactory.hpp"
#include "util/profiledirectory.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::base::Optional;

namespace {
    const char LOG_NAME[] = "export";

    /* Meta-context for generating field names. Used to implement '-F'. */
    // FIXME: move into library
    class MetaContext : public interpreter::SimpleContext,
                        public interpreter::Context::ReadOnlyAccessor,
                        public interpreter::PropertyAcceptor
    {
     public:
        MetaContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual MetaContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        // PropertyAcceptor:
        virtual void addProperty(const String_t& name, interpreter::TypeHint th);

     private:
        std::vector<String_t> m_names;
        std::vector<uint8_t> m_types;
        std::size_t m_position;
    };


    static const interpreter::NameTable meta_mapping[] = {
        { "ID",   0, 0, interpreter::thInt },
        { "NAME", 1, 0, interpreter::thString },
        { "TYPE", 2, 0, interpreter::thString },
    };

    MetaContext::MetaContext()
        : m_names(),
          m_types(),
          m_position(0)
    { }

    interpreter::Context::PropertyAccessor* MetaContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
    {
        return lookupName(name, meta_mapping, result) ? this : 0;
    }

    afl::data::Value* MetaContext::get(PropertyIndex_t index)
    {
        if (m_position < m_names.size()) {
            switch (meta_mapping[index].index) {
             case 0:
                return interpreter::makeSizeValue(m_position);
             case 1:
                return interpreter::makeStringValue(m_names[m_position]);
             case 2:
                switch (m_types[m_position]) {
                 case interpreter::thNone:
                    return interpreter::makeStringValue("any");
                 case interpreter::thBool:
                    return interpreter::makeStringValue("bool");
                 case interpreter::thInt:
                    return interpreter::makeStringValue("int");
                 case interpreter::thFloat:
                    return interpreter::makeStringValue("float");
                 case interpreter::thString:
                    return interpreter::makeStringValue("string");
                 case interpreter::thProcedure:
                    return interpreter::makeStringValue("procedure");
                 case interpreter::thFunction:
                    return interpreter::makeStringValue("function");
                 case interpreter::thArray:
                    return interpreter::makeStringValue("array");
                }
                break;
            }
        }
        return 0;
    }

    bool MetaContext::next()
    {
        if (m_position+1 < m_names.size()) {
            ++m_position;
            return true;
        } else {
            return false;
        }
    }

    MetaContext* MetaContext::clone() const
    {
        // This object is never cloned in c2export. When we make MetaContext a proper
        // type for use by scripts, we'd have to implement this.
        throw interpreter::Error("not clonable");
    }

    game::map::Object* MetaContext::getObject()
    {
        return 0;
    }

    void MetaContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
    {
        acceptor.enumTable(meta_mapping);
    }

    String_t MetaContext::toString(bool /*readable*/) const
    {
        return "#<meta>";
    }

    void MetaContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
    {
        rejectStore(out, aux, ctx);
    }

    void MetaContext::addProperty(const String_t& name, interpreter::TypeHint th)
    {
        m_names.push_back(name);
        m_types.push_back(th);
    }
}

void
interpreter::exporter::ConsoleApplication::appMain()
{
    util::ProfileDirectory profile(environment(), fileSystem(), translator(), log());
    afl::string::Translator& tx = translator();

    // Parse args
    interpreter::exporter::Configuration config;

    Optional<String_t> arg_array;
    Optional<String_t> arg_gamedir;
    Optional<String_t> arg_rootdir;
    Optional<String_t> arg_outfile;
    int arg_race = 0;
    bool opt_fields = false;
    std::auto_ptr<afl::charset::Charset> gameCharset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
    bool hadCharsetOption = false;

    int i;
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "C") {
                if (afl::charset::Charset* cs = util::CharsetFactory().createCharset(commandLine.getRequiredParameter(p))) {
                    gameCharset.reset(cs);
                } else {
                    errorExit(tx("the specified character set is not known"));
                }
            } else if (p == "f") {
                String_t pp = commandLine.getRequiredParameter(p);
                try {
                    config.fieldList().addList(pp);
                }
                catch (Error& e) {
                    errorExit(afl::string::Format("'-f %s': %s", pp, e.what()));
                }
            } else if (p == "F") {
                opt_fields = true;
            } else if (p == "S") {
                arg_array = "SHIP";
            } else if (p == "P") {
                arg_array = "PLANET";
            } else if (p == "A") {
                arg_array = commandLine.getRequiredParameter(p);
            } else if (p == "t") {
                config.setFormatByName(commandLine.getRequiredParameter(p), tx);
            } else if (p == "o") {
                arg_outfile = commandLine.getRequiredParameter(p);
            } else if (p == "O") {
                config.setCharsetByName(commandLine.getRequiredParameter(p), tx);
                hadCharsetOption = true;
            } else if (p == "c") {
                afl::base::Ref<afl::io::Stream> file = fileSystem().openFile(commandLine.getRequiredParameter(p), afl::io::FileSystem::OpenRead);
                Configuration tmpConfig(config);
                tmpConfig.load(*file, tx);
                config = tmpConfig;
            } else if (p == "h" || p == "help") {
                help();
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use '%s -h' for help."), environment().getInvocationName()));
            }
        } else {
            if (arg_race == 0 && afl::string::strToInteger(p, i) && i > 0 && i <= game::MAX_PLAYERS) {
                arg_race = i;
            } else if (!arg_gamedir.isValid()) {
                arg_gamedir = p;
            } else if (!arg_rootdir.isValid()) {
                arg_rootdir = p;
            } else {
                errorExit(tx("too many arguments"));
            }
        }
    }

    // Validate args
    String_t arg_array2;
    if (!arg_array.get(arg_array2)) {
        errorExit(tx("please specify the object type to export ('-P', '-S', '-A'). Use '-h' for help."));
    }

    // Default field set
    if (config.fieldList().size() == 0) {
        if (opt_fields) {
            config.fieldList().addList("NAME@-30,TYPE@-10");
        } else {
            config.fieldList().addList("ID@5,NAME@-30");
        }
    }

    // Set up game directories
    afl::io::FileSystem& fs = fileSystem();
    String_t defaultRoot = fs.makePathName(fs.makePathName(environment().getInstallationDirectoryName(), "share"), "specs");
    game::v3::RootLoader loader(fs.openDirectory(arg_rootdir.orElse(defaultRoot)), &profile, 0 /* pCallback */, translator(), log(), fs);

    // Check game data
    // FIXME: load correct config!
    const String_t usedGameDir = fs.getAbsolutePathName(arg_gamedir.orElse("."));
    const game::config::UserConfiguration uc;
    afl::base::Ptr<game::Root> root = loader.load(fs.openDirectory(usedGameDir), *gameCharset, uc, false);
    if (root.get() == 0 || root->getTurnLoader().get() == 0) {
        errorExit(afl::string::Format(tx("no game data found in directory \"%s\""), usedGameDir));
    }

    // Check player number
    if (arg_race != 0) {
        String_t extra;
        if (!root->getTurnLoader()->getPlayerStatus(arg_race, extra, translator()).contains(game::TurnLoader::Available)) {
            errorExit(afl::string::Format(tx("no game data available for player %d"), arg_race));
        }
    } else {
        arg_race = root->getTurnLoader()->getDefaultPlayer(root->playerList().getAllPlayers());
        if (arg_race == 0) {
            errorExit(tx("please specify the player number"));
        }
    }

    // Make a session and load it
    bool ok = false;
    game::Session session(translator(), fs);
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

    session.postprocessTurn(session.getGame()->currentTurn(), game::PlayerSet_t(arg_race), game::PlayerSet_t(arg_race), game::map::Object::ReadOnly);

    // What do we want to export?
    std::auto_ptr<Context> array(findArray(arg_array2, session.world()));
    if (opt_fields) {
        std::auto_ptr<MetaContext> meta(new MetaContext());
        array->enumProperties(*meta);
        array = meta;
    }

    // Do it.
    String_t outfile;
    if (arg_outfile.get(outfile)) {
        // Output to file
        afl::base::Ref<afl::io::Stream> s = fs.openFile(outfile, afl::io::FileSystem::Create);
        config.exportFile(*array, *s);
    } else {
        // Output to console. The console performs character set conversion.
        if (hadCharsetOption) {
            log().write(afl::sys::LogListener::Warn, "export", tx("WARNING: Option '-O' has been ignored because standard output is being used."));
        }
        if (!config.exportText(*array, standardOutput())) {
            errorExit(tx("the selected format needs an output file name ('-o')"));
        }
    }
}

void
interpreter::exporter::ConsoleApplication::help()
{
    afl::io::TextWriter& out = standardOutput();
    afl::string::Translator& tx = translator();
    out.writeLine(afl::string::Format(tx("PCC2 Export v%s - (c) 2017-2023 Stefan Reuther"), PCC2_VERSION));
    out.writeLine();
    out.writeLine(afl::string::Format(tx("Usage:\n"
                                         "  %s [-h]\n"
                                         "  %$0s [-opts] [-f F@W...] [-S|-P|-A OBJECT] [-t TYPE] DIR [ROOT] PLAYER\n\n"
                                         "%s"
                                         "\n"
                                         "Report bugs to <Streu@gmx.de>"),
                                      environment().getInvocationName(),
                                      util::formatOptions(tx("Options:\n"
                                                             "-C CHARSET\tSet game character set\n"
                                                             "-f FIELD@WIDTH\tAdd field to report\n"
                                                             "-S\tExport ships (same as '-A SHIP')\n"
                                                             "-P\tExport planets (same as '-A PLANET')\n"
                                                             "-A OBJECT\tExport specified object type (CCScript array name)\n"
                                                             "-t TYPE\tSet output file format/type\n"
                                                             "-o FILE\tSet output file name (default: stdout)\n"
                                                             "-O CHARSET\tSet output file character set (default: UTF-8)\n"
                                                             "-F\tExport list of fields instead of game data\n"
                                                             "-c FILE\tRead configuration from file\n"
                                                             "\n"
                                                             "Types:\n"
                                                             "dbf\tdBASE file (needs '-o')\n"
                                                             "text\tsimple text table, default\n"
                                                             "table\tboxy text table\n"
                                                             "csv, tsv, ssv\tcomma/tab/semicolon-separated values\n"
                                                             "json\tJSON (JavaScript)\n"
                                                             "html\tHTML\n"))));
    out.flush();
    exit(0);
}

interpreter::Context*
interpreter::exporter::ConsoleApplication::findArray(const String_t& name, World& world)
{
    // Look up name
    afl::data::NameMap::Index_t i = world.globalPropertyNames().getIndexByName(afl::string::strUCase(name));
    if (i == afl::data::NameMap::nil) {
        errorExit(afl::string::Format(translator()("unknown object type '%s'"), name));
    }

    // Check for array
    CallableValue* cv = dynamic_cast<CallableValue*>(world.globalValues()[i]);
    if (!cv) {
        errorExit(afl::string::Format(translator()("unknown object type '%s'"), name));
    }

    // Check for content
    try {
        Context* ctx = cv->makeFirstContext();
        if (!ctx) {
            errorExit(afl::string::Format(translator()("this game does not contain any objects of type '%s'"), name));
        }
        return ctx;
    }
    catch (Error& e) {
        // This happens when they do something like '-A CADD',
        // because CAdd refuses makeFirstContext() with
        // Error::typeError. No need to display the very
        // error message; it's not a known object type, period.
        errorExit(afl::string::Format(translator()("unknown object type '%s'"), name));
        return 0;
    }
}
