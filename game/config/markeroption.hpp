/**
  *  \file game/config/markeroption.hpp
  *  \brief Class game::config::MarkerOption
  */
#ifndef C2NG_GAME_CONFIG_MARKEROPTION_HPP
#define C2NG_GAME_CONFIG_MARKEROPTION_HPP

#include "game/config/configurationoption.hpp"

namespace game { namespace config {

    class Configuration;

    /** Configuration option for a canned marker.
        Stores color, kind (shape), and note for the marker. */
    class MarkerOption : public ConfigurationOption {
     public:
        /** Data for a marker. */
        struct Data {
            uint8_t markerKind;        /**< Marker kind. \see game::map::Drawing::getMarkerKind() */
            uint8_t color;             /**< Marker color. \see game::map::Drawing::getColor() */
            String_t note;             /**< Note for this template (NOT marker comment). */

            Data()
                : markerKind(), color(), note()
                { }
            Data(uint8_t markerKind, uint8_t color, const String_t& note)
                : markerKind(markerKind), color(color), note(note)
                { }
        };

        /** Constructor.
            \param markerKind Initial kind (shape); see getMarkerKind
            \param color      Color; see getColor */
        MarkerOption(uint8_t markerKind, uint8_t color);
        ~MarkerOption();

        // ConfigurationOption:
        virtual void set(String_t value);
        virtual String_t toString() const;

        /** Access content (mutable).
            Remember to use markChanged() after changing the value this way.
            \return content */
        Data& operator()();

        /** Access content (constant).
            \return content */
        const Data& operator()() const;

        /** Set content.
            \param data New content */
        void set(const Data& data);

     private:
        Data m_data;
    };

    struct MarkerOptionDescriptor {
        // Instantiation information
        const char* m_name;
        uint8_t m_markerKind;
        uint8_t m_color;

        // Meta-information
        typedef MarkerOption OptionType_t;
        MarkerOption* create(Configuration& /*option*/) const
            { return new MarkerOption(m_markerKind, m_color); }
    };

} }

#endif
