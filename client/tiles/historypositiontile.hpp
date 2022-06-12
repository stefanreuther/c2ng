/**
  *  \file client/tiles/historypositiontile.hpp
  *  \brief Class client::tiles::HistoryPositionTile
  */
#ifndef C2NG_CLIENT_TILES_HISTORYPOSITIONTILE_HPP
#define C2NG_CLIENT_TILES_HISTORYPOSITIONTILE_HPP

#include "afl/base/deleter.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "client/widgets/historypositionlistbox.hpp"
#include "ui/group.hpp"
#include "ui/root.hpp"
#include "ui/skincolorscheme.hpp"

namespace client { namespace tiles {

    class HistoryAdaptor;

    /** History position tile.
        Works together with HistoryAdaptor to display a list of history positions,
        and forwards the currently-selected line to the HistoryAdaptor.
        To use,
        - construct
        - call attach() to connect events */
    class HistoryPositionTile : public ui::Group {
     public:
        /** Constructor.
            @param root User-interface root
            @param tx   Translator */
        HistoryPositionTile(ui::Root& root, afl::string::Translator& tx);

        /** Destructor. */
        ~HistoryPositionTile();

        /** Attach HistoryAdaptor.
            @param adaptor HistoryAdaptor instance; must live longer than HistoryPositionTile */
        void attach(HistoryAdaptor& adaptor);

     private:
        afl::base::Deleter m_deleter;
        ui::SkinColorScheme m_internalColorScheme;
        client::widgets::HistoryPositionListbox m_list;

        afl::base::SignalConnection conn_listChange;
        afl::base::SignalConnection conn_listScroll;

        void onListChange(const HistoryAdaptor& adaptor);
        void onListScroll(HistoryAdaptor& adaptor);
    };

} }

#endif
