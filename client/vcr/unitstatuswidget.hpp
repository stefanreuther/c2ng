/**
  *  \file client/vcr/unitstatuswidget.hpp
  */
#ifndef C2NG_CLIENT_VCR_UNITSTATUSWIDGET_HPP
#define C2NG_CLIENT_VCR_UNITSTATUSWIDGET_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "game/teamsettings.hpp"
#include "gfx/context.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace client { namespace vcr {

    class UnitStatusWidget : public ui::SimpleWidget {
     public:
        struct Data {
            String_t unitName;
            String_t ownerName;
            String_t beamName;
            String_t launcherName;
            String_t unitImageName;
            int numBeams;
            int numLaunchers;
            int numBays;
            game::TeamSettings::Relation relation;
            bool isPlanet;
            Data()
                : unitName(), ownerName(), beamName(), launcherName(), unitImageName(),
                  numBeams(0), numLaunchers(0), numBays(0), relation(), isPlanet(false)
                { }
        };

        struct WeaponStatus {
            int displayed;
            int actual;
            bool blocked;
            WeaponStatus()
                : displayed(0), actual(0), blocked(false)
                { }
        };

        struct Status {
            int numFighters;
            int numTorpedoes;
            int shield;
            int damage;
            int crew;
            std::vector<WeaponStatus> launcherStatus;
            std::vector<WeaponStatus> beamStatus;
            Status()
                : numFighters(0), numTorpedoes(0), shield(0), damage(0), crew(0),
                  launcherStatus(), beamStatus()
                { }
        };

        enum Property {
            NumFighters,
            NumTorpedoes,
            Shield,
            Damage,
            Crew
        };
        enum Weapon {
            Launcher,
            Beam
        };

        UnitStatusWidget(ui::Root& root, afl::string::Translator& tx);

        void setData(const Data& data);
        void setProperty(Property p, int value);
        void addProperty(Property p, int delta);

        void setWeaponLevel(Weapon w, int slot, int value);
        void setWeaponStatus(Weapon w, int slot, bool blocked);

        void unblockAllWeapons();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        Data m_data;
        Status m_status;
        afl::base::Ptr<gfx::Canvas> m_image;
        afl::base::SignalConnection conn_imageChange;

        void onImageChange();

        void drawMainColumn(gfx::Canvas& can, gfx::Rectangle r);
        void drawWeaponColumn(gfx::Canvas& can, gfx::Rectangle r);
        void drawWeaponBar(gfx::Context<uint8_t>& ctx, gfx::Rectangle r, int level);

        int* getProperty(Property p);
        WeaponStatus* getWeapon(Weapon w, int slot);

        static bool unblockWeapons(std::vector<WeaponStatus>& w);
    };

} }

#endif
