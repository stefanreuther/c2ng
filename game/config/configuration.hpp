/**
  *  \file game/config/configuration.hpp
  *  \brief Class game::config::Configuration
  */
#ifndef C2NG_GAME_CONFIG_CONFIGURATION_HPP
#define C2NG_GAME_CONFIG_CONFIGURATION_HPP

#include <memory>
#include "afl/container/ptrmap.hpp"
#include "afl/string/string.hpp"
#include "game/config/configurationoption.hpp"
#include "afl/base/signal.hpp"

namespace game { namespace config {

    /** Configuration data.
        Stores a pool of configuration options, such as HConfig/PConfig or pcc2.ini.

        Configuration options are objects of a type derived from ConfigurationOption.
        The regular way of accessing a configuration is by indexing the configuration with a descriptor.
        A descriptor is a pod-structure that defines name, type, and initial value of an option;
        see operator[].
        
        If an option is accessed with a different type than the one already in the configuration, it is attempted to convert it.
        Typically, this will have the original user-supplied value in a StringOption, and the application-defined descriptor.

        Using this mechanism means that every indexing access needs a runtime type check.
        If a configuration value is needed in a tight loop, it should be cached.
        However, this implementation has the advantage of not needing a-prioriy knowledge about option types.

        This class is completely different from the PCC2 version, to add more flexibility. */
    class Configuration {
     public:
        /** Constructor.
            Makes an empty configuration. */
        Configuration();

        /** Destructor. */
        ~Configuration();

        /** Get option, given a name.
            For typed access, use the indexing operator.
            \param name Name of option
            \return option, if defined. Null if the option does not exist. */
        ConfigurationOption* getOptionByName(String_t name) const;

        /** Set option.
            This is the function to use for a configuration parser.
            For typed access, use the indexing operator.
            \param name Name of option
            \param value New value
            \param source Source of this value */
        void setOption(String_t name, String_t value, ConfigurationOption::Source source);

        /** Access by descriptor.
            When accessing an option that does not already exist or have the wrong type, it is created or converted.
            Index access is perceived as a read-only operation and thus allowed on const objects, although it may change the underlying data.

            \param Descriptor descriptor type. Must have
            - a member type \c OptionType_t with the actual option type
            - a member function \c create(Configuration&) to create the option instance
            \param desc descriptor instance
            \return option of appropriate type */
        template<typename Descriptor>
        typename Descriptor::OptionType_t&
        operator[](const Descriptor& desc);

        template<typename Descriptor>
        const typename Descriptor::OptionType_t&
        operator[](const Descriptor& desc) const;

        /** Mark all options unset (default). */
        void markAllOptionsUnset();

        /** Notify all listeners.
            If there is an option that is marked as changed, resets all options' change flags and broadcasts a sig_change. */
        void notifyListeners();

        /** Signal: configuration change. */
        afl::base::Signal<void()> sig_change;

     private:
        class CasePreservingString {
         public:
            CasePreservingString(String_t value)
                : m_value(value)
                { }
            CasePreservingString(const char* value)
                : m_value(value)
                { }
            bool operator<(const CasePreservingString& other) const
                { return afl::string::strCaseCompare(m_value, other.m_value) < 0; }
         private:
            String_t m_value;
        };

        typedef afl::container::PtrMap<CasePreservingString, ConfigurationOption> Map_t;
        mutable Map_t m_options;
    };

} }

// Access by descriptor.
template<typename Descriptor>
typename Descriptor::OptionType_t&
game::config::Configuration::operator[](const Descriptor& desc)
{
    typedef typename Descriptor::OptionType_t OptionType_t;
    ConfigurationOption* option = m_options[desc.m_name];
    OptionType_t* result = dynamic_cast<OptionType_t*>(option);
    if (result == 0) {
        std::auto_ptr<OptionType_t> newOption(desc.create(*this));
        if (option != 0) {
            try {
                newOption->set(option->toString());
                newOption->setSource(option->getSource());
            }
            catch (...) { }
        }
        result = newOption.release();
        m_options.insertNew(desc.m_name, result);
    }
    return *result;
}

// Access by descriptor.
template<typename Descriptor>
const typename Descriptor::OptionType_t&
game::config::Configuration::operator[](const Descriptor& desc) const
{
    return (*const_cast<Configuration*>(this))[desc];
}

#endif
