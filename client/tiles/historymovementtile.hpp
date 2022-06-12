/**
  *  \file client/tiles/historymovementtile.hpp
  *  \brief Class client::tiles::HistoryMovementTile
  */
#ifndef C2NG_CLIENT_TILES_HISTORYMOVEMENTTILE_HPP
#define C2NG_CLIENT_TILES_HISTORYMOVEMENTTILE_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "client/tiles/historyadaptor.hpp"
#include "client/widgets/collapsibledataview.hpp"
#include "ui/widgets/simpletable.hpp"

namespace client { namespace tiles {

    /** History movement tile.
        Displays the movement information for the turn currently selected on a HistoryAdaptor. */
    class HistoryMovementTile : public client::widgets::CollapsibleDataView {
     public:
        /** Constructor.
            @param root User-interface root
            @param tx   Translator */
        HistoryMovementTile(ui::Root& root, afl::string::Translator& tx);

        /** Attach HistoryAdaptor.
            Connects events to render automatically.
            @param adaptor HistoryAdaptor */
        void attach(HistoryAdaptor& adaptor);

        // CollapsibleDataView:
        virtual void setChildPositions();
        virtual gfx::Point getPreferredChildSize() const;

     private:
        afl::string::Translator& m_translator;
        ui::widgets::SimpleTable m_table;
        afl::base::SignalConnection conn_turnChange;

        void init();
        void onTurnChange(HistoryAdaptor& adaptor);
    };

} }

#endif
