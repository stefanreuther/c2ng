/**
  *  \file game/proxy/exportproxy.cpp
  *  \brief Class game::proxy::ExportProxy
  */

#include "game/proxy/exportproxy.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
#include "interpreter/propertyacceptor.hpp"

using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using interpreter::Context;
using interpreter::exporter::Configuration;
using util::CharsetFactory;


/*
 *  Trampoline
 */

class game::proxy::ExportProxy::Trampoline {
 public:
    Trampoline(ExportAdaptor& adaptor, const util::RequestSender<ExportProxy>& reply);

    void packStatus(Configuration& out);

    void setCharsetIndex(CharsetFactory::Index_t index);
    void setFormat(interpreter::exporter::Format fmt);
    bool load(String_t fileName, String_t& errorMessage);
    bool save(String_t fileName, String_t& errorMessage);
    bool exportFile(String_t fileName, String_t& errorMessage);
    void add(Index_t index, String_t name, int width);
    void swap(Index_t a, Index_t b);
    void remove(Index_t index);
    void clear();
    void setFieldName(Index_t index, String_t name);
    void setFieldWidth(Index_t index, int width);
    void changeFieldWidth(Index_t index, int delta);
    void toggleFieldAlignment(Index_t index);
    void enumProperties(afl::data::StringList_t& out);

 private:
    void sendStatus();

    Configuration m_config;
    ExportAdaptor& m_adaptor;
    util::RequestSender<ExportProxy> m_reply;
};

inline
game::proxy::ExportProxy::Trampoline::Trampoline(ExportAdaptor& adaptor, const util::RequestSender<ExportProxy>& reply)
    : m_config(),
      m_adaptor(adaptor),
      m_reply(reply)
{
    m_adaptor.initConfiguration(m_config);
}

void
game::proxy::ExportProxy::Trampoline::packStatus(Configuration& out)
{
    out = m_config;
}

void
game::proxy::ExportProxy::Trampoline::setCharsetIndex(CharsetFactory::Index_t index)
{
    m_config.setCharsetIndex(index);
    sendStatus();
}

void
game::proxy::ExportProxy::Trampoline::setFormat(interpreter::exporter::Format fmt)
{
    m_config.setFormat(fmt);
    sendStatus();
}

bool
game::proxy::ExportProxy::Trampoline::load(String_t fileName, String_t& /*errorMessage*/)
{
    Ref<Stream> s = m_adaptor.fileSystem().openFile(fileName, FileSystem::OpenRead);
    Configuration newConfig;
    newConfig.load(*s, m_adaptor.translator());
    m_config = newConfig;
    sendStatus();
    return true;
}

bool
game::proxy::ExportProxy::Trampoline::save(String_t fileName, String_t& /*errorMessage*/)
{
    Ref<Stream> s = m_adaptor.fileSystem().openFile(fileName, FileSystem::Create);
    m_config.save(*s);
    return true;
}

bool
game::proxy::ExportProxy::Trampoline::exportFile(String_t fileName, String_t& errorMessage)
{
    std::auto_ptr<Context> ctx(m_adaptor.createContext());
    if (ctx.get() != 0) {
        Ref<Stream> s = m_adaptor.fileSystem().openFile(fileName, FileSystem::Create);
        m_config.exportFile(*ctx, *s);
        return true;
    } else {
        errorMessage = m_adaptor.translator()("No data to export");
        return false;
    }
}

void
game::proxy::ExportProxy::Trampoline::add(Index_t index, String_t name, int width)
{
    m_config.fieldList().add(index, name, width);
    sendStatus();
}

void
game::proxy::ExportProxy::Trampoline::swap(Index_t a, Index_t b)
{
    m_config.fieldList().swap(a, b);
    sendStatus();
}

void
game::proxy::ExportProxy::Trampoline::remove(Index_t index)
{
    m_config.fieldList().remove(index);
    sendStatus();
}

void
game::proxy::ExportProxy::Trampoline::clear()
{
    m_config.fieldList().clear();
    sendStatus();
}

void
game::proxy::ExportProxy::Trampoline::setFieldName(Index_t index, String_t name)
{
    m_config.fieldList().setFieldName(index, name);
    sendStatus();
}

void
game::proxy::ExportProxy::Trampoline::setFieldWidth(Index_t index, int width)
{
    m_config.fieldList().setFieldWidth(index, width);
    sendStatus();
}

void
game::proxy::ExportProxy::Trampoline::changeFieldWidth(Index_t index, int delta)
{
    m_config.fieldList().changeFieldWidth(index, delta);
    sendStatus();
}

void
game::proxy::ExportProxy::Trampoline::toggleFieldAlignment(Index_t index)
{
    m_config.fieldList().toggleFieldAlignment(index);
    sendStatus();
}

inline void
game::proxy::ExportProxy::Trampoline::enumProperties(afl::data::StringList_t& out)
{
    // ex WExportFieldList::insertField (part)
    class Collector : public interpreter::PropertyAcceptor {
     public:
        Collector(afl::data::StringList_t& out)
            : m_out(out)
            { }

        virtual void addProperty(const String_t& name, interpreter::TypeHint th)
            {
                if (th != interpreter::thProcedure && th != interpreter::thFunction && th != interpreter::thArray) {
                    m_out.push_back(name);
                }
            }
     private:
        afl::data::StringList_t& m_out;
    };

    std::auto_ptr<Context> ctx(m_adaptor.createContext());
    if (ctx.get() != 0) {
        Collector coll(out);
        ctx->enumProperties(coll);
    }
}

void
game::proxy::ExportProxy::Trampoline::sendStatus()
{
    class Task : public util::Request<ExportProxy> {
     public:
        Task(Trampoline& tpl)
            { tpl.packStatus(m_status); }
        virtual void handle(ExportProxy& proxy)
            { proxy.sig_change.raise(m_status); }
     private:
        Configuration m_status;
    };
    m_reply.postNewRequest(new Task(*this));
    m_adaptor.saveConfiguration(m_config);
}


/*
 *  TrampolineFromAdaptor
 */

class game::proxy::ExportProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(ExportAdaptor&)> {
 public:
    TrampolineFromAdaptor(const util::RequestSender<ExportProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(ExportAdaptor& adaptor)
        { return new Trampoline(adaptor, m_reply); }
 private:
    util::RequestSender<ExportProxy> m_reply;
};


/*
 *  ExportProxy
 */

game::proxy::ExportProxy::ExportProxy(util::RequestSender<ExportAdaptor> adaptorSender, util::RequestDispatcher& receiver)
    : sig_change(),
      m_receiver(receiver, *this),
      m_sender(adaptorSender.makeTemporary(new TrampolineFromAdaptor(m_receiver.getSender())))
{ }

game::proxy::ExportProxy::~ExportProxy()
{ }

/*
 *  Overall Setup and Operation
 */

// Get current status.
void
game::proxy::ExportProxy::getStatus(WaitIndicator& ind, interpreter::exporter::Configuration& config)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Configuration& config)
            : m_config(config)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.packStatus(m_config); }
     private:
        Configuration& m_config;
    };
    Task t(config);
    ind.call(m_sender, t);
}

// Set character set by index.
void
game::proxy::ExportProxy::setCharsetIndex(util::CharsetFactory::Index_t index)
{
    m_sender.postRequest(&Trampoline::setCharsetIndex, index);
}

// Set format.
void
game::proxy::ExportProxy::setFormat(interpreter::exporter::Format fmt)
{
    m_sender.postRequest(&Trampoline::setFormat, fmt);
}

// Load configuration from file.
bool
game::proxy::ExportProxy::load(WaitIndicator& ind, String_t fileName, String_t& errorMessage)
{
    return callFileFunction(ind, fileName, errorMessage, &Trampoline::load);
}

// Save configuration to file.
bool
game::proxy::ExportProxy::save(WaitIndicator& ind, String_t fileName, String_t& errorMessage)
{
    return callFileFunction(ind, fileName, errorMessage, &Trampoline::save);
}

// Perform export into a file.
bool
game::proxy::ExportProxy::exportFile(WaitIndicator& ind, String_t fileName, String_t& errorMessage)
{
    return callFileFunction(ind, fileName, errorMessage, &Trampoline::exportFile);
}


/*
 *  Field List
 */

// Add field.
void
game::proxy::ExportProxy::add(Index_t index, String_t name, int width)
{
    m_sender.postRequest(&Trampoline::add, index, name, width);
}

// Swap fields.
void
game::proxy::ExportProxy::swap(Index_t a, Index_t b)
{
    m_sender.postRequest(&Trampoline::swap, a, b);
}

// Delete a field.
void
game::proxy::ExportProxy::remove(Index_t index)
{
    m_sender.postRequest(&Trampoline::remove, index);
}

// Clear the list.
void
game::proxy::ExportProxy::clear()
{
    m_sender.postRequest(&Trampoline::clear);
}

// Change field name.
void
game::proxy::ExportProxy::setFieldName(Index_t index, String_t name)
{
    m_sender.postRequest(&Trampoline::setFieldName, index, name);
}

// Change width of a field.
void
game::proxy::ExportProxy::setFieldWidth(Index_t index, int width)
{
    m_sender.postRequest(&Trampoline::setFieldWidth, index, width);
}

// Change width of a field, relative.
void
game::proxy::ExportProxy::changeFieldWidth(Index_t index, int delta)
{
    m_sender.postRequest(&Trampoline::changeFieldWidth, index, delta);
}

// Toggle field's alignment.
void
game::proxy::ExportProxy::toggleFieldAlignment(Index_t index)
{
    m_sender.postRequest(&Trampoline::toggleFieldAlignment, index);
}


/*
 *  Adding Fields
 */

// Retrieve list of properties.
void
game::proxy::ExportProxy::enumProperties(WaitIndicator& ind, afl::data::StringList_t& out)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(afl::data::StringList_t& out)
            : m_out(out)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.enumProperties(m_out); }
     private:
        afl::data::StringList_t& m_out;
    };
    Task t(out);
    ind.call(m_sender, t);
}

// Shortcut for calling a synchronous file-access function.
bool
game::proxy::ExportProxy::callFileFunction(WaitIndicator& ind, String_t fileName, String_t& errorMessage, FileFunction_t fcn)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(String_t fileName, String_t& errorMessage, FileFunction_t fcn)
            : m_fileName(fileName), m_errorMessage(errorMessage), m_function(fcn), m_result(false)
            { }
        virtual void handle(Trampoline& tpl)
            {
                try {
                    m_result = (tpl.*m_function)(m_fileName, m_errorMessage);
                }
                catch (afl::except::FileProblemException& e) {
                    m_errorMessage = Format("%s: %s", e.getFileName(), e.what());
                    m_result = false;
                }
                catch (std::exception& e) {
                    m_errorMessage = e.what();
                    m_result = false;
                }
            }
        bool getResult() const
            { return m_result; }
     private:
        String_t m_fileName;
        String_t& m_errorMessage;
        FileFunction_t m_function;
        bool m_result;
    };

    Task t(fileName, errorMessage, fcn);
    ind.call(m_sender, t);
    return t.getResult();
}
