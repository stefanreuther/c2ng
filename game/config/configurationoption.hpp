/**
  *  \file game/config/configurationoption.hpp
  *  \brief Class game::config::ConfigurationOption
  */
#ifndef C2NG_GAME_CONFIG_CONFIGURATIONOPTION_HPP
#define C2NG_GAME_CONFIG_CONFIGURATIONOPTION_HPP

#include "afl/string/string.hpp"

namespace game { namespace config {

    /** Configuration option, base class.
        This is the basic interface to assigning configuration options.

        Attributes:
        - Value: can be set using set() and obtained using toString().
          Derived classes implement the generic interface and additional typed interfaces.
        - Source: where does this option originate from (default? which config file?).
        - Change flag: used to drive Configuration::notifyListeners().

        Lifetime: a ConfigurationOption typically lives in a Configuration and has its lifetime managed by it.

        Terminology:
        - an option was "set" or "updated" when it was obtained from a configuration file (source != Default).
          This is used to track origins of configuration values, for rewriting config files or display.
        - an option was "changed" when its value changed for whatever reason.
          This is used to drive Configuration::notifyListeners() on configuration changes. */
    class ConfigurationOption {
     public:
        /** Source of this option's value.
            These values are ordered by specificity. */
        enum Source {
            Default,            ///< Default value, not set by user.
            System,             ///< System configuration file ("/etc/...").
            User,               ///< User configuration file ("$HOME/...").
            Game                ///< Game configuration file.
        };

        /** Constructor. */
        ConfigurationOption();

        /** Virtual destructor. */
        virtual ~ConfigurationOption();

        /** Set value from string.
            This function must
            - update the option value.
            - if there is a change, call markChanged().

            Likewise, if the derived class implements special-purpose set() functions,
            those must call markChanged() if appropriate.

            This function does not update the source.
            Use setSource() or setAndMarkUpdated() to do that.

            \param value Option value, as a string */
        virtual void set(String_t value) = 0;

        /** Get value as string.
            \return option value */
        virtual String_t toString() const = 0;

        /** Get source of this option (place where it was set).
            \return source */
        Source getSource() const;

        /** Set source of this option.
            \param source source */
        void setSource(Source source);

        /** Check whether option was set.
            An option is considered set if has been given a value from a source other than the default. */
        bool wasSet() const;

        /** Mark this option updated.
            If this option is set at a level below \c source, upgrade it to that level.
            In particular, this moves a default option to the respective config file.
            \param source source (minimum) */
        void markUpdated(Source source);

        /** Set this option and mark it updated.
            This is a convenience method for set() and markUpdated().
            This is the preferred way to set an option from user input.
            \param value New value
            \param source source */
        void setAndMarkUpdated(String_t value, Source source);

        /** Mark this option changed.
            \param state New state. Default value is true to mark this option changed for the next notifyListeners iteration. */
        void markChanged(bool state = true);

        /** Check whether this option was changed.
            \return true if option was changed */
        bool isChanged() const;


        /** Remove comment from an option value.
            \param str [in/out] option value */
        static void removeComment(String_t& str);

     private:
        Source m_source : 8;
        bool m_changed;
    };

} }

// Virtual destructor.
inline
game::config::ConfigurationOption::~ConfigurationOption()
{ }

// Get source of this option (place where it was set).
inline game::config::ConfigurationOption::Source
game::config::ConfigurationOption::getSource() const
{
    // ex ConfigOption::getSource
    return m_source;
}

// Check whether option was set.
inline bool
game::config::ConfigurationOption::wasSet() const
{
    // ex ConfigOption::wasSet
    return m_source != Default;
}

// Mark this option changed.
inline void
game::config::ConfigurationOption::markChanged(bool state)
{
    m_changed = state;
}

// Check whether this option was changed.
inline bool
game::config::ConfigurationOption::isChanged() const
{
    return m_changed;
}

#endif
