/**
  *  \file client/widgets/chartmouseconfig.hpp
  *  \brief Class client::widgets::ChartMouseConfig
  */
#ifndef C2NG_CLIENT_WIDGETS_CHARTMOUSECONFIG_HPP
#define C2NG_CLIENT_WIDGETS_CHARTMOUSECONFIG_HPP

#include "afl/base/deleter.hpp"
#include "afl/string/translator.hpp"
#include "game/map/renderoptions.hpp"
#include "ui/icons/icon.hpp"
#include "ui/root.hpp"
#include "ui/widgets/treelistbox.hpp"

namespace client { namespace widgets {

    /** Mouse configuration widget.
        Displays "lock mode" and "wheel" configuration options in a tree and lets the user toggle them.

        To use,
        - create
        - set current configuration using set()
        - when user confirms, query updated configuration using get() and store it in config file */
    class ChartMouseConfig : public ui::widgets::TreeListbox {
     public:
        /** Possible type of a checkbox.
            Internal, but must be public to use for array dimensions in implementation. */
        enum Value {
            Unchecked, Checked,
            Unselected, Selected,
            None
        };
        static const size_t NUM_VALUES = static_cast<size_t>(None)+1;

        /** Constructor.
            @param root UI root
            @param tx   Translator */
        ChartMouseConfig(ui::Root& root, afl::string::Translator& tx);
        ~ChartMouseConfig();

        /** Set current values.
            @param leftLock  Left mouse button lock configuration (UserConfiguration::Lock_Left, MatchXxx)
            @param rightLock Right mouse button lock configuration (UserConfiguration::Lock_Right, MatchXxx)
            @param wheelMode Wheel mode configuration (UserConfiguration::ChartWheel, UserConfiguration::WheelMode) */
        void set(int leftLock, int rightLock, int wheelMode);

        /** Get left mouse button lock configuration.
            @return modes (MatchXxx) */
        int getLeftLock() const;

        /** Get right mouse button lock configuration.
            @return modes (MatchXxxx) */
        int getRightLock() const;

        /** Get wheel mode configuration.
            @return mode (corresponding to a UserConfiguration::WheelMode value) */
        int getWheelMode() const;

     private:
        void init(afl::string::Translator& tx);
        void render();
        Value getValue(int32_t id) const;

        void onIconClick(int32_t id);
        void onImageChange();

        ui::Root& m_root;
        afl::base::Deleter m_deleter;
        int m_leftLock;
        int m_rightLock;
        int m_wheelMode;
        ui::icons::Icon* m_icons[NUM_VALUES];
        afl::base::SignalConnection conn_imageChange;
    };

} }

#endif
