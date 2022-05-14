/**
  *  \file client/widgets/chartdisplayconfig.hpp
  *  \brief Class client::widgets::ChartDisplayConfig
  */
#ifndef C2NG_CLIENT_WIDGETS_CHARTDISPLAYCONFIG_HPP
#define C2NG_CLIENT_WIDGETS_CHARTDISPLAYCONFIG_HPP

#include "afl/base/deleter.hpp"
#include "afl/string/translator.hpp"
#include "game/map/renderoptions.hpp"
#include "ui/icons/icon.hpp"
#include "ui/root.hpp"
#include "ui/widgets/treelistbox.hpp"

namespace client { namespace widgets {

    /** Starchart display configuration widget.
        Shows rendering options for all modes in a tree and lets the user toggle them.

        To use,
        - create
        - set current configuration using set() for each area
        - when user confirms, query updated configuration using get() and store it in config file */
    class ChartDisplayConfig : public ui::widgets::TreeListbox {
     public:
        /** Possible type of a checkbox.
            Internal, but must be public to use for array dimensions in implementation. */
        enum Value {
            Unchecked, Checked, Filled, Inside, Mixed
        };
        static const size_t NUM_VALUES = static_cast<size_t>(Mixed)+1;

        /** Constructor.
            @param root UI root
            @param tx   Translator */
        ChartDisplayConfig(ui::Root& root, afl::string::Translator& tx);
        ~ChartDisplayConfig();

        /** Set current values.
            @param area Area to use
            @param opts Options fot that area */
        void set(game::map::RenderOptions::Area area, const game::map::RenderOptions& opts);

        /** Get current values.
            @param area Area to query
            @return New options */
        game::map::RenderOptions get(game::map::RenderOptions::Area area) const;

     private:
        void init(afl::string::Translator& tx);
        void render();
        Value getValue(int32_t id) const;

        void onIconClick(int32_t id);
        void onImageChange();

        ui::Root& m_root;
        afl::base::Deleter m_deleter;
        game::map::RenderOptions m_options[game::map::RenderOptions::NUM_AREAS];
        ui::icons::Icon* m_icons[NUM_VALUES];
        afl::base::SignalConnection conn_imageChange;
    };

} }

#endif
