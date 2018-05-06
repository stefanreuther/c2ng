/**
  *  \file game/config/configuration.cpp
  *  \brief Class game::config::Configuration
  */

#include "game/config/configuration.hpp"
#include "game/config/stringoption.hpp"

// FIXME: signal for changes

// Constructor.
game::config::Configuration::Configuration()
{ }

// Destructor.
game::config::Configuration::~Configuration()
{ }

// Get option, given a name.
game::config::ConfigurationOption*
game::config::Configuration::getOptionByName(String_t name) const
{
    // ex Config::getConfigOption
    return m_options[name];
}

// Set option.
void
game::config::Configuration::setOption(String_t name, String_t value, ConfigurationOption::Source source)
{
    ConfigurationOption* opt = getOptionByName(name);
    if (opt == 0) {
        opt = m_options.insertNew(name, new StringOption(""));
    }
    opt->setAndMarkUpdated(value, source);
}

// Enumeration.
afl::base::Ref<afl::base::Enumerator<std::pair<String_t,const game::config::ConfigurationOption*> > >
game::config::Configuration::getOptions() const
{
    /*
     *  We never delete configuration entries; all we possibly do is change existing ones.
     *  Therefore, all this iterator needs to do is to iterate through the map; map iterators remain stable.
     */
    class Iterator : public afl::base::Enumerator<std::pair<String_t,const ConfigurationOption*> > {
     public:
        Iterator(const Configuration& parent)
            : m_iterator(parent.m_options.begin()),
              m_end(parent.m_options.end())
            { }

        virtual bool getNextElement(std::pair<String_t,const ConfigurationOption*>& result)
            {
                if (m_iterator != m_end) {
                    result.first = m_iterator->first.toString();
                    result.second = m_iterator->second;
                    ++m_iterator;
                    return true;
                } else {
                    return false;
                }
            }
     private:
        Map_t::const_iterator m_iterator;
        Map_t::const_iterator m_end;
    };
    return *new Iterator(*this);
}

// Merge another set of options.
void
game::config::Configuration::merge(const Configuration& other)
{
    for (Map_t::const_iterator i = other.m_options.begin(), e = other.m_options.end(); i != e; ++i) {
        if (i->second->wasSet()) {
            setOption(i->first.toString(), i->second->toString(), i->second->getSource());
        }
    }
}

// Mark all options unset.
void
game::config::Configuration::markAllOptionsUnset()
{
    // ex Config::markAllOptionsUnset
    setAllOptionsSource(ConfigurationOption::Default);
}

void
game::config::Configuration::setAllOptionsSource(ConfigurationOption::Source source)
{
    for (Map_t::iterator i = m_options.begin(), e = m_options.end(); i != e; ++i) {
        i->second->setSource(source);
    }
}

void
game::config::Configuration::notifyListeners()
{
    bool needed = false;
    for (Map_t::iterator i = m_options.begin(), e = m_options.end(); i != e; ++i) {
        if (i->second->isChanged()) {
            i->second->markChanged(false);
            needed = true;
        }
    }
    if (needed) {
        sig_change.raise();
    }
}
