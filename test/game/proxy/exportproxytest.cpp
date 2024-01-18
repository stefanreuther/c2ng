/**
  *  \file test/game/proxy/exportproxytest.cpp
  *  \brief Test for game::proxy::ExportProxy
  */

#include "game/proxy/exportproxy.hpp"

#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/waitindicator.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplecontext.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/values.hpp"
#include "util/requestsender.hpp"

using afl::base::Ref;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using afl::io::FileMapping;
using afl::io::FileSystem;
using afl::io::Stream;
using interpreter::Context;
using interpreter::SimpleContext;
using util::CharsetFactory;

namespace {
    /* Remove a character (used for CR-removal) */
    String_t removeCharacter(String_t s, char ch)
    {
        size_t i = 0;
        while (i < s.size()) {
            if (s[i] == ch) {
                s.erase(i, 1);
            } else {
                ++i;
            }
        }
        return s;
    }

    /* TestContext - same as for interpreter::exporter::Configuration */
    class TestContext : public SimpleContext, public Context::ReadOnlyAccessor {
     public:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match("ID")) {
                    result = 1;
                    return this;
                } else if (name.match("NAME")) {
                    result = 2;
                    return this;
                } else {
                    return 0;
                }
            }
        virtual afl::data::Value* get(PropertyIndex_t index)
            { return index == 1 ? interpreter::makeIntegerValue(42) : interpreter::makeStringValue("Fred"); }
        virtual bool next()
            { return false; }
        virtual TestContext* clone() const
            { return new TestContext(); }
        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const
            {
                acceptor.addProperty("ID", interpreter::thInt);
                acceptor.addProperty("NAME", interpreter::thString);
            }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "<TestContext>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { }
    };

    /*
     *  TestAdaptor
     *
     *  Publish a given FileSystem instance and initialize with a given set of fields.
     *  In addition, record field list in saveConfiguration for verification.
     */
    class TestAdaptor : public game::proxy::ExportAdaptor {
     public:
        TestAdaptor(FileSystem& fs, String_t initFields)
            : m_fileSystem(fs),
              m_translator(),
              m_initFields(initFields),
              m_savedFields()
            { }
        virtual void initConfiguration(interpreter::exporter::Configuration& config)
            { config.fieldList().addList(m_initFields); }
        virtual void saveConfiguration(const interpreter::exporter::Configuration& config)
            { m_savedFields = config.fieldList().toString(); }
        virtual Context* createContext()
            { return new TestContext(); }
        virtual FileSystem& fileSystem()
            { return m_fileSystem; }
        virtual afl::string::Translator& translator()
            { return m_translator; }

        const String_t& getSavedFields() const
            { return m_savedFields; }
     private:
        FileSystem& m_fileSystem;
        afl::string::NullTranslator m_translator;
        String_t m_initFields;
        String_t m_savedFields;
    };

    /* Event listener */
    class TestCallback {
     public:
        TestCallback()
            : m_config(),
              m_ok()
            { }
        void onChange(const interpreter::exporter::Configuration& config)
            {
                m_config = config;
                m_ok = true;
            }
        bool isOK() const
            { return m_ok; }
        const interpreter::exporter::Configuration& config() const
            { return m_config; }
     private:
        interpreter::exporter::Configuration m_config;
        bool m_ok;
    };
}

/** General test: setup, status inquiry and event routing. */
AFL_TEST("game.proxy.ExportProxy:basics", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy. At this time, nothing happens yet at the adaptor (no callback, no saveConfiguration())
    game::proxy::ExportProxy testee(recv.getSender(), ind);
    TestCallback cb;
    testee.sig_change.add(&cb, &TestCallback::onChange);
    ind.processQueue();
    a.checkEqual("01. getSavedFields", ad.getSavedFields(), "");
    a.checkEqual("02. isOK", cb.isOK(), false);

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("11. fieldList", config.fieldList().toString(), "ID@10,NAME");

    // Get list of possible fields
    afl::data::StringList_t list;
    testee.enumProperties(ind, list);
    a.checkEqual("21. size", list.size(), 2U);
    a.checkEqual("22. list", list[0], "ID");
    a.checkEqual("23. list", list[1], "NAME");

    // Add a field; this will cause callbacks
    testee.add(0, "Id", 17);
    ind.processQueue();
    a.checkEqual("31. getSavedFields", ad.getSavedFields(), "ID@17,ID@10,NAME");
    a.checkEqual("32. isOK", cb.isOK(), true);
    a.checkEqual("33. fieldList", cb.config().fieldList().toString(), "ID@17,ID@10,NAME");
}

/** Test setCharsetIndex(). */
AFL_TEST("game.proxy.ExportProxy:setCharsetIndex", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action: set a character set that is not default
    CharsetFactory::Index_t csx = 0;
    a.check("01. charset", CharsetFactory().findIndexByKey("koi8r").get(csx));
    testee.setCharsetIndex(csx);
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("11. getCharsetIndex", config.getCharsetIndex(), csx);
}

/** Test setFormat(). */
AFL_TEST("game.proxy.ExportProxy:setFormat", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    testee.setFormat(interpreter::exporter::DBaseFormat);
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("01. getFormat", config.getFormat(), interpreter::exporter::DBaseFormat);
}

/** Test add(). */
AFL_TEST("game.proxy.ExportProxy:add", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    testee.add(2, "ID", 0);
    testee.add(0, "NAME", -20);
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("01. fieldList", config.fieldList().toString(), "NAME@-20,ID@10,NAME,ID");
}

/** Test swap(). */
AFL_TEST("game.proxy.ExportProxy:swap", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    testee.swap(0, 1);
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("01. fieldList", config.fieldList().toString(), "NAME,ID@10");
}

/** Test remove(). */
AFL_TEST("game.proxy.ExportProxy:remove", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    testee.remove(1);
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("01. fieldList", config.fieldList().toString(), "ID@10");
}

/** Test clear(). */
AFL_TEST("game.proxy.ExportProxy:clear", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    testee.clear();
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("01. fieldList", config.fieldList().toString(), "");
}

/** Test setFieldName(). */
AFL_TEST("game.proxy.ExportProxy:setFieldName", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    testee.setFieldName(0, "NAME");
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("01. fieldList", config.fieldList().toString(), "NAME@10,NAME");
}

/** Test setFieldWidth(). */
AFL_TEST("game.proxy.ExportProxy:setFieldWidth", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    testee.setFieldWidth(1, -42);
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("01. fieldList", config.fieldList().toString(), "ID@10,NAME@-42");
}

/** Test changeFieldWidth(). */
AFL_TEST("game.proxy.ExportProxy:changeFieldWidth", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    testee.changeFieldWidth(0, -30);
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("01. fieldList", config.fieldList().toString(), "ID,NAME");
}

/** Test toggleFieldAlignment(). */
AFL_TEST("game.proxy.ExportProxy:toggleFieldAlignment", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    testee.toggleFieldAlignment(0);
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("01. fieldList", config.fieldList().toString(), "ID@-10,NAME");
}

/** Test exportFile() on regular file system: should create file. */
AFL_TEST("game.proxy.ExportProxy:exportFile", a)
{
    // File system
    FileSystem& fs = FileSystem::getInstance();
    Ref<Directory> dir = fs.openDirectory(fs.getWorkingDirectoryName());
    Ref<DirectoryEntry> entry = dir->getDirectoryEntryByName("__testex.tmp");

    // Environment/initial state
    game::test::WaitIndicator ind;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Configure
    testee.setCharsetIndex(CharsetFactory::LATIN1_INDEX);
    testee.setFormat(interpreter::exporter::CommaSVFormat);

    // Action
    String_t err;
    a.check("01. exportFile", testee.exportFile(ind, entry->getPathName(), err));

    // Verify
    {
        Ref<Stream> in = entry->openFile(FileSystem::OpenRead);
        Ref<FileMapping> map = in->createVirtualMapping();
        a.checkEqual("11. content", removeCharacter(afl::string::fromBytes(map->get()), '\r'),
                     "\"ID\",\"NAME\""
                     "\n42,Fred\n");
    }
    entry->eraseNT();
}

/** Test exportFile() on NullFileSystem: should report error. */
AFL_TEST("game.proxy.ExportProxy:exportFile:NullFileSystem", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Configure
    testee.setCharsetIndex(CharsetFactory::LATIN1_INDEX);
    testee.setFormat(interpreter::exporter::CommaSVFormat);

    // Action
    String_t err;
    a.check("01. exportFile", !testee.exportFile(ind, "file.txt", err));
    a.checkDifferent("02. error", err, "");
}

/** Test exportFile() with null Context: should report error. */
AFL_TEST("game.proxy.ExportProxy:exportFile:null-context", a)
{
    class NullAdaptor : public game::proxy::ExportAdaptor {
     public:
        NullAdaptor(game::proxy::ExportAdaptor& outer)
            : m_outer(outer)
            { }
        virtual void initConfiguration(interpreter::exporter::Configuration& config)
            { m_outer.initConfiguration(config); }
        virtual void saveConfiguration(const interpreter::exporter::Configuration& config)
            { m_outer.saveConfiguration(config); }
        virtual Context* createContext()
            { return 0; }
        virtual FileSystem& fileSystem()
            { return m_outer.fileSystem(); }
        virtual afl::string::Translator& translator()
            { return m_outer.translator(); }
     private:
        ExportAdaptor& m_outer;
    };

    // File system
    FileSystem& fs = FileSystem::getInstance();
    Ref<Directory> dir = fs.openDirectory(fs.getWorkingDirectoryName());
    Ref<DirectoryEntry> entry = dir->getDirectoryEntryByName("__testex.tmp");
    entry->eraseNT();
    a.checkEqual("01. getFileType", entry->getFileType(), DirectoryEntry::tUnknown);

    // Environment/initial state
    game::test::WaitIndicator ind;
    TestAdaptor outer(fs, "ID@10,NAME");
    NullAdaptor inner(outer);
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, inner);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Configure
    testee.setCharsetIndex(CharsetFactory::LATIN1_INDEX);
    testee.setFormat(interpreter::exporter::CommaSVFormat);

    // Action: must fail
    String_t err;
    a.check("11. exportFile", !testee.exportFile(ind, entry->getPathName(), err));
    a.checkDifferent("12. error", err, "");

    // Verify
    a.checkEqual("21. getFileType", entry->getFileType(), DirectoryEntry::tUnknown);
    entry->eraseNT();

    // Get list of possible fields: must be empty
    afl::data::StringList_t list;
    testee.enumProperties(ind, list);
    a.checkEqual("31. size", list.size(), 0U);
}

/** Test load() on regular file system. */
AFL_TEST("game.proxy.ExportProxy:load", a)
{
    // File system
    FileSystem& fs = FileSystem::getInstance();
    Ref<Directory> dir = fs.openDirectory(fs.getWorkingDirectoryName());
    Ref<DirectoryEntry> entry = dir->getDirectoryEntryByName("__testex.tmp");
    entry->openFile(FileSystem::Create)->fullWrite(afl::string::toBytes("fields=name\n"
                                                                        "fields=id\n"
                                                                        "charset=koi8r\n"));

    // Environment/initial state
    game::test::WaitIndicator ind;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    String_t err;
    a.check("01. load", testee.load(ind, entry->getPathName(), err));

    // Verify
    a.checkEqual("11. getSavedFields", ad.getSavedFields(), "NAME,ID");

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    a.checkEqual("21. fieldList", config.fieldList().toString(), "NAME,ID");
    a.checkEqual("22. charset", CharsetFactory().getCharsetKey(config.getCharsetIndex()), "koi8r");
    entry->eraseNT();
}

/** Test load() on NullFileSystem: must report error. */
AFL_TEST("game.proxy.ExportProxy:load:error", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    String_t err;
    a.check("01. load", !testee.load(ind, "file.txt", err));
    a.checkDifferent("02. error", err, "");
}

/** Test save() on regular file system. */
AFL_TEST("game.proxy.ExportProxy:save", a)
{
    // File system
    FileSystem& fs = FileSystem::getInstance();
    Ref<Directory> dir = fs.openDirectory(fs.getWorkingDirectoryName());
    Ref<DirectoryEntry> entry = dir->getDirectoryEntryByName("__testex.tmp");
    entry->eraseNT();

    // Environment/initial state
    game::test::WaitIndicator ind;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);
    testee.setCharsetIndex(CharsetFactory::LATIN1_INDEX);
    testee.setFormat(interpreter::exporter::TextFormat);

    // Action
    String_t err;
    a.check("01. save", testee.save(ind, entry->getPathName(), err));

    // Verify
    {
        Ref<Stream> in = entry->openFile(FileSystem::OpenRead);
        Ref<FileMapping> map = in->createVirtualMapping();
        a.checkEqual("11. file content", removeCharacter(afl::string::fromBytes(map->get()), '\r'),
                     "Fields=Id@10\n"
                     "Fields=Name\n"
                     "Charset=latin1\n"
                     "Format=text\n");
    }
    entry->eraseNT();
}

/** Test save() on NullFileSystem: must report error. */
AFL_TEST("game.proxy.ExportProxy:save:error", a)
{
    // Environment/initial state
    game::test::WaitIndicator ind;
    afl::io::NullFileSystem fs;
    TestAdaptor ad(fs, "ID@10,NAME");
    util::RequestReceiver<game::proxy::ExportAdaptor> recv(ind, ad);

    // Create proxy
    game::proxy::ExportProxy testee(recv.getSender(), ind);

    // Action
    String_t err;
    a.check("01. save", !testee.save(ind, "file.txt", err));
    a.checkDifferent("02. error", err, "");
}
