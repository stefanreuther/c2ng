/**
  *  \file game/proxy/attachmentproxy.cpp
  *  \brief Class game::proxy::AttachmentProxy
  */

#include <memory>
#include "game/proxy/attachmentproxy.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/v3/attachmentconfiguration.hpp"
#include "game/v3/attachmentunpacker.hpp"


/*
 *  Trampoline
 */

class game::proxy::AttachmentProxy::Trampoline : private afl::sys::LogListener {
 public:
    Trampoline(const util::RequestSender<AttachmentProxy>& reply, MaintenanceAdaptor& ad)
        : m_reply(reply),
          m_adaptor(ad),
          m_unpacker()
        { }

    void loadDirectory(PlayerSet_t& players, bool autoSelect, Infos_t& result, bool& proceed)
        {
            // Load
            m_unpacker.reset(new game::v3::AttachmentUnpacker());
            for (int i = 1; i <= MAX_PLAYERS; ++i) {
                if (players.contains(i)) {
                    m_unpacker->loadDirectory(m_adaptor.targetDirectory(), i, *this, m_adaptor.translator());
                }
            }
            proceed = false;

            // Postprocess
            m_unpacker->dropUnchangedFiles(m_adaptor.targetDirectory(), *this, m_adaptor.translator());
            if (autoSelect) {
                proceed = checkNewAttachments(m_adaptor.userConfiguration(), *m_unpacker);
                m_unpacker->dropUnselectedAttachments();
            }

            // Generate output
            for (size_t i = 0, n = m_unpacker->getNumAttachments(); i < n; ++i) {
                game::v3::AttachmentUnpacker::Attachment* att = m_unpacker->getAttachmentByIndex(i);
                result.push_back(AttachmentProxy::Info(m_unpacker->getAttachmentName(att),
                                                       game::v3::AttachmentUnpacker::toString(m_unpacker->getAttachmentKind(att), m_adaptor.translator()),
                                                       m_unpacker->getAttachmentSize(att),
                                                       m_unpacker->isAttachmentSelected(att),
                                                       m_unpacker->getAttachmentKind(att) == game::v3::AttachmentUnpacker::CriticalFile));
            }
        }

    void selectAttachment(String_t name, bool enable)
        {
            if (m_unpacker.get() != 0) {
                m_unpacker->selectAttachment(m_unpacker->getAttachmentByName(name), enable);
            }
        }

    void saveFiles()
        {
            if (m_unpacker.get() != 0) {
                try {
                    m_unpacker->saveFiles(m_adaptor.targetDirectory(), *this, m_adaptor.translator());
                    markAttachmentsProcessed(m_adaptor.userConfiguration(), *m_unpacker);
                    m_unpacker.reset();
                }
                catch (std::exception& e) {
                    // This should not happen, but if it does, make sure we still reach the emitActionComplete()
                    // because UI side will be waiting for it.
                    m_reply.postRequest(&AttachmentProxy::emitMessage, m_adaptor.translator()("Unexpected error"));
                }
            }
            emitActionComplete();
        }

 private:
    util::RequestSender<AttachmentProxy> m_reply;
    MaintenanceAdaptor& m_adaptor;
    std::auto_ptr<game::v3::AttachmentUnpacker> m_unpacker;

    void emitActionComplete()
        {
            m_reply.postRequest(&AttachmentProxy::emitActionComplete);
        }

    virtual void handleMessage(const Message& msg)
        {
            m_reply.postRequest(&AttachmentProxy::emitMessage, msg.m_message);
        }
};


/*
 *  TrampolineFromAdaptor
 */

class game::proxy::AttachmentProxy::TrampolineFromAdaptor : public afl::base::Closure<Trampoline*(MaintenanceAdaptor&)> {
 public:
    TrampolineFromAdaptor(const util::RequestSender<AttachmentProxy>& reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(MaintenanceAdaptor& ad)
        { return new Trampoline(m_reply, ad); }
 private:
    util::RequestSender<AttachmentProxy> m_reply;
};


/*
 *  AttachmentProxy
 */

game::proxy::AttachmentProxy::AttachmentProxy(util::RequestSender<MaintenanceAdaptor> sender, util::RequestDispatcher& reply)
    : m_receiver(reply, *this),
      m_sender(sender.makeTemporary(new TrampolineFromAdaptor(m_receiver.getSender())))
{ }

game::proxy::AttachmentProxy::~AttachmentProxy()
{ }

void
game::proxy::AttachmentProxy::loadDirectory(WaitIndicator& ind, PlayerSet_t players, bool autoSelect, Infos_t& result, bool& proceed)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(PlayerSet_t players, bool autoSelect, Infos_t& result, bool& proceed)
            : m_players(players), m_autoSelect(autoSelect), m_result(result), m_proceed(proceed)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.loadDirectory(m_players, m_autoSelect, m_result, m_proceed); }
     private:
        PlayerSet_t m_players;
        bool m_autoSelect;
        Infos_t& m_result;
        bool& m_proceed;
    };
    Task t(players, autoSelect, result, proceed);
    ind.call(m_sender, t);
}

void
game::proxy::AttachmentProxy::selectAttachment(const String_t& name, bool enable)
{
    m_sender.postRequest(&Trampoline::selectAttachment, name, enable);
}

void
game::proxy::AttachmentProxy::selectAttachments(const Infos_t& infos)
{
    // This is an O(n^2) operation, but our n is usually small.
    for (size_t i = 0, n = infos.size(); i < n; ++i) {
        selectAttachment(infos[i].fileName, infos[i].selected);
    }
}

void
game::proxy::AttachmentProxy::saveFiles()
{
    m_sender.postRequest(&Trampoline::saveFiles);
}

void
game::proxy::AttachmentProxy::emitActionComplete()
{
    sig_actionComplete.raise();
}

void
game::proxy::AttachmentProxy::emitMessage(String_t msg)
{
    sig_message.raise(msg);
}
