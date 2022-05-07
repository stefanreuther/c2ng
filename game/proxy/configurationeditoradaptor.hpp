/**
  *  \file game/proxy/configurationeditoradaptor.hpp
  *  \brief Interface game::proxy::ConfigurationEditorAdaptor
  */
#ifndef C2NG_GAME_PROXY_CONFIGURATIONEDITORADAPTOR_HPP
#define C2NG_GAME_PROXY_CONFIGURATIONEDITORADAPTOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/translator.hpp"
#include "game/config/configuration.hpp"
#include "game/config/configurationeditor.hpp"

namespace game { namespace proxy {

    /** Adaptor for ConfigurationEditorProxy.
        The ConfigurationEditorProxy contains a prepared ConfigurationEditor object
        with the options being edited. */
    class ConfigurationEditorAdaptor : public afl::base::Deletable {
     public:
        /** Access configuration object.
            This method typically returns the desired subobject of a session.
            @return configuration object; must live as long as the ConfigurationEditorAdaptor */
        virtual game::config::Configuration& config() = 0;

        /** Access ConfigurationEditor object.
            This method typically returns a ConfigurationEditor created in the adaptor's constructor.
            @return ConfigurationEditor object; must live as long as the ConfigurationEditorAdaptor */
        virtual game::config::ConfigurationEditor& editor() = 0;

        /** Access translator object.
            This method typically returns the current session's Translator.
            @return translator object */
        virtual afl::string::Translator& translator() = 0;

        /** Notify listeners.
            If you perform changes directly on user request, implement this using Session::notifyListeners()
            to let every change initiated by ConfigurationEditorProxy have immediate effect.
            If you perform changes in bulk, implement this empty and manually commit
            (gameSender.postRequest(&Session::notifyListeners)). */
        virtual void notifyListeners() = 0;
    };

} }

#endif
