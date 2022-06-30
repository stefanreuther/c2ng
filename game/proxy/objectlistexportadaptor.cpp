/**
  *  \file game/proxy/objectlistexportadaptor.cpp
  *  \brief Class game::proxy::ObjectListExportAdaptor
  */

#include "game/proxy/objectlistexportadaptor.hpp"
#include "afl/base/ref.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/root.hpp"
#include "interpreter/context.hpp"

using game::config::UserConfiguration;

/*
 *  ObjectListExportAdaptor::Context
 *
 *  Provides iteration behaviour, but defers all other calls to an underlying (child) context.
 *  The child is created on-demand.
 *
 *  Alternative design: permanently keep a child around, leave the iteration to it,
 *  using the Id list as a filter only.
 */

class game::proxy::ObjectListExportAdaptor::Context : public interpreter::Context {
 public:
    Context(afl::base::Ref<Data> d, size_t index);

    // Context:
    virtual PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
    virtual bool next();
    virtual interpreter::Context* clone() const;
    virtual game::map::Object* getObject();
    virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);
    virtual String_t toString(bool readable) const;
    virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

 private:
    afl::base::Ref<Data> m_data;
    size_t m_index;
    std::auto_ptr<interpreter::Context> m_child;

    interpreter::Context* makeChild();
};

inline
game::proxy::ObjectListExportAdaptor::Context::Context(afl::base::Ref<Data> d, size_t index)
    : m_data(d), m_index(index), m_child()
{ }

interpreter::Context::PropertyAccessor*
game::proxy::ObjectListExportAdaptor::Context::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (interpreter::Context* ch = makeChild()) {
        return ch->lookup(name, result);
    } else {
        return 0;
    }
}

bool
game::proxy::ObjectListExportAdaptor::Context::next()
{
    const size_t newIndex = m_index+1;
    if (newIndex < m_data->ids.size()) {
        m_index = newIndex;
        m_child.reset();
        return true;
    } else {
        return false;
    }
}

interpreter::Context*
game::proxy::ObjectListExportAdaptor::Context::clone() const
{
    return new Context(m_data, m_index);
}

game::map::Object*
game::proxy::ObjectListExportAdaptor::Context::getObject()
{
    if (interpreter::Context* ch = makeChild()) {
        return ch->getObject();
    } else {
        return 0;
    }
}

void
game::proxy::ObjectListExportAdaptor::Context::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    if (interpreter::Context* ch = makeChild()) {
        ch->enumProperties(acceptor);
    }
}

String_t
game::proxy::ObjectListExportAdaptor::Context::toString(bool /*readable*/) const
{
    return "#<ObjectListExportAdaptor>";
}

void
game::proxy::ObjectListExportAdaptor::Context::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

interpreter::Context*
game::proxy::ObjectListExportAdaptor::Context::makeChild()
{
    if (m_child.get() == 0) {
        if (m_index < m_data->ids.size()) {
            switch (m_data->mode) {
             case Ships:
                m_child.reset(game::interface::ShipContext::create(m_data->ids[m_index], m_data->session));
                break;

             case Planets:
                m_child.reset(game::interface::PlanetContext::create(m_data->ids[m_index], m_data->session));
                break;
            }
        }
    }
    return m_child.get();
}


/*
 *  ObjectListExportAdaptor
 */

game::proxy::ObjectListExportAdaptor::ObjectListExportAdaptor(Session& session, Mode mode, const std::vector<Id_t>& ids)
    : m_data(*new Data(session, mode, ids))
{ }

void
game::proxy::ObjectListExportAdaptor::initConfiguration(interpreter::exporter::Configuration& config)
{
    if (const game::config::StringOption* opt = getOption()) {
        try {
            config.fieldList().addList((*opt)());
        }
        catch (...)
        { }
    }
}

void
game::proxy::ObjectListExportAdaptor::saveConfiguration(const interpreter::exporter::Configuration& config)
{
    if (game::config::StringOption* opt = getOption()) {
        opt->set(config.fieldList().toString());
    }
}

interpreter::Context*
game::proxy::ObjectListExportAdaptor::createContext()
{
    if (!m_data->ids.empty()) {
        return new Context(m_data, 0);
    } else {
        return 0;
    }
}

afl::io::FileSystem&
game::proxy::ObjectListExportAdaptor::fileSystem()
{
    return m_data->session.world().fileSystem();
}

afl::string::Translator&
game::proxy::ObjectListExportAdaptor::translator()
{
    return m_data->session.translator();
}

game::config::StringOption*
game::proxy::ObjectListExportAdaptor::getOption() const
{
    if (Root* r = m_data->session.getRoot().get()) {
        switch (m_data->mode) {
         case Ships:   return &r->userConfiguration()[UserConfiguration::ExportShipFields];
         case Planets: return &r->userConfiguration()[UserConfiguration::ExportPlanetFields];
        }
    }
    return 0;
}
