/**
  *  \file client/widgets/costdisplay.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_COSTDISPLAY_HPP
#define C2NG_CLIENT_WIDGETS_COSTDISPLAY_HPP

#include "ui/widgets/simpletable.hpp"
#include "ui/root.hpp"
#include "game/spec/cost.hpp"
#include "afl/bits/smallset.hpp"
#include "util/numberformatter.hpp"

namespace client { namespace widgets {

    class CostDisplay : public ui::widgets::SimpleTable {
     public:
        typedef game::spec::Cost::Type Type_t;
        typedef afl::bits::SmallSet<Type_t> Types_t;
        
        CostDisplay(ui::Root& root, Types_t types, util::NumberFormatter fmt);

        void setCost(const game::spec::Cost& cost);
        void setAvailableAmount(const game::spec::Cost& amount);
        void setRemainingAmount(const game::spec::Cost& amount);
        void setMissingAmount(const game::spec::Cost& amount);

     private:
        void init(ui::Root& root);
        void render();

        Types_t m_types;
        util::NumberFormatter m_formatter;
        game::spec::Cost m_cost;
        game::spec::Cost m_availableAmount;
        game::spec::Cost m_remainingAmount;
        game::spec::Cost m_missingAmount;
    };

} }

#endif
