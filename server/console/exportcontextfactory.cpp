/**
  *  \file server/console/exportcontextfactory.cpp
  */

#include "server/console/exportcontextfactory.hpp"

#include "afl/sys/longcommandlineparser.hpp"
#include "server/console/context.hpp"
#include "server/console/dbexporter.hpp"
#include "server/console/fileexporter.hpp"
#include "server/console/parser.hpp"
#include "server/console/terminal.hpp"
#include "server/ports.hpp"
#include "server/types.hpp"
#include "util/string.hpp"

/*
 *  Command Handler
 */

class server::console::ExportContextFactory::Impl : public Context {
 public:
    Impl(ClientPool& pool, ClientPool::Index_t index)
        : m_pool(pool), m_index(index)
        { }
    virtual bool call(const String_t& cmd, interpreter::Arguments args, Parser& parser, std::auto_ptr<afl::data::Value>& result)
        {
            if (cmd == "db") {
                exportDatabase(parser.terminal(), m_pool.get(m_index), args);
                result.reset();
                return true;
            } else if (cmd == "object") {
                exportObject(parser.terminal(), parser.fileSystem(), args);
                result.reset();
                return true;
            } else {
                return false;
            }
        }

    virtual String_t getName()
        { return "export"; }

 private:
    void exportObject(Terminal& term, afl::io::FileSystem& fs, interpreter::Arguments& args)
        {
            DbExporter exp;
            DbExporter::Level level = DbExporter::Recursive;
            FileExporter::Mode mode = FileExporter::InlineFiles;
            while (args.getNumArgs() > 0) {
                String_t it = toString(args.getNext());
                if (util::isOption(it, "recursive")) {
                    level = DbExporter::Recursive;
                } else if (util::isOption(it, "full")) {
                    level = DbExporter::Full;
                } else if (util::isOption(it, "header")) {
                    level = DbExporter::Header;
                } else if (util::isOption(it, "files=none")) {
                    mode = FileExporter::NoFiles;
                } else if (util::isOption(it, "files=inline")) {
                    mode = FileExporter::InlineFiles;
                } else if (util::isOption(it, "files=normal")) {
                    mode = FileExporter::NormalFiles;
                } else if (util::isOption(it, "files=dedup")) {
                    mode = FileExporter::DedupFiles;
                } else if (util::isOption(it)) {
                    throw std::runtime_error("invalid option specified");
                } else if (const char* p = util::strStartsWith(it, "user:")) {
                    exp.add(DbExporter::User, p, level);
                } else if (const char* p = util::strStartsWith(it, "pm:")) {
                    exp.add(DbExporter::PM, p, level);
                } else if (const char* p = util::strStartsWith(it, "email:")) {
                    exp.add(DbExporter::Email, p, level);
                } else if (const char* p = util::strStartsWith(it, "forum:")) {
                    exp.add(DbExporter::Forum, p, level);
                } else if (const char* p = util::strStartsWith(it, "thread:")) {
                    exp.add(DbExporter::Thread, p, level);
                } else if (const char* p = util::strStartsWith(it, "post:")) {
                    exp.add(DbExporter::Post, p, level);
                } else if (const char* p = util::strStartsWith(it, "msg:")) {
                    exp.add(DbExporter::Post, p, level);
                } else if (const char* p = util::strStartsWith(it, "group:")) {
                    exp.add(DbExporter::Group, p, level);
                } else if (const char* p = util::strStartsWith(it, "game:")) {
                    exp.add(DbExporter::Game, p, level);
                } else if (const char* p = util::strStartsWith(it, "tool:")) {
                    exp.add(DbExporter::Tool, p, level);
                } else if (const char* p = util::strStartsWith(it, "shiplist:")) {
                    exp.add(DbExporter::ShipList, p, level);
                } else if (const char* p = util::strStartsWith(it, "master:")) {
                    exp.add(DbExporter::Master, p, level);
                } else if (const char* p = util::strStartsWith(it, "host:")) {
                    exp.add(DbExporter::Host, p, level);
                } else if (const char* p = util::strStartsWith(it, "token:")) {
                    exp.add(DbExporter::Token, p, level);
                } else if (const char* p = util::strStartsWith(it, "file:")) {
                    exp.add(DbExporter::UserFile, p, level);
                } else if (const char* p = util::strStartsWith(it, "userfile:")) {
                    exp.add(DbExporter::UserFile, p, level);
                } else if (const char* p = util::strStartsWith(it, "hostfile:")) {
                    exp.add(DbExporter::HostFile, p, level);
                } else {
                    throw std::runtime_error("invalid object specified");
                }
            }
            exp.complete(m_pool.get(m_index));
            exp.generate(term, m_pool.get(m_index));
            term.printOutput("");
            if (exp.hasAny(DbExporter::UserFile)) {
                std::auto_ptr<FileExporter> fx(FileExporter::create(mode, fs, "user"));
                if (fx.get() != 0) {
                    term.printOutput("# Files");
                    exp.generateUserFiles(term, m_pool.get(m_pool.find("file")), *fx);
                    term.printOutput("");
                }
            }
            if (exp.hasAny(DbExporter::HostFile)) {
                std::auto_ptr<FileExporter> fx(FileExporter::create(mode, fs, "host"));
                if (fx.get() != 0) {
                    term.printOutput("# Host Files");
                    exp.generateHostFiles(term, m_pool.get(m_pool.find("hostfile")), *fx);
                    term.printOutput("");
                }
            }
        }

    ClientPool& m_pool;
    ClientPool::Index_t m_index;
};

/*
 *  Entry point
 */

server::console::ExportContextFactory::ExportContextFactory(ClientPool& pool, ClientPool::Index_t index)
    : m_pool(pool),
      m_index(index)
{ }

server::console::ExportContextFactory::~ExportContextFactory()
{ }

String_t
server::console::ExportContextFactory::getCommandName()
{
    return "export";
}

server::console::Context*
server::console::ExportContextFactory::create()
{
    return new Impl(m_pool, m_index);
}

bool
server::console::ExportContextFactory::handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
{
    return false;
}
