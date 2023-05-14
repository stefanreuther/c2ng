/**
  *  \file client/dialogs/cargotransferdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_CARGOTRANSFERDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_CARGOTRANSFERDIALOG_HPP

#include "afl/base/observable.hpp"
#include "afl/string/translator.hpp"
#include "client/widgets/cargotransferline.hpp"
#include "game/element.hpp"
#include "game/proxy/cargotransferproxy.hpp"
#include "game/session.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractcheckbox.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "util/requestsender.hpp"
#include "util/vector.hpp"

namespace client { namespace dialogs {

    class CargoTransferDialog {
     public:
        CargoTransferDialog(ui::Root& root, afl::string::Translator& tx, game::proxy::CargoTransferProxy& proxy);

        bool run(util::RequestSender<game::Session> gameSender);

     private:
        struct AddHelper;

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        game::proxy::CargoTransferProxy& m_proxy;
        game::proxy::CargoTransferProxy::Cargo m_cargo[2];
        ui::EventLoop m_loop;
        util::Vector<client::widgets::CargoTransferLine*, game::Element::Type> m_lines;
        afl::base::Observable<int> m_sellSupplies;
        ui::widgets::AbstractCheckbox m_overloadCheckbox;
        bool m_overload;

        void onMove(game::Element::Type id, bool target, int amount);
        void onLoadAmount(game::Element::Type id, bool target, int amount);
        void onUnload();
        void onChange(size_t side, const game::proxy::CargoTransferProxy::Cargo& cargo);
        void onEnableOverload();

        void addCargoTransferLine(game::Element::Type type,
                                  const AddHelper& helper,
                                  ui::Group& lineGroup,
                                  ui::widgets::FocusIterator& iter,
                                  afl::base::Deleter& del);
    };

} }

#endif
