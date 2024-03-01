/**
  *  \file game/proxy/vcrexportadaptor.cpp
  *  \brief VCR export adaptors
  */

#include "game/proxy/vcrexportadaptor.hpp"
#include "game/interface/vcrcontext.hpp"
#include "game/interface/vcrsidecontext.hpp"

using afl::base::Ref;
using afl::io::FileSystem;
using afl::string::Translator;
using game::spec::ShipList;
using game::vcr::Database;

namespace {
    /*
     *  Common part of adaptor
     */
    class BaseVcrAdaptor : public game::proxy::ExportAdaptor {
     public:
        BaseVcrAdaptor(game::proxy::VcrDatabaseAdaptor& db);
        ~BaseVcrAdaptor();

        virtual void saveConfiguration(const interpreter::exporter::Configuration& /*config*/)
            { }
        virtual FileSystem& fileSystem()
            { return m_fileSystem; }
        virtual Translator& translator()
            { return m_translator; }

     protected:
        Ref<const game::Root> m_root;
        Ref<const ShipList> m_shipList;
        Ref<Database> m_battles;
        FileSystem& m_fileSystem;
        Translator& m_translator;
    };
}

// Out-of-line implementation to reduce object code size
BaseVcrAdaptor::BaseVcrAdaptor(game::proxy::VcrDatabaseAdaptor& db)
    : m_root(db.getRoot()), m_shipList(db.getShipList()), m_battles(db.getBattles()), m_fileSystem(db.fileSystem()), m_translator(db.translator())
{ }

BaseVcrAdaptor::~BaseVcrAdaptor()
{ }

/*
 *  Public interface
 */

game::proxy::VcrExportAdaptor_t*
game::proxy::makeVcrExportAdaptor()
{
    // Adaptor
    class Adaptor : public BaseVcrAdaptor {
     public:
        Adaptor(VcrDatabaseAdaptor& db)
            : BaseVcrAdaptor(db)
            { }

        virtual void initConfiguration(interpreter::exporter::Configuration& config)
            { config.fieldList().addList("NUMUNITS@5,LEFT.ID@5,LEFT@-30,RIGHT.ID@5,RIGHT@-30"); }
        virtual interpreter::Context* createContext()
            { return new game::interface::VcrContext(0, m_translator, m_root, m_battles.asPtr(), m_shipList); }
    };

    // Closure
    class AdaptorFromDatabase : public VcrExportAdaptor_t {
     public:
        virtual ExportAdaptor* call(VcrDatabaseAdaptor& db)
            { return new Adaptor(db); }
    };
    return new AdaptorFromDatabase();
}

game::proxy::VcrExportAdaptor_t*
game::proxy::makeVcrSideExportAdaptor(size_t battleNr)
{
    // Adaptor
    class Adaptor : public BaseVcrAdaptor {
     public:
        Adaptor(VcrDatabaseAdaptor& db, size_t battleNr)
            : BaseVcrAdaptor(db), m_battleNumber(battleNr)
            { }
        virtual void initConfiguration(interpreter::exporter::Configuration& config)
            { config.fieldList().addList("ID@5,NAME@-20,OWNER$@2,STATUS@-15"); }
        virtual interpreter::Context* createContext()
            { return new game::interface::VcrSideContext(m_battleNumber, 0, m_translator, m_root, m_battles.asPtr(), m_shipList); }

     private:
        size_t m_battleNumber;
    };

    // Closure
    class AdaptorFromDatabase : public VcrExportAdaptor_t {
     public:
        AdaptorFromDatabase(size_t battleNr)
            : m_battleNumber(battleNr)
            { }
        virtual ExportAdaptor* call(VcrDatabaseAdaptor& db)
            { return new Adaptor(db, m_battleNumber); }
     private:
        size_t m_battleNumber;
    };
    return new AdaptorFromDatabase(battleNr);
}
