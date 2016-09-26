/**
  *  \file game/config/genericintegerarrayoption.hpp
  */
#ifndef C2NG_GAME_CONFIG_GENERICINTEGERARRAYOPTION_HPP
#define C2NG_GAME_CONFIG_GENERICINTEGERARRAYOPTION_HPP

#include "game/config/configurationoption.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"

namespace game { namespace config {

    class ValueParser;

    /** Base integer option array.
        Base class for an int32_t array, can inform user about its dimensions.
        This is used for the script interface, but shouldn't be used directly. */
    class GenericIntegerArrayOption : public ConfigurationOption {
     public:
        explicit GenericIntegerArrayOption(const ValueParser& parser);
        ~GenericIntegerArrayOption();

        virtual afl::base::Memory<int32_t> getArray() = 0;

        bool isAllTheSame() const;
        void set(int index, int32_t value);
        void set(int32_t value);
        int32_t operator()(int index) const;

        // ConfigurationOption:
        virtual void set(String_t value);

        const ValueParser& parser() const;

     private:
        const ValueParser& m_parser;
    };

} }

#endif
