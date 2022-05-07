/**
  *  \file game/proxy/configurationeditorproxy.cpp
  *  \brief Class game::proxy::ConfigurationEditorProxy
  */

#include "game/proxy/configurationeditorproxy.hpp"
#include "afl/base/signalconnection.hpp"
#include "game/proxy/waitindicator.hpp"

using game::config::ConfigurationEditor;

/*
 *  Trampoline
 */

class game::proxy::ConfigurationEditorProxy::Trampoline {
 public:
    Trampoline(const util::RequestSender<ConfigurationEditorProxy>& reply, ConfigurationEditorAdaptor& adaptor);

    void setSource(size_t index, game::config::ConfigurationOption::Source src);
    void toggleValue(size_t index);
    void setValue(size_t index, String_t value);
    void loadValues();
    void packValues(Infos_t& values);

    void onConfigChange();
    void onEditorChange(size_t index);

 private:
    util::RequestSender<ConfigurationEditorProxy> m_reply;
    ConfigurationEditorAdaptor& m_adaptor;
    afl::base::SignalConnection conn_configChange;
    afl::base::SignalConnection conn_editorChange;
};

game::proxy::ConfigurationEditorProxy::Trampoline::Trampoline(const util::RequestSender<ConfigurationEditorProxy>& reply, ConfigurationEditorAdaptor& adaptor)
    : m_reply(reply),
      m_adaptor(adaptor),
      conn_configChange(adaptor.config().sig_change.add(this, &Trampoline::onConfigChange)),
      conn_editorChange(adaptor.editor().sig_change.add(this, &Trampoline::onEditorChange))
{
    loadValues();
}

void
game::proxy::ConfigurationEditorProxy::Trampoline::setSource(size_t index, game::config::ConfigurationOption::Source src)
{
    if (ConfigurationEditor::Node* p = m_adaptor.editor().getNodeByIndex(index)) {
        p->setSource(m_adaptor.config(), src);
        m_adaptor.notifyListeners();
    }
}

void
game::proxy::ConfigurationEditorProxy::Trampoline::toggleValue(size_t index)
{
    if (ConfigurationEditor::Node* p = m_adaptor.editor().getNodeByIndex(index)) {
        p->toggleValue(m_adaptor.config());
        m_adaptor.notifyListeners();
    }
}

void
game::proxy::ConfigurationEditorProxy::Trampoline::setValue(size_t index, String_t value)
{
    if (ConfigurationEditor::Node* p = m_adaptor.editor().getNodeByIndex(index)) {
        p->setValue(m_adaptor.config(), value);
        m_adaptor.notifyListeners();
    }
}

void
game::proxy::ConfigurationEditorProxy::Trampoline::loadValues()
{
    m_adaptor.editor().loadValues(m_adaptor.config(), m_adaptor.translator());
}

inline void
game::proxy::ConfigurationEditorProxy::Trampoline::packValues(Infos_t& values)
{
    ConfigurationEditor& ed = m_adaptor.editor();
    for (size_t i = 0, n = ed.getNumNodes(); i < n; ++i) {
        if (ConfigurationEditor::Node* p = ed.getNodeByIndex(i)) {
            values.push_back(p->describe(m_adaptor.config(), m_adaptor.translator()));
        }
    }
}

void
game::proxy::ConfigurationEditorProxy::Trampoline::onConfigChange()
{
    // Configuration changed: reconsider editor content and generate onEditorChange() callbacks
    m_adaptor.editor().updateValues(m_adaptor.config(), m_adaptor.translator());
}

void
game::proxy::ConfigurationEditorProxy::Trampoline::onEditorChange(size_t index)
{
    // Forward request to other side
    if (ConfigurationEditor::Node* p = m_adaptor.editor().getNodeByIndex(index)) {
        m_reply.postRequest(&ConfigurationEditorProxy::emitItemChange, index, p->describe(m_adaptor.config(), m_adaptor.translator()));
    }
}


/*
 *  TrampolineFromAdaptor
 */

class game::proxy::ConfigurationEditorProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(ConfigurationEditorAdaptor&)> {
 public:
    TrampolineFromAdaptor(const util::RequestSender<ConfigurationEditorProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(ConfigurationEditorAdaptor& adaptor)
        { return new Trampoline(m_reply, adaptor); }
 private:
    util::RequestSender<ConfigurationEditorProxy> m_reply;
};


/*
 *  ConfigurationEditorProxy
 */

game::proxy::ConfigurationEditorProxy::ConfigurationEditorProxy(util::RequestSender<ConfigurationEditorAdaptor> adaptorSender, util::RequestDispatcher& reply)
    : m_infos(),
      m_receiver(reply, *this),
      m_sender(adaptorSender.makeTemporary(new TrampolineFromAdaptor(m_receiver.getSender())))
{ }

game::proxy::ConfigurationEditorProxy::~ConfigurationEditorProxy()
{ }

void
game::proxy::ConfigurationEditorProxy::loadValues(WaitIndicator& ind)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Infos_t& values)
            : m_values(values)
            { }
        virtual void handle(Trampoline& tpl)
            {
                m_values.clear();
                tpl.loadValues();
                tpl.packValues(m_values);
            }
     private:
        Infos_t& m_values;
    };

    // Read into local variable (-> no aliasing even if existing copy is in use)
    Infos_t values;
    Task t(values);
    ind.call(m_sender, t);

    // Set values
    m_infos.swap(values);
}

const game::proxy::ConfigurationEditorProxy::Infos_t&
game::proxy::ConfigurationEditorProxy::getValues() const
{
    return m_infos;
}

void
game::proxy::ConfigurationEditorProxy::setSource(size_t index, game::config::ConfigurationOption::Source src)
{
    m_sender.postRequest(&Trampoline::setSource, index, src);
}

void
game::proxy::ConfigurationEditorProxy::toggleValue(size_t index)
{
    m_sender.postRequest(&Trampoline::toggleValue, index);
}

void
game::proxy::ConfigurationEditorProxy::setValue(size_t index, String_t value)
{
    m_sender.postRequest(&Trampoline::setValue, index, value);
}

void
game::proxy::ConfigurationEditorProxy::emitItemChange(size_t index, game::config::ConfigurationEditor::Info info)
{
    // Store locally
    if (index < m_infos.size()) {
        m_infos[index] = info;
    }

    // Forward to listener
    sig_itemChange.raise(index, info);
}
