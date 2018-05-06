/**
  *  \file game/v3/check/configuration.hpp
  */
#ifndef C2NG_GAME_V3_CHECK_CONFIGURATION_HPP
#define C2NG_GAME_V3_CHECK_CONFIGURATION_HPP

namespace game { namespace v3 { namespace check {

    class Configuration {
     public:
        Configuration();

        void setResultMode(bool enable);
        bool isResultMode() const;

        void setHtmlMode(bool enable);
        bool isHtmlMode() const;

        void setChecksumsMode(bool enable);
        bool isChecksumsMode() const;

        void setHandleMinus1Special(bool enable);
        bool isHandleMinus1Special() const;

        void setPickyMode(bool enable);
        bool isPickyMode() const;

     private:
        bool m_resultMode;
        bool m_htmlMode;
        bool m_checksumsMode;
        bool m_handleMinus1Special;
        bool m_pickyMode;
    };

} } }

#endif
