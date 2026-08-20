/**
  *  \file game/proxy/simulationexportadaptor.cpp
  *  \brief ExportAdaptor for Battle Simulation
  */

#include "game/proxy/simulationexportadaptor.hpp"
#include "game/interface/simobjectcontext.hpp"

game::proxy::SimulationExportAdaptor_t*
game::proxy::makeSimulationExportAdaptor()
{
    // Adaptor
    class Adaptor : public ExportAdaptor {
     public:
        Adaptor(SimulationAdaptor& ad)
            : m_session(ad.simSession()),
              m_root(ad.getRoot()),
              m_shipList(ad.getShipList()),
              m_fileSystem(ad.fileSystem()),
              m_translator(ad.translator())
            { }
        virtual void initConfiguration(interpreter::exporter::Configuration& config)
            { config.fieldList().addList("ID@5,NAME@-20,OWNER$@6,HULL@-30"); }
        virtual void saveConfiguration(const interpreter::exporter::Configuration& /*config*/)
            { }
        virtual interpreter::Context* createContext()
            {
                if (m_root.get() != 0 && m_shipList.get() != 0 && m_session->setup().getNumObjects() > 0) {
                    return new game::interface::SimObjectContext(m_session, *m_root, *m_shipList, 0, m_translator);
                } else {
                    return 0;
                }
            }
        virtual afl::io::FileSystem& fileSystem()
            { return m_fileSystem; }
        virtual afl::string::Translator& translator()
            { return m_translator; }
     private:
        const afl::base::Ref<game::sim::Session> m_session;
        const afl::base::Ptr<const Root> m_root;
        const afl::base::Ptr<const game::spec::ShipList> m_shipList;
        afl::io::FileSystem& m_fileSystem;
        afl::string::Translator& m_translator;
    };

    // Adaptor factory
    class AdaptorFromSimulation : public SimulationExportAdaptor_t {
     public:
        ExportAdaptor* call(SimulationAdaptor& ad)
            { return new Adaptor(ad); }
    };

    return new AdaptorFromSimulation();
}
