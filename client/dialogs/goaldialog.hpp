/**
  *  \file client/dialogs/goaldialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_GOALDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_GOALDIALOG_HPP

#include "game/types.hpp"
#include "ui/root.hpp"
#include "afl/base/deleter.hpp"
#include "afl/string/translator.hpp"
#include "ui/eventloop.hpp"
#include "game/map/planet.hpp"

namespace client { namespace dialogs {

    class GoalDialog {
     public:
        GoalDialog(ui::Root& root, afl::string::Translator& tx, bool allowUnchanged, ui::Widget* pHelp);
        ~GoalDialog();

        void setGoal(game::PlanetaryBuilding building, int goal);
        void setSpeed(game::PlanetaryBuilding building, int speed);

        int getGoal(game::PlanetaryBuilding building) const;
        int getSpeed(game::PlanetaryBuilding building) const;
        game::map::Planet::AutobuildSettings getResult() const;

        void setFocusToStructure(game::PlanetaryBuilding building);

        bool run();

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        afl::base::Deleter m_deleter;
        ui::EventLoop m_loop;
        bool m_allowUnchanged;

        class InputComponent;
        InputComponent* m_goalInputs[game::NUM_PLANETARY_BUILDING_TYPES];
        InputComponent* m_speedInputs[game::NUM_PLANETARY_BUILDING_TYPES];
        ui::Widget* m_pWindow;

        void init(ui::Widget* pHelp);
    };

} }

#endif
