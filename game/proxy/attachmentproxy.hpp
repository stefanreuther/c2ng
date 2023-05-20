/**
  *  \file game/proxy/attachmentproxy.hpp
  *  \brief Class game::proxy::AttachmentProxy
  */
#ifndef C2NG_GAME_PROXY_ATTACHMENTPROXY_HPP
#define C2NG_GAME_PROXY_ATTACHMENTPROXY_HPP

#include <vector>
#include "game/proxy/maintenanceadaptor.hpp"
#include "util/requestdispatcher.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Attachment Reception Proxy.
        This bidirectional proxy allows reception of result file attachments.

        This proxy is modelled after MaintenanceProxy, and has the same requirements:
        - prepare an operation using loadDirectory();
        - optionally, configure using selectAttachment();
        - execute operation using saveFiles().

        Completion of saveFiles() will be signalled using sig_actionComplete.

        Attachment unpacking operations will not log to a system console;
        instead, they will produce messages using sig_message.
        User-interface shall render those to the player.
        AttachmentProxy will generate error messages during loadDirectory(),
        and status messages during saveFiles(). */
    class AttachmentProxy {
     public:
        /** Information about an attachment. */
        struct Info {
            String_t fileName;           ///< File name. Serves as identifier for the attachment.
            String_t kindName;           ///< File kind as string. @see game::v3::AttachmentUnpacker::toString().
            size_t size;                 ///< File size in bytes.
            bool selected;               ///< true if attachment is selected for reception.
            bool critical;               ///< true if this is a critical file.
            Info(const String_t& fileName, const String_t& kindName, size_t size, bool selected, bool critical)
                : fileName(fileName), kindName(kindName), size(size), selected(selected), critical(critical)
                { }
        };
        typedef std::vector<Info> Infos_t;

        /** Constructor.
            @param sender Sender
            @param reply  RequestDispatcher to receive updates in this thread */
        AttachmentProxy(util::RequestSender<MaintenanceAdaptor> sender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~AttachmentProxy();

        /** Load directory content (prepare).
            @param [in]  ind        WaitIndicator for UI synchronisation
            @param [in]  players    Set of players to check
            @param [in]  autoSelect If true, automatically select all files accepted by user configuration,
                                    and drop all others (for "unattended" operation).
                                    If false, just normal operation.
            @param [out] result     Result is produced here
            @param [out] proceed    If true, suggest to proceed silently; otherwise, ask user.
            @see game::v3::AttachmentUnpacker::loadDirectory(), game::v3::checkNewAttachments() */
        void loadDirectory(WaitIndicator& ind, PlayerSet_t players, bool autoSelect, Infos_t& result, bool& proceed);

        /** Select or unselect attachment for reception (configure).
            Identifies the attachment by its name.
            If the name is invalid, the call is ignored.
            @param name   File name
            @param enable true to receive, false to skip
            @see game::v3::AttachmentUnpacker::selectAttachment() */
        void selectAttachment(const String_t& name, bool enable);

        /** Select or unselect attachments, according to given Infos_t.
            The given list need not be identical ot the one retrieved using loadDirectory().
            @param infos List of file name/status
            @see selectAttachment()
            @see game::v3::AttachmentUnpacker::selectAttachment() */
        void selectAttachments(const Infos_t& infos);

        /** Save files (exxecute/finish).
            The operation will execute in the game thread;
            its completion will be signalled using sig_actionComplete. */
        void saveFiles();

        /** Signal: action complete. */
        afl::base::Signal<void()> sig_actionComplete;

        /** Signal: status message.
            @param message Message text */
        afl::base::Signal<void(String_t)> sig_message;

     private:
        class Trampoline;
        class TrampolineFromAdaptor;

        util::RequestReceiver<AttachmentProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;

        void emitActionComplete();
        void emitMessage(String_t msg);
    };

} }

#endif
