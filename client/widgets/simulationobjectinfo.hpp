/**
  *  \file client/widgets/simulationobjectinfo.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_SIMULATIONOBJECTINFO_HPP
#define C2NG_CLIENT_WIDGETS_SIMULATIONOBJECTINFO_HPP

#include <vector>
#include "afl/base/deleter.hpp"
#include "afl/string/translator.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "ui/cardgroup.hpp"
#include "ui/root.hpp"

namespace client { namespace widgets {

    class SimulationObjectInfo : public ui::CardGroup {
     public:
        typedef game::proxy::SimulationSetupProxy::ObjectInfo ObjectInfo_t;

        SimulationObjectInfo(ui::Root& root, ui::Widget& keyHandler, afl::string::Translator& tx);
        ~SimulationObjectInfo();

        void setContent(const ObjectInfo_t& info);
        void clearContent();
        void showIntroPage();

     private:
        class Child {
         public:
            virtual void setContent(const ObjectInfo_t& info) = 0;
        };

        class Header;
        class ShipWeapons;
        class ShipDetails;
        class BaseInfo;
        class Footer;

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        afl::base::Deleter m_deleter;

        std::vector<Child*> m_children;
        ui::Widget* m_emptyPage;
        ui::Widget* m_planetPage;
        ui::Widget* m_shipPage;
        ui::Widget* m_introPage;

        void init(ui::Widget& keyHandler);
    };

} }


#endif
