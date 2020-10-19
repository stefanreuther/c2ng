/**
  *  \file client/tiles/shipcargotile.hpp
  */
#ifndef C2NG_CLIENT_TILES_SHIPCARGOTILE_HPP
#define C2NG_CLIENT_TILES_SHIPCARGOTILE_HPP

#include "game/proxy/objectobserver.hpp"
#include "client/widgets/collapsibledataview.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "client/widgets/simplegauge.hpp"
#include "ui/widgets/simpletable.hpp"
#include "util/requestreceiver.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/textbutton.hpp"

namespace client { namespace tiles {

    class ShipCargoTile : public client::widgets::CollapsibleDataView {
     public:
        struct Data {
            enum { Neutronium, Tritanium, Duranium, Molybdenum, Colonists, Supplies, Money, TotalMass };
            String_t formattedAmounts[8];

            enum { FuelGauge, CargoGauge };
            String_t gaugeLabels[2];
            int gaugeHave[2];
            int gaugeTotal[2];

            enum Transfer { NoTransfer, ShipTransfer, UnloadTransfer, GatherTransfer, BidiTransfer, JettisonTransfer };
            Transfer unloadReview;
            Transfer transferReview;

            Data()
                {
                    gaugeHave[0] = gaugeHave[1] = gaugeTotal[0] = gaugeTotal[1] = 0;
                    unloadReview = transferReview = NoTransfer;
                }
        };

        ShipCargoTile(ui::Root& root, client::widgets::KeymapWidget& kmw);
        void attach(game::proxy::ObjectObserver& oop);

        void setData(const Data& data);

        virtual void setChildPositions();
        virtual gfx::Point getPreferredChildSize() const;

     private:
        class Job;

        ui::widgets::SimpleTable m_table;
        client::widgets::SimpleGauge m_fuelGauge;
        client::widgets::SimpleGauge m_cargoGauge;
        ui::widgets::Button m_cargoButton;
        ui::widgets::Button m_unloadButton;
        ui::widgets::TextButton m_reviewUnloadButton;
        ui::widgets::TextButton m_reviewTransferButton;

        util::RequestReceiver<ShipCargoTile> m_receiver;

        void init(ui::Root& root, client::widgets::KeymapWidget& kmw);
        void setGaugeData(client::widgets::SimpleGauge& gauge, const Data& data, size_t index);
        void setReviewData(ui::widgets::TextButton& button, Data::Transfer t);
    };

} }

#endif
