/**
  *  \file client/tiles/shipmovementtile.hpp
  */
#ifndef C2NG_CLIENT_TILES_SHIPMOVEMENTTILE_HPP
#define C2NG_CLIENT_TILES_SHIPMOVEMENTTILE_HPP

#include "afl/string/translator.hpp"
#include "client/widgets/collapsibledataview.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "game/proxy/objectobserver.hpp"
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
            bool hasExplanation;

            Data()
                : fleetStatus(ui::NoFrame), hasExplanation(false)
                {
                    afl::base::Memory<util::SkinColor::Color>(colors).fill(util::SkinColor::Green);
                }
        };

        ShipMovementTile(ui::Root& root, afl::string::Translator& tx, gfx::KeyEventConsumer& kmw);
        void attach(game::proxy::ObjectObserver& oop);

        void setData(const Data& data);

        virtual void setChildPositions();
        virtual gfx::Point getPreferredChildSize() const;

     private:
        class Job;

        afl::string::Translator& m_translator;
        ui::widgets::SimpleTable m_table;
        ui::widgets::Button m_warpButton;
        ui::widgets::Button m_chartButton;
        ui::widgets::Button m_queryButton;
        ui::widgets::Button m_fleetButton;
        ui::widgets::FrameGroup m_fleetFrame;
        util::RequestReceiver<ShipMovementTile> m_receiver;

        void init(gfx::KeyEventConsumer& kmw);
    };

} }

#endif
