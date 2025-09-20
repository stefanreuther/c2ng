/**
  *  \file client/widgets/configstoragecontrol.hpp
  *  \brief Class client::widgets::ConfigStorageControl
  */
#ifndef C2NG_CLIENT_WIDGETS_CONFIGSTORAGECONTROL_HPP
#define C2NG_CLIENT_WIDGETS_CONFIGSTORAGECONTROL_HPP

#include "afl/base/signal.hpp"
#include "game/config/configurationeditor.hpp"
#include "game/config/configurationoption.hpp"
#include "ui/group.hpp"
#include "ui/widgets/optiongrid.hpp"

namespace client { namespace widgets {

    /** Configuration storage control.
        Displays a ConfigurationEditor::Source (current storage location) in an OptionGrid widget.
        When used, emits a sig_change with a ConfigurationOption::Source for a new location.

        @change In PCC2, the widget observes and manipulates the configuration directly.
        In c2ng, the access (in particular, combination of storage locations for multi-value options)
        is done by ConfigurationEditor/ConfigurationEditorProxy. */
    class ConfigStorageControl : public ui::Group {
     public:
        /** Constructor.
            @param root UI root (for colors, fonts)
            @param tx   Translator (for storage location names) */
        ConfigStorageControl(ui::Root& root, afl::string::Translator& tx);

        /** Destructor. */
        ~ConfigStorageControl();

        /** Set source (storage location).
            Selects the value to display.
            @param source Source */
        void setSource(game::config::ConfigurationEditor::Source source);

        /** Signal: new location chosen.
            @param source New storage location */
        afl::base::Signal<void(game::config::ConfigurationOption::Source)> sig_change;

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::widgets::OptionGrid m_grid;
        game::config::ConfigurationEditor::Source m_source;

        void init();
        void render();

        void onButtonClick(int id);
    };

} }

#endif
