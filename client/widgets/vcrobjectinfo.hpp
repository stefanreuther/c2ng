/**
  *  \file client/widgets/vcrobjectinfo.hpp
  *  \brief Class client::widgets::VcrObjectInfo
  */
#ifndef C2NG_CLIENT_WIDGETS_VCROBJECTINFO_HPP
#define C2NG_CLIENT_WIDGETS_VCROBJECTINFO_HPP

#include "game/vcr/objectinfo.hpp"
#include "ui/rich/documentview.hpp"
#include "util/numberformatter.hpp"

namespace client { namespace widgets {

    class VcrObjectInfo : public ui::rich::DocumentView {
     public:
        VcrObjectInfo(bool fullInfo, util::NumberFormatter fmt, afl::string::Translator& tx, gfx::ResourceProvider& provider);
        ~VcrObjectInfo();

        void setShipInfo(const game::vcr::ShipInfo& info);
        void setPlanetInfo(const game::vcr::PlanetInfo& info);
        void clear();

     private:
        bool m_fullInfo;
        util::NumberFormatter m_formatter;
        afl::string::Translator& m_translator;
        gfx::ResourceProvider& m_provider;
    };

} }

#endif
