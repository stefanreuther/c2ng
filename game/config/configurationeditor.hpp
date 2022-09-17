/**
  *  \file game/config/configurationeditor.hpp
  *  \brief Class game::config::ConfigurationEditor
  */
#ifndef C2NG_GAME_CONFIG_CONFIGURATIONEDITOR_HPP
#define C2NG_GAME_CONFIG_CONFIGURATIONEDITOR_HPP

#include "afl/base/closure.hpp"
#include "afl/base/deletable.hpp"
#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "game/config/configuration.hpp"
#include "game/config/integeroption.hpp"

namespace game { namespace config {

    /** Configuration editor.
        Describes the user-perceived structure of a configuration file, or a section there-of.
        Options are represented in a tree structure, where each node represents a number of options
        (zero for divider/folder nodes, one or more for actual values).
        Each entry contains some meta-information, including a human-readable name, depth in tree structure, and editor type hint.

        The ConfigurationEditor instance is independant from the actual Configuration instance,
        but shall only contain options from one type (i.e. not a mix of UserConfiguration and HostConfiguration).

        ConfigurationEditor can manage a current state and produce change callbacks;
        see loadValues(), updateValues().

        ConfigurationEditor provides only few methods to modify the Configuration,
        its main job is metadata, storage location editing, and change tracking.
        Users are expected to modify the ConfigurationOption instances directly. */
    class ConfigurationEditor {
     public:
        /*
         *  Editor Type Hints
         *
         *  Not interpreted by ConfigurationEditor.
         *  We define some generic values as negative numbers.
         *  Users can define their own values as positive numbers.
         */
        static const int NoEditor      = -1;         ///< Field is not editable. ex NoEditor
        static const int DefaultEditor = -2;         ///< Enter a string and use setValue(). ex DefaultEditor
        static const int ToggleEditor  = -3;         ///< Yes/not toggle; use toggleValue(). ex BoolEditor

        /** Source of an option.
            Extension of ConfigurationOption::Source to support multiple options in one node. */
        enum Source {
            NotStored,          ///< Not stored (no option associated).
            Mixed,              ///< Mixed source values.
            Default,            ///< ConfigurationOption::Default: Default value, not set by user.
            System,             ///< ConfigurationOption::System: System configuration file ("/etc/...").
            User,               ///< ConfigurationOption::User: User configuration file ("$HOME/...").
            Game                ///< ConfigurationOption::Game: Game configuration file.
        };

        /** Packed information about a node. */
        struct Info {
            int level;          ///< Level. @see Node::getLevel().
            int type;           ///< Type. @see Node::getType().
            Source source;      ///< Source. @see Node::getSource().
            String_t name;      ///< Name. @see Node::getName().
            String_t value;     ///< Value. @see Node::getValue().
            Info()
                : level(), type(), source(), name(), value()
                { }
        };

        /*
         *  Node Base Class
         */

        /** Node in editor tree.
            Derived classes provide meta-information including the list of options described by this node. */
        class Node : public afl::base::Deletable {
         protected:
            /** Constructor.
                @param level Indentation level (0=top)
                @param name  Human-readable, translated name */
            Node(int level, const String_t& name);

         public:
            /*
             *  Abstract Methods
             */

            /** Get this option's editor type.
                The interpretation of the type is defined by the user.
                @return type */
            virtual int getType() = 0;

            /** Get this option's value in configuration.
                @param config Configuration
                @param tx     Translator
                @return Human-readable value. Can be actual option value, a summary, a constant like "(dialog)", etc. */
            virtual String_t getValue(const Configuration& config, afl::string::Translator& tx) = 0;

            /** Enumerate options.
                Calls the provided function for all options described by this Node, in Configuration.
                @param config Configuration
                @param fcn    Closure to call */
            virtual void enumOptions(Configuration& config, afl::base::Closure<void(ConfigurationOption&)>& fcn) = 0;

            // TODO: get range, get enum values


            /*
             *  Concrete Methods
             */

            /** Get indentation level.
                @return level as given to constructor (0=top) */
            int getLevel() const;

            /** Get name.
                @return human-readable name as given to constructor */
            String_t getName() const;

            /** Get first option.
                Convenience method to access the first (and often, only) option represented by this node.
                Aliases (AliasOption) are resolved.
                This method uses enumOptions() to retrieve the options.
                @param config Configuration
                @return Option; null if none */
            ConfigurationOption* getFirstOption(Configuration& config);

            /** Get source (storage location) of this option.
                Extends ConfigurationOption::getSource() to also allow reporting of empty (no option)
                or mixed (multiple options in different locations) storage.
                @param config Configuration
                @return Source */
            Source getSource(const Configuration& config);

            /** Set source (storage location) of this option.
                @param config Configuration
                @param src    New storage location */
            void setSource(Configuration& config, ConfigurationOption::Source src);

            /** Toggle value.
                Convenience method to toggle the value of an IntegerOption;
                to use for options that report a type of ToggleEditor.
                Ignored if this is not actually an IntegerOption.
                @param config Configuration */
            void toggleValue(Configuration& config);

            /** Set value.
                Convenience method to set the value of any option.
                @param config Configuration
                @param value New value
                @see ConfigurationOption::setAndMarkUpdated() */
            void setValue(Configuration& config, String_t value);

            /** Describe.
                Packs all information about this option into a structure.
                @param config Configuration
                @param tx     Translator
                @return information */
            Info describe(Configuration& config, afl::string::Translator& tx);

            /** Update cached values.
                Stores the current source and value, to allow for detection of changes later on.
                @param config Configuration
                @param tx     Translator
                @return true if change detected since last call */
            bool update(Configuration& config, afl::string::Translator& tx);

         private:
            int m_level;
            String_t m_name;

            Source m_lastReportedSource;
            String_t m_lastReportedValue;
        };

        /*
         *  GenericNode
         *  (Needs to be in header file to allow user to use addOption().)
         */

        /** Generic node for an option or option group typically edited by a dialog.
            Reports source (storage location) for a configurable set of options, and a constant value.
            To use, construct (possibly using ConfigurationEditor::addGeneric()),
            and register options using addOption() or addOptionByName(). */
        class GenericNode : public Node {
         public:
            /** Constructor.
                @param level Indentation level (0=top)
                @param name  Human-readable, translated name
                @param type  Fixed type to report in getType()
                @param value Fixed value to report in getValue() */
            GenericNode(int level, const String_t& name, int type, const String_t& value);

            /** Add option, given a descriptor.
                @tparam Desc Descriptor type
                @param t Descriptor
                @return *this */
            template<typename Desc>
            GenericNode& addOption(Desc& t)
                { return addOptionByName(t.m_name); }

            /** Add option, given its name.
                @param name Name
                @return *this */
            GenericNode& addOptionByName(const String_t& name);

            // Abstract methods:
            virtual int getType();
            virtual String_t getValue(const Configuration& config, afl::string::Translator& tx);
            virtual void enumOptions(Configuration& config, afl::base::Closure<void(ConfigurationOption&)>& fcn);

         private:
            int m_type;
            String_t m_value;
            std::vector<String_t> m_optionNames;
        };

        /** Constructor.
            Make an empty ConfigurationEditor. */
        ConfigurationEditor();

        /** Destructor. */
        ~ConfigurationEditor();

        /** Add newly-constructed node.
            @param p New node, should not be null */
        void addNewNode(Node* p);

        /** Add newly-constructed node, template version.
            Like addNewNode(), but returns a correctly-typed reference to the original node;
            for in-place updates.
            @param p New node, must be derived from Node, must not be null
            @return *p */
        template<typename T>
        T& addNew(T* p);

        /** Add a divider node.
            Adds a node that reports NoEditor and has no options.
            @param level Indentation level (0=top)
            @param name  Human-readable, translated name */
        void addDivider(int level, const String_t& name);

        /** Add a boolean integer option node.
            Adds a node that reports ToggleEditor and represents a single IntegerOption.
            @param level Indentation level (0=top)
            @param name  Human-readable, translated name
            @param opt   Option descriptor, statically allocated */
        void addToggle(int level, const String_t& name, const IntegerOptionDescriptor& opt);

        /** Add a generic node for an option group typically edited by a dialog.
            To add options, use addOption(), addOptionByName() on the result.
            @param level Indentation level (0=top)
            @param name  Human-readable, translated name
            @param type  Fixed type to report in getType()
            @param value Fixed value to report in getValue()
            @return GenericNode instance */
        GenericNode& addGeneric(int level, const String_t& name, int type, const String_t& value);

        /** Add nodes for all options from a given configuration.
            @param level Indentation level (0=top)
            @param name  Human-readable, translated name
            @param config Configuration */
        void addAll(int level, int type, const Configuration& config);

        /** Get node, given an index.
            @param index Index [0,getNumNodes())
            @return Node; null if index is out-of-bounds */
        Node* getNodeByIndex(size_t index) const;

        /** Get number of nodes.
            @return number */
        size_t getNumNodes() const;

        /** Initialize change tracking.
            Loads all values by calling each node's update(), but does not generate any change signals.
            Call first in a session; subsequently use updateValues().
            @param config Configuration
            @param tx     Translator */
        void loadValues(Configuration& config, afl::string::Translator& tx);

        /** Check for changes.
            Updates all values by calling each node's update(), and generates a sig_change for each change.
            Call as response to Configuration::sig_change, or when using a new Configuration instance.

            After initiating a change, prefer to cause Configuration::sig_change to be called
            instead of calling updateValues() directly to have other listeners benefit from the change.

            @param config Configuration
            @param tx     Translator */
        void updateValues(Configuration& config, afl::string::Translator& tx);


        /** Signal: changed node.
            Called by updateValues() for each individual changed node.
            @param index Node index; access the node using getNodeByIndex() if desired */
        afl::base::Signal<void(size_t)> sig_change;


        /** Utility: convert ConfigurationOption::Source to ConfigurationEditor::Source.
            @param src ConfigurationOption::Source
            @return ConfigurationEditor::Source corresponding to src */
        static Source convertSource(ConfigurationOption::Source src);

     private:
        afl::container::PtrVector<Node> m_nodes;
    };

} }

template<typename T>
inline T&
game::config::ConfigurationEditor::addNew(T* p)
{
    addNewNode(p);
    return *p;
}

#endif
