/**
  *  \file game/interface/plugins.cpp
  *  \brief Plugin Integration
  */

#include "game/interface/plugins.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/format.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "util/translation.hpp"

using util::plugin::Plugin;
using interpreter::Opcode;

namespace {
    void addPushString(interpreter::BytecodeObject& bco, const String_t& s)
    {
        afl::data::StringValue sv(s);
        bco.addPushLiteral(&sv);
    }

    // Generate a "Print Format(Translate(...), pluginName, errorMessage)".
    // The error message is on top-of-stack and is accessed from there; the stack is left unchanged.
    void addErrorPrint(interpreter::BytecodeObject& bco, const String_t& message, const String_t& id)
    {
        addPushString(bco, message);
        bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("TRANSLATE"));
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
        addPushString(bco, id);
        bco.addInstruction(Opcode::maStack, Opcode::miStackDup, 2);
        bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("FORMAT"));
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 3);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);
    }

    void compileItem(interpreter::BytecodeObject& bco, const Plugin::Item& item)
    {
        interpreter::BytecodeObject::Label_t label;
        switch (item.type) {
         case Plugin::PlainFile:
            // Nothing to do
            break;

         case Plugin::ScriptFile:
            // Load file
            //   Load MakeFileName(Directory, "...")
            label = bco.makeLabel();
            bco.addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco.addName("DIRECTORY"));
            addPushString(bco, item.name);
            bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("MAKEFILENAME"));
            bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 2);
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialLoad, 0);
            bco.addJump(Opcode::jIfEmpty, label);
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
            bco.addLabel(label);
            bco.addInstruction(Opcode::maStore, Opcode::miStackDrop, 1);
            break;

         case Plugin::ResourceFile:
            // Load resource file
            //   LoadResource "..."
            // (LoadResource internally applies the plugin directory.)
            addPushString(bco, item.name);
            bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("LOADRESOURCE"));
            bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 1);
            break;

         case Plugin::HelpFile:
            // Load help file
            //   LoadHelpFile "..."
            // (LoadHelpFile internally applies the plugin directory.)
            addPushString(bco, item.name);
            bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("LOADHELPFILE"));
            bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 1);
            break;

         case Plugin::Command:
            // Evaluate code
            //   Eval "..."
            addPushString(bco, item.name);
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
            break;
        }
    }
}

// Create plugin loader for a single plugin.
interpreter::BCORef_t
game::interface::createPluginLoader(const util::plugin::Plugin& plugin)
{
    // ex client/main.cc:loadPlugin (sort-of)

    // Create a BCO
    interpreter::BCORef_t result(interpreter::BytecodeObject::create(true));
    interpreter::BytecodeObject& bco = *result;
    bco.setFileName(plugin.getDefinitionFileName());
    bco.setSubroutineName(plugin.getId());
    bco.setOrigin(plugin.getId());

    // Create plugin context
    addPushString(bco, plugin.getId());
    bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("SYSTEM.PLUGIN"));
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);

    // Error protection: wrap the whole plugin in
    //   Try ...
    //   Else Print "...."
    interpreter::BytecodeObject::Label_t catchLabel = bco.makeLabel();
    interpreter::BytecodeObject::Label_t endLabel = bco.makeLabel();
    bco.addJump(Opcode::jCatch, catchLabel);

    // Compile individual items
    const Plugin::ItemList_t& items = plugin.getItems();
    for (size_t i = 0, n = items.size(); i < n; ++i) {
        compileItem(bco, items[i]);
    }

    // Error protection, else part
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
    bco.addJump(Opcode::jAlways, endLabel);
    bco.addLabel(catchLabel);

    // At this point, the stack contains the error message.
    addErrorPrint(bco, N_("Load of plugin \"%s\" failed: %s"), plugin.getId());
    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    bco.addLabel(endLabel);

    // Leave plugin context
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);

    // Finish the BCO
    bco.relocate();
    return result;
}

// Create plugin loader for all unloaded plugins.
interpreter::BCORef_t
game::interface::createLoaderForUnloadedPlugins(util::plugin::Manager& manager)
{
    // ex client/main.cc:loadPlugins

    // Create a BCO
    interpreter::BCORef_t result(interpreter::BytecodeObject::create(true));
    interpreter::BytecodeObject& bco = *result;
    bco.setSubroutineName("<PluginLoader>");

    // List plugins
    std::vector<Plugin*> plugins;
    manager.enumPlugins(plugins, true);

    // Call each plugin's initializer
    for (size_t i = 0, n = plugins.size(); i < n; ++i) {
        if (Plugin* plug = plugins[i]) {
            if (!plug->isLoaded()) {
                // Load it
                interpreter::SubroutineValue sv(createPluginLoader(*plug));
                bco.addPushLiteral(&sv);
                bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0);

                // Mark loaded, assuming loading succeeds
                plug->setLoaded(true);
            }
        }
    }

    // Finish the BCO
    bco.relocate();

    return result;
}

// Create a file loader.
interpreter::BCORef_t
game::interface::createFileLoader(const String_t& fileName, const String_t& origin, bool optional)
{
    // Create a BCO
    interpreter::BCORef_t result(interpreter::BytecodeObject::create(true));
    interpreter::BytecodeObject& bco = *result;
    bco.setSubroutineName(afl::string::Format("<FileLoader:%s>", fileName));
    bco.setOrigin(origin);

    // Wrap in a try/else to be able to log error messages
    interpreter::BytecodeObject::Label_t catchLabel = bco.makeLabel();
    interpreter::BytecodeObject::Label_t endLabel = bco.makeLabel();
    bco.addJump(Opcode::jCatch, catchLabel);

    // Load the file
    addPushString(bco, fileName);
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialLoad, 0);

    // Error handling. TOS is either empty (ok) or an error message.
    if (!optional) {
        interpreter::BytecodeObject::Label_t successLabel = bco.makeLabel();
        bco.addJump(Opcode::jIfEmpty, successLabel);
        addErrorPrint(bco, N_("Load of file \"%s\" failed: %s"), fileName);
        bco.addLabel(successLabel);
    }
    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

    // Error protection, else part
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
    bco.addJump(Opcode::jAlways, endLabel);
    bco.addLabel(catchLabel);

    // At this point, the stack contains the error message.
    addErrorPrint(bco, N_("Execution of file \"%s\" failed: %s"), fileName);
    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    bco.addLabel(endLabel);

    // Finish the BCO
    bco.relocate();
    return result;
}
