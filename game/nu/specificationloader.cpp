/**
  *  \file game/nu/specificationloader.cpp
  *  \brief Class game::nu::SpecificationLoader
  */

#include "game/nu/specificationloader.hpp"
#include "afl/data/access.hpp"
#include "game/root.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/nu/loader.hpp"

using afl::data::Access;
using afl::sys::LogListener;

namespace {
    const char LOG_NAME[] = "game.nu";
}

game::nu::SpecificationLoader::SpecificationLoader(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                                   afl::base::Ref<GameState> gameState,
                                                   afl::string::Translator& tx,
                                                   afl::sys::LogListener& log)
    : m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_gameState(gameState),
      m_translator(tx),
      m_log(log)
{ }

game::nu::SpecificationLoader::~SpecificationLoader()
{ }

std::auto_ptr<game::Task_t>
game::nu::SpecificationLoader::loadShipList(game::spec::ShipList& list, Root& root, std::auto_ptr<StatusTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(SpecificationLoader& parent, game::spec::ShipList& list, Root& root, std::auto_ptr<StatusTask_t>& then)
            : m_parent(parent), m_shipList(list), m_root(root), m_then(then)
            { }
        virtual void call()
            {
                try {
                    m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Task: loadShipList");

                    // Load result from network
                    Access rst(m_parent.m_gameState->loadResultPreAuthenticated());

                    // Load defaults from local storage
                    m_parent.loadHullFunctionDefinitions(m_shipList);

                    // Process the result
                    Loader(m_parent.m_translator, m_parent.m_log).loadShipList(m_shipList, m_root, rst);
                    return m_then->call(true);
                }
                catch (std::exception& e) {
                    m_parent.m_log.write(LogListener::Error, LOG_NAME, String_t(), e);
                    return m_then->call(false);
                }
            }
     private:
        SpecificationLoader& m_parent;
        game::spec::ShipList& m_shipList;
        Root& m_root;
        std::auto_ptr<StatusTask_t> m_then;
    };
    return m_gameState->login(std::auto_ptr<Task_t>(new Task(*this, list, root, then)));
}

afl::base::Ref<afl::io::Stream>
game::nu::SpecificationLoader::openSpecificationFile(const String_t& fileName)
{
    return m_defaultSpecificationDirectory->openFile(fileName, afl::io::FileSystem::OpenRead);
}

void
game::nu::SpecificationLoader::loadHullFunctionDefinitions(game::spec::ShipList& list)
{
    // We load the basic function definitions in the same way as for V3.
    // This enables subsequent code to use it, in particular, the hull "cancloak" flag.
    // We do not ever define modified functions.
    list.basicHullFunctions().clear();
    afl::base::Ptr<afl::io::Stream> ps = m_defaultSpecificationDirectory->openFileNT("hullfunc.usr", afl::io::FileSystem::OpenRead);
    if (ps.get()) {
        list.basicHullFunctions().load(*ps, m_translator, m_log);
    }
    ps = m_defaultSpecificationDirectory->openFileNT("hullfunc.cc", afl::io::FileSystem::OpenRead);
    if (ps.get()) {
        list.basicHullFunctions().load(*ps, m_translator, m_log);
    }
}
