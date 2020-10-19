/**
  *  \file client/tiles/shipmovementtile.hpp
  */
#ifndef C2NG_CLIENT_TILES_SHIPMOVEMENTTILE_HPP
#define C2NG_CLIENT_TILES_SHIPMOVEMENTTILE_HPP

#include "game/proxy/objectobserver.hpp"
#include "client/widgets/collapsibledataview.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/simpletable.hpp"
#include "util/requestreceiver.hpp"
#include "util/skincolor.hpp"

namespace client { namespace tiles {

    class ShipMovementTile : public client::widgets::CollapsibleDataView {
     public:
        struct Data {
            enum { Location, Waypoint, Distance, WarpFactor, Eta, FuelUsage, EngineLoad };
            String_t text[7];
            util::SkinColor::Color colors[7];
            ui::FrameType fleetStatus;

            Data()
                {
                    afl::base::Memory<util::SkinColor::Color>(colors).fill(util::SkinColor::Green);
                    fleetStatus = ui::NoFrame;
                }
        };

        ShipMovementTile(ui::Root& root, client::widgets::KeymapWidget& kmw);
        void attach(game::proxy::ObjectObserver& oop);

        void setData(const Data& data);

        virtual void setChildPositions();
        virtual gfx::Point getPreferredChildSize() const;

     private:
        class Job;

        ui::widgets::SimpleTable m_table;
        ui::widgets::Button m_warpButton;
        ui::widgets::Button m_chartButton;
        ui::widgets::Button m_queryButton;
        ui::widgets::Button m_fleetButton;
        ui::widgets::FrameGroup m_fleetFrame;
        util::RequestReceiver<ShipMovementTile> m_receiver;

        void init(client::widgets::KeymapWidget& kmw);
    };

} }

#endif
