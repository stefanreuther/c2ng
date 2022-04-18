/**
  *  \file u/t_game_proxy_exportproxy.cpp
  *  \brief Test for game::proxy::ExportProxy
  */

#include "game/proxy/exportproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/waitindicator.hpp"
#include "interpreter/context.hpp"
#include "interpreter/propertyacceptor.hpp"
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
    class TestContext : public Context, public Context::ReadOnlyAccessor {
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
        virtual game::map::Object* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor)
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
        virtual void onChange(const interpreter::exporter::Configuration& config)
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
void
TestGameProxyExportProxy::testIt()
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
    TS_ASSERT_EQUALS(ad.getSavedFields(), "");
    TS_ASSERT_EQUALS(cb.isOK(), false);

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    TS_ASSERT_EQUALS(config.fieldList().toString(), "ID@10,NAME");

    // Get list of possible fields
    afl::data::StringList_t list;
    testee.enumProperties(ind, list);
    TS_ASSERT_EQUALS(list.size(), 2U);
    TS_ASSERT_EQUALS(list[0], "ID");
    TS_ASSERT_EQUALS(list[1], "NAME");

    // Add a field; this will cause callbacks
    testee.add(0, "Id", 17);
    ind.processQueue();
    TS_ASSERT_EQUALS(ad.getSavedFields(), "ID@17,ID@10,NAME");
    TS_ASSERT_EQUALS(cb.isOK(), true);
    TS_ASSERT_EQUALS(cb.config().fieldList().toString(), "ID@17,ID@10,NAME");
}

/** Test setCharsetIndex(). */
void
TestGameProxyExportProxy::testSetCharsetIndex()
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
    TS_ASSERT(CharsetFactory().findIndexByKey("koi8r", csx));
    testee.setCharsetIndex(csx);
    ind.processQueue();

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    TS_ASSERT_EQUALS(config.getCharsetIndex(), csx);
}

/** Test setFormat(). */
void
TestGameProxyExportProxy::testSetFormat()
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
    TS_ASSERT_EQUALS(config.getFormat(), interpreter::exporter::DBaseFormat);
}

/** Test add(). */
void
TestGameProxyExportProxy::testAdd()
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
    TS_ASSERT_EQUALS(config.fieldList().toString(), "NAME@-20,ID@10,NAME,ID");
}

/** Test swap(). */
void
TestGameProxyExportProxy::testSwap()
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
    TS_ASSERT_EQUALS(config.fieldList().toString(), "NAME,ID@10");
}

/** Test remove(). */
void
TestGameProxyExportProxy::testRemove()
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
    TS_ASSERT_EQUALS(config.fieldList().toString(), "ID@10");
}

/** Test clear(). */
void
TestGameProxyExportProxy::testClear()
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
    TS_ASSERT_EQUALS(config.fieldList().toString(), "");
}

/** Test setFieldName(). */
void
TestGameProxyExportProxy::testSetFieldName()
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
    TS_ASSERT_EQUALS(config.fieldList().toString(), "NAME@10,NAME");
}

/** Test setFieldWidth(). */
void
TestGameProxyExportProxy::testSetFieldWidth()
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
    TS_ASSERT_EQUALS(config.fieldList().toString(), "ID@10,NAME@-42");
}

/** Test changeFieldWidth(). */
void
TestGameProxyExportProxy::testChangeFieldWidth()
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
    TS_ASSERT_EQUALS(config.fieldList().toString(), "ID,NAME");
}

/** Test toggleFieldAlignment(). */
void
TestGameProxyExportProxy::testToggleFieldAlignment()
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
    TS_ASSERT_EQUALS(config.fieldList().toString(), "ID@-10,NAME");
}

/** Test exportFile() on regular file system: should create file. */
void
TestGameProxyExportProxy::testExportFile()
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
    TS_ASSERT(testee.exportFile(ind, entry->getPathName(), err));

    // Verify
    {
        Ref<Stream> in = entry->openFile(FileSystem::OpenRead);
        Ref<FileMapping> map = in->createVirtualMapping();
        TS_ASSERT_EQUALS(removeCharacter(afl::string::fromBytes(map->get()), '\r'),
                         "\"ID\",\"NAME\""
                         "\n42,Fred\n");
    }
    entry->eraseNT();
}

/** Test exportFile() on NullFileSystem: should report error. */
void
TestGameProxyExportProxy::testExportFileNullFS()
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
    TS_ASSERT(!testee.exportFile(ind, "file.txt", err));
    TS_ASSERT_DIFFERS(err, "");
}

/** Test exportFile() with null Context: should report error. */
void
TestGameProxyExportProxy::testExportFileNullContext()
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
    TS_ASSERT_EQUALS(entry->getFileType(), DirectoryEntry::tUnknown);

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
    TS_ASSERT(!testee.exportFile(ind, entry->getPathName(), err));
    TS_ASSERT_DIFFERS(err, "");

    // Verify
    TS_ASSERT_EQUALS(entry->getFileType(), DirectoryEntry::tUnknown);
    entry->eraseNT();

    // Get list of possible fields: must be empty
    afl::data::StringList_t list;
    testee.enumProperties(ind, list);
    TS_ASSERT_EQUALS(list.size(), 0U);
}

/** Test load() on regular file system. */
void
TestGameProxyExportProxy::testLoad()
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
    TS_ASSERT(testee.load(ind, entry->getPathName(), err));

    // Verify
    TS_ASSERT_EQUALS(ad.getSavedFields(), "NAME,ID");

    // Get status
    interpreter::exporter::Configuration config;
    testee.getStatus(ind, config);
    TS_ASSERT_EQUALS(config.fieldList().toString(), "NAME,ID");
    TS_ASSERT_EQUALS(CharsetFactory().getCharsetKey(config.getCharsetIndex()), "koi8r");
    entry->eraseNT();
}

/** Test load() on NullFileSystem: must report error. */
void
TestGameProxyExportProxy::testLoadError()
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
    TS_ASSERT(!testee.load(ind, "file.txt", err));
    TS_ASSERT_DIFFERS(err, "");
}

/** Test save() on regular file system. */
void
TestGameProxyExportProxy::testSave()
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
    TS_ASSERT(testee.save(ind, entry->getPathName(), err));

    // Verify
    {
        Ref<Stream> in = entry->openFile(FileSystem::OpenRead);
        Ref<FileMapping> map = in->createVirtualMapping();
        TS_ASSERT_EQUALS(removeCharacter(afl::string::fromBytes(map->get()), '\r'),
                         "Fields=Id@10\n"
                         "Fields=Name\n"
                         "Charset=latin1\n"
                         "Format=text\n");
    }    entry->eraseNT();
}

/** Test save() on NullFileSystem: must report error. */
void
TestGameProxyExportProxy::testSaveError()
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
    TS_ASSERT(!testee.save(ind, "file.txt", err));
    TS_ASSERT_DIFFERS(err, "");
}

