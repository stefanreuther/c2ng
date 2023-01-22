/**
  *  \file game/interface/configurationeditorcontext.hpp
  *  \brief Class game::interface::ConfigurationEditorContext
  */
#ifndef C2NG_GAME_INTERFACE_CONFIGURATIONEDITORCONTEXT_HPP
#define C2NG_GAME_INTERFACE_CONFIGURATIONEDITORCONTEXT_HPP

#include "game/config/configurationeditor.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/context.hpp"
#include "interpreter/singlecontext.hpp"
#include "util/treelist.hpp"

namespace game { namespace interface {

    /** Configuration Editor Context.
        This object is used to prepare a configuration editor user interface.
        - create an instance (through IFConfigurationEditorContext)
        - on this instance, call script functions to add items
        - on this instance, call compileEditor() to modify items

        For now, scripts can only create and fill the ConfigurationEditorContext, functions to examine and use it are not provided.

        Underlying data consists of a game::config::ConfigurationEditor which provides the configuration editing functionality,
        and a util::TreeList which provides the user-visible shape of the editor.

        ConfigurationEditor is indexed with 0-based indexes of type size_t.
        util::TreeList provides 1-based indexes of type int32_t.
        Be sure to use getTreeIdFromEditorIndex(), getEditorIndexFromTreeId() to convert. */
    class ConfigurationEditorContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Data. */
        struct Data : public afl::base::RefCounted {
            Session& session;                             ///< Link to session.
            game::config::ConfigurationEditor editor;     ///< ConfigurationEditor instance.
            util::TreeList optionNames;                   ///< Tree structure.

            explicit Data(Session& session)
                : session(session), editor(), optionNames()
                { }
        };

        struct DataRef {
            afl::base::Ref<Data> ref;
            size_t root;
            DataRef(const afl::base::Ref<Data>& ref, size_t root)
                : ref(ref), root(root)
                { }
        };

        /** Type used for tagging editor nodes.
            All nodes in a ConfigurationEditorContext will have this type.
            Compare game::config::ConfigurationEditor::DefaultEditor. */
        static const int ScriptEditor = 100;

        /** Constructor.
            Makes a fresh ConfigurationEditorContext with no options on it.
            @param session Session */
        explicit ConfigurationEditorContext(Session& session);

        /** Constructor.
            Makes a ConfigurationEditorContext from pre-existing data.
            @param session Session */
        explicit ConfigurationEditorContext(const afl::base::Ref<Data>& ref, size_t root);

        /** Destructor. */
        ~ConfigurationEditorContext();

        /** Access underlying data.
            @return data */
        const DataRef& data() const
            { return m_data; }

        // ReadOnlyAccessor:
        virtual afl::data::Value* get(PropertyIndex_t index);

        // Context:
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual ConfigurationEditorContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Generate code to edit an option, given an index.
            @param [in,out] bco    BytecodeObject; code will be added to this
            @param [in]     index  Index into ConfigurationEditor
            @see getEditorIndexFromTreeId() */
        void compileEditor(interpreter::BytecodeObject& bco, size_t index) const;

        /** Get index into Data::editor, given an Id from Data::optionNames.
            @param id Id
            @return Index */
        static size_t getEditorIndexFromTreeId(int32_t id);

        /** Get Id from Data::optionNames, given an index into Data::editor.
            @param index Index
            @return Id
            @throw interpreter::Error if index cannot be converted */
        static int32_t getTreeIdFromEditorIndex(size_t index);

     private:
        DataRef m_data;
    };

    /** Implementation of "ConfigurationEditorContext()" function.
        For use with SimpleFunction<Session&>.
        @param session Game session; required for UserConfiguration, Translator.
        @param args Script-provided arguments */
    afl::data::Value* IFConfigurationEditorContext(Session& session, interpreter::Arguments& args);

} }

#endif
