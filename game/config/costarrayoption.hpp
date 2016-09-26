/**
  *  \file game/config/costarrayoption.hpp
  *  \brief Class game::config::CostArrayOption
  */
#ifndef C2NG_GAME_CONFIG_COSTARRAYOPTION_HPP
#define C2NG_GAME_CONFIG_COSTARRAYOPTION_HPP

#include "game/config/configurationoption.hpp"
#include "game/spec/cost.hpp"
#include "game/limits.hpp"

namespace game { namespace config {

    class Configuration;

    /** Array of costs.
        This option type is used to define unit costs. */
    class CostArrayOption : public ConfigurationOption {
     public:
        /** Constructor. */
        CostArrayOption();

        /** Destructor. */
        virtual ~CostArrayOption();

        // ConfigurationOption:
        virtual void set(String_t value);
        virtual String_t toString() const;

        /** Set single slot.
            \param index Player number [1,MAX_PLAYERS]. Out-of-range values are ignored.
            \param cost New value */
        void set(int index, const game::spec::Cost& cost);

        /** Get single slot.
            \param index Player number [1,MAX_PLAYERS]. Out-of-range values are mapped to the last element.
            \return cost */
        game::spec::Cost operator()(int index) const;

     private:
        game::spec::Cost m_data[MAX_PLAYERS];
    };

    /** Descriptor for CostArrayOption. */
    struct CostArrayOptionDescriptor {
        const char* m_name;

        typedef CostArrayOption OptionType_t;
        OptionType_t* create(Configuration& /*option*/) const
            { return new OptionType_t(); }
    };

} }

#endif
