/**
  *  \file tools/c2export.cpp
  *  \brief c2export Utility
  *
  *  This is the main function for the c2export (command line data exporter) utility.
  */

#include <cstdlib>
#include "afl/base/optional.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/defaultcharsetfactory.hpp"
#include "afl/io/textfile.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/limits.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/specificationloader.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "game/v3/rootloader.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/exporter/dbfexporter.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "interpreter/exporter/htmlexporter.hpp"
#include "interpreter/exporter/jsonexporter.hpp"
#include "interpreter/exporter/separatedtextexporter.hpp"
#include "interpreter/exporter/textexporter.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/values.hpp"
#include "util/application.hpp"
#include "util/constantanswerprovider.hpp"
#include "util/profiledirectory.hpp"
#include "util/translation.hpp"
#include "version.hpp"

namespace {
    const char LOG_NAME[] = "export";

    enum OutputType {
        tText,
        tTable,
        tCommaSV,
        tTabSV,
        tSemicolonSV,
        tJSON,
        tHTML,
        tDBF
    };

    /* Meta-context for generating field names. Used to implement '-F'. */
    // FIXME: move into library
    class MetaContext : public interpreter::Context,
                        public interpreter::PropertyAcceptor
    {
     public:
        MetaContext();

        // Context:
        virtual MetaContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual MetaContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

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

    MetaContext* MetaContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
    {
        return lookupName(name, meta_mapping, result) ? this : 0;
    }

    void MetaContext::set(PropertyIndex_t /*index*/, afl::data::Value* /*value*/)
    {
        throw interpreter::Error::notAssignable();
    }

    afl::data::Value* MetaContext::get(PropertyIndex_t index)
    {
        if (m_position < m_names.size()) {
            switch (meta_mapping[index].index) {
             case 0:
                return interpreter::makeIntegerValue(int32_t(m_position));
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

    void MetaContext::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
    {
        throw interpreter::Error::notSerializable();
    }

    void MetaContext::addProperty(const String_t& name, interpreter::TypeHint th)
    {
        m_names.push_back(name);
        m_types.push_back(th);
    }



    void doTextExport(OutputType typ, interpreter::exporter::FieldList& job, interpreter::Context* ctx, afl::io::TextWriter& tf)
    {
        switch (typ) {
         case tText:
            interpreter::exporter::TextExporter(tf, false).doExport(ctx, util::ConstantAnswerProvider::sayYes, job);
            break;
         case tTable:
            interpreter::exporter::TextExporter(tf, true).doExport(ctx, util::ConstantAnswerProvider::sayYes, job);
            break;
         case tCommaSV:
            interpreter::exporter::SeparatedTextExporter(tf, ',').doExport(ctx, util::ConstantAnswerProvider::sayYes, job);
            break;
         case tTabSV:
            interpreter::exporter::SeparatedTextExporter(tf, '\t').doExport(ctx, util::ConstantAnswerProvider::sayYes, job);
            break;
         case tSemicolonSV:
            interpreter::exporter::SeparatedTextExporter(tf, ';').doExport(ctx, util::ConstantAnswerProvider::sayYes, job);
            break;
         case tJSON:
            interpreter::exporter::JsonExporter(tf).doExport(ctx, util::ConstantAnswerProvider::sayYes, job);
            break;
         case tHTML:
            interpreter::exporter::HtmlExporter(tf).doExport(ctx, util::ConstantAnswerProvider::sayYes, job);
            break;
         case tDBF:
            /* handled outside */
            break;
        }
    }

    class ConsoleExportApplication : public util::Application {
     public:
        ConsoleExportApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        void appMain();

     private:
        void help();
        String_t fetchArg(const char* opt, afl::sys::CommandLineParser& parser);
        interpreter::Context* findArray(const String_t& name, interpreter::World& world);
    };
}

void
ConsoleExportApplication::appMain()
{
    util::ProfileDirectory profile(environment(), fileSystem(), translator(), log());

    // Parse args
    interpreter::exporter::FieldList job;

    afl::base::Optional<String_t> arg_array;
    afl::base::Optional<String_t> arg_gamedir;
    afl::base::Optional<String_t> arg_rootdir;
    afl::base::Optional<String_t> arg_outfile;
    OutputType arg_typ = tText;
    int arg_race = 0;
    bool opt_fields = false;
    std::auto_ptr<afl::charset::Charset> gameCharset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
    std::auto_ptr<afl::charset::Charset> outputCharset;

    int i;
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "C") {
                if (afl::charset::Charset* cs = afl::charset::DefaultCharsetFactory().createCharset(fetchArg("-C", commandLine))) {
                    gameCharset.reset(cs);
                } else {
                    errorExit(_("the specified character set is not known"));
                }
            } else if (p == "f") {
                String_t pp = fetchArg("-f", commandLine);
                try {
                    job.addList(pp);
                }
                catch (interpreter::Error& e) {
                    errorExit(afl::string::Format("'-f %s': %s", pp, e.what()));
                }
            } else if (p == "F") {
                opt_fields = true;
            } else if (p == "S") {
                arg_array = "SHIP";
            } else if (p == "P") {
                arg_array = "PLANET";
            } else if (p == "A") {
                arg_array = fetchArg("-A", commandLine);
            } else if (p == "t") {
                String_t t = fetchArg("-t", commandLine);
                if (t == "text") {
                    arg_typ = tText;
                } else if (t == "table") {
                    arg_typ = tTable;
                } else if (t == "dbf") {
                    arg_typ = tDBF;
                } else if (t == "csv") {
                    arg_typ = tCommaSV;
                } else if (t == "tsv") {
                    arg_typ = tTabSV;
                } else if (t == "ssv") {
                    arg_typ = tSemicolonSV;
                } else if (t == "json") {
                    arg_typ = tJSON;
                } else if (t == "html") {
                    arg_typ = tHTML;
                } else {
                    errorExit(_("invalid output format specified"));
                }
            } else if (p == "o") {
                arg_outfile = fetchArg("-o", commandLine);
            } else if (p == "O") {
                if (afl::charset::Charset* cs = afl::charset::DefaultCharsetFactory().createCharset(fetchArg("-O", commandLine))) {
                    outputCharset.reset(cs);
                } else {
                    errorExit(_("the specified character set is not known"));
                }
            } else if (p == "h" || p == "help") {
                help();
            } else {
                errorExit(afl::string::Format(_("invalid option specified. Use '%s -h' for help.").c_str(), environment().getInvocationName()));
            }
        } else {
            if (arg_race == 0 && afl::string::strToInteger(p, i) && i > 0 && i <= game::MAX_PLAYERS) {
                arg_race = i;
            } else if (!arg_gamedir.isValid()) {
                arg_gamedir = p;
            } else if (!arg_rootdir.isValid()) {
                arg_rootdir = p;
            } else {
                errorExit(_("too many arguments"));
            }
        }
    }

    // Validate args
    String_t arg_array2;
    if (!arg_array.get(arg_array2)) {
        errorExit(_("please specify the object type to export ('-P', '-S', '-A'). Use '-h' for help."));
    }

    // Default field set
    if (job.size() == 0) {
        if (opt_fields) {
            job.addList("NAME@30,TYPE@10");
        } else {
            job.addList("ID@5,NAME@30");
        }
    }

    try {
        // Set up game directories
        afl::io::FileSystem& fs = fileSystem();
        String_t defaultRoot = fs.makePathName(fs.makePathName(environment().getInstallationDirectoryName(), "share"), "specs");
        game::v3::RootLoader loader(fs.openDirectory(arg_rootdir.orElse(defaultRoot)), profile, translator(), log(), fs);
        loader.setCharsetNew(gameCharset->clone());

        // Check game data
        afl::base::Ptr<game::Root> root = loader.load(fs.openDirectory(arg_gamedir.orElse(".")), false);
        if (root.get() == 0 || root->getTurnLoader().get() == 0) {
            errorExit(_("no game data found"));
        }

        // Check player number
        if (arg_race != 0) {
            String_t extra;
            if (!root->getTurnLoader()->getPlayerStatus(arg_race, extra, translator()).contains(game::TurnLoader::Available)) {
                errorExit(afl::string::Format(_("no game data available for player %d").c_str(), arg_race));
            }
        } else {
            arg_race = root->getTurnLoader()->getDefaultPlayer(root->playerList().getAllPlayers());
            if (arg_race == 0) {
                errorExit(_("please specify the player number"));
            }
        }

        // Make a session and load it
        game::Session session(translator(), fs);
        session.setGame(new game::Game());
        session.setRoot(root);
        session.setShipList(new game::spec::ShipList());
        root->specificationLoader().loadShipList(*session.getShipList(), *root);

        root->getTurnLoader()->loadCurrentTurn(session.getGame()->currentTurn(), *session.getGame(), arg_race, *root, session);
        session.getGame()->currentTurn().universe().postprocess(game::PlayerSet_t(arg_race), game::PlayerSet_t(arg_race), game::map::Object::ReadOnly,
                                                                root->hostVersion(), root->hostConfiguration(),
                                                                session.getGame()->currentTurn().getTurnNumber(),
                                                                translator(), log());

        // What do we want to export?
        std::auto_ptr<interpreter::Context> array(findArray(arg_array2, session.world()));
        if (opt_fields) {
            std::auto_ptr<MetaContext> meta(new MetaContext());
            array->enumProperties(*meta);
            array = meta;
        }

        // Do it.
        if (arg_typ == tDBF) {
            // Output to DBF file. Requires file name.
            String_t outfile;
            if (!arg_outfile.get(outfile)) {
                errorExit(_("output to DBF file needs an output file name ('-o')"));
            }
            afl::base::Ref<afl::io::Stream> s = fs.openFile(outfile, afl::io::FileSystem::Create);
            interpreter::exporter::DbfExporter(*s).doExport(array.get(), util::ConstantAnswerProvider::sayYes, job);
        } else {
            String_t outfile;
            if (!arg_outfile.get(outfile)) {
                // Output to console. The console performs character set conversion.
                if (outputCharset.get() != 0) {
                    log().write(afl::sys::LogListener::Warn, "export", _("WARNING: Option '-O' has been ignored because standard output is being used."));
                }
                doTextExport(arg_typ, job, array.get(), standardOutput());
            } else {
                // Output to file
                afl::base::Ref<afl::io::Stream> s = fs.openFile(outfile, afl::io::FileSystem::Create);
                afl::io::TextFile tf(*s);
                if (outputCharset.get() != 0) {
                    tf.setCharsetNew(outputCharset->clone());
                }
                doTextExport(arg_typ, job, array.get(), tf);
                tf.flush();
            }
        }
    }
    catch (game::Exception& ge) {
        errorExit(ge.getUserError());
    }
    // Other exceptions handled by caller.
}

void
ConsoleExportApplication::help()
{
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(afl::string::Format(_("PCC2 Export v%s - (c) 2016 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(afl::string::Format(_("Usage:\n"
                                        "  %s [-h]\n"
                                        "  %$0s [-opts] [-f F@W...] [-S|-P|-A OBJECT] [-t TYPE] DIR [ROOT] PLAYER\n\n"
                                        "Options:\n"
                                        "  -C CHARSET      Set game character set\n"
                                        "  -f FIELD@WIDTH  Add field to report\n"
                                        "  -S              Export ships (same as '-A SHIP')\n"
                                        "  -P              Export planets (same as '-A PLANET')\n"
                                        "  -A OBJECT       Export specified object type (CCScript array name)\n"
                                        "  -t TYPE         Set output file format/type\n"
                                        "  -o FILE         Set output file name (default: stdout)\n"
                                        "  -O CHARSET      Set output file character set (default: UTF-8)\n"
                                        "  -F              Export list of fields instead of game data\n"
                                        "\n"
                                        "Types:\n"
                                        "  dbf             dBASE file (needs '-o')\n"
                                        "  text            simple text table, default\n"
                                        "  table           boxy text table\n"
                                        "  csv, tsv, ssv   comma/tab/semicolon-separated values\n"
                                        "  json            JSON (JavaScript)\n"
                                        "  html            HTML\n"
                                        "\n"
                                        "Report bugs to <Streu@gmx.de>\n").c_str(),
                                      environment().getInvocationName()));
    out.flush();
    exit(0);
}

String_t
ConsoleExportApplication::fetchArg(const char* opt, afl::sys::CommandLineParser& parser)
{
    String_t result;
    if (!parser.getParameter(result)) {
        errorExit(afl::string::Format(_("option '%s' needs an argument").c_str(), opt));
    }
    return result;
}


interpreter::Context*
ConsoleExportApplication::findArray(const String_t& name, interpreter::World& world)
{
    // Look up name
    afl::data::NameMap::Index_t i = world.globalPropertyNames().getIndexByName(afl::string::strUCase(name));
    if (i == afl::data::NameMap::nil) {
        errorExit(afl::string::Format(_("unknown object type '%s'").c_str(), name));
    }

    // Check for array
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(world.globalValues()[i]);
    if (!cv) {
        errorExit(afl::string::Format(_("unknown object type '%s'").c_str(), name));
    }

    // Check for content
    try {
        interpreter::Context* ctx = cv->makeFirstContext();
        if (!ctx) {
            errorExit(afl::string::Format(_("this game does not contain any objects of type '%s'").c_str(), name));
        }
        return ctx;
    }
    catch (interpreter::Error& e) {
        // This happens when they do something like '-ACADD',
        // because CAdd refuses makeFirstContext() with an
        // IntError::typeError. No need to display the very
        // error message; it's not a known object type, period.
        errorExit(afl::string::Format(_("unknown object type '%s'").c_str(), name));
        return 0;
    }
}

int main(int, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return ConsoleExportApplication(env, fs).run();
}
