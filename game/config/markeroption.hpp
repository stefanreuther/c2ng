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
        /** Constructor.
            \param markerKind Initial kind (shape); see getMarkerKind
            \param color      Color; see getColor */
        MarkerOption(uint8_t markerKind, uint8_t color);
        ~MarkerOption();

        // ConfigurationOption:
        virtual void set(String_t value);
        virtual String_t toString() const;

        /** Get color.
            \return color
            \see game::map::Drawing::getColor */
        uint8_t  getColor() const;

        /** Set color.
            \param color Color
            \see game::map::Drawing::setColor */
        void setColor(uint8_t color);

        /** Get marker kind (shape).
            \return Kind [0,game::map::Drawing::NUM_USER_MARKERS)
            \see game::map::Drawing::getMarkerKind */
        uint8_t getMarkerKind() const;

        /** Set marker kind (shape).
            \param k Kind [0,game::map::Drawing::NUM_USER_MARKERS)
            \see game::map::Drawing::setMarkerKind */
        void setMarkerKind(uint8_t markerKind);

        /** Get note.
            The note describes this canned marker slot; it is not used as marker comment!
            \return note */
        String_t getNote() const;

        /** Set note.
            The note describes this canned marker slot; it is not used as marker comment!
            \param note Note */
        void setNote(String_t note);

     private:
        uint8_t m_markerKind;
        uint8_t m_color;
        String_t m_note;
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
