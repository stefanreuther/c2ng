/**
  *  \file game/proxy/browserproxy.cpp
  *  \brief Class game::proxy::BrowserProxy
  */

#include "game/proxy/browserproxy.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/string/format.hpp"
#include "game/browser/account.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/turnloader.hpp"
#include "game/types.hpp"

using afl::base::Ptr;
using afl::container::PtrVector;
using afl::sys::LogListener;
using game::Player;
using game::PlayerList;
using game::Root;
using game::TurnLoader;
using game::browser::Browser;
using game::browser::Folder;
using game::Task_t;
using game::config::ConfigurationOption;
using game::config::IntegerOption;
using game::config::StringOption;
using game::config::UserConfiguration;
using game::proxy::BrowserProxy;

namespace {
    const char* LOG_NAME = "game.proxy";

    /* Pack folders for output */
    void packFolders(std::vector<BrowserProxy::Item>& out, const PtrVector<Folder>& in)
    {
        for (size_t i = 0, n = in.size(); i < n; ++i) {
            out.push_back(BrowserProxy::Item(in[i]->getName(), in[i]->getKind(), in[i]->canEnter()));
        }
    }

    /*
     *  Game -> UI tasks
     */

    /* Publish a sig_update */
    class UpdateTask : public util::Request<BrowserProxy> {
     public:
        UpdateTask(const PtrVector<Folder>& path, const PtrVector<Folder>& content, Browser::OptionalIndex_t index)
            : m_info()
            {
                packFolders(m_info.path, path);
                packFolders(m_info.content, content);
                m_info.index = index;
            }
        void handle(BrowserProxy& proxy)
            { proxy.sig_update.raise(m_info); }
     private:
        BrowserProxy::Info m_info;
    };

    /* Publish a sig_selectedInfoUpdate */
    class UpdateInfoTask : public util::Request<BrowserProxy> {
     public:
        UpdateInfoTask(BrowserProxy::OptionalIndex_t index, std::auto_ptr<BrowserProxy::FolderInfo>& info)
            : m_index(index),
              m_info(info)
            { }

        virtual void handle(BrowserProxy& proxy)
            { proxy.sig_selectedInfoUpdate.raise(m_index, *m_info); }
     private:
        BrowserProxy::OptionalIndex_t m_index;
        std::auto_ptr<BrowserProxy::FolderInfo> m_info;
    };


    /*
     *  Browser tasks (part of a possibly long-lived browser task chain)
     */

    /* Inform UI side of reloaded content and finish task chain */
    class PostLoadTask : public Task_t {
     public:
        PostLoadTask(util::RequestSender<BrowserProxy> reply, game::browser::Session& session)
            : m_reply(reply), m_session(session)
            { }
        virtual void call()
            {
                m_session.log().write(LogListener::Trace, LOG_NAME, "Task: PostLoadTask");
                Browser& bro = m_session.browser();
                m_reply.postNewRequest(new UpdateTask(bro.path(), bro.content(), bro.getSelectedChildIndex()));
                m_session.finishTask();
            }
        static std::auto_ptr<Task_t> make(util::RequestSender<BrowserProxy> reply, game::browser::Session& session)
            { return std::auto_ptr<Task_t>(new PostLoadTask(reply, session)); }
     private:
        util::RequestSender<BrowserProxy> m_reply;
        game::browser::Session& m_session;
    };

    /* Save accounts, then proceed chain */
    class SaveAccountsTask : public Task_t {
     public:
        SaveAccountsTask(game::browser::Session& session, std::auto_ptr<Task_t> then)
            : m_session(session), m_then(then)
            { }
        virtual void call()
            {
                m_session.log().write(LogListener::Trace, LOG_NAME, "Task: SaveAccountsTask");
                m_session.accountManager().save();
                m_then->call();
            }
     private:
        game::browser::Session& m_session;
        std::auto_ptr<Task_t> m_then;
    };

    static void buildFolderInfo(Folder& f, BrowserProxy::FolderInfo& info)
    {
        info.title = f.getName();
        info.subtitle = f.getDescription();
    }

    static void buildPlayerList(const Root& root, TurnLoader& loader, BrowserProxy::FolderInfo& info, afl::string::Translator& tx)
    {
        const PlayerList& pl = root.playerList();
        for (const Player* p = pl.getFirstPlayer(); p != 0; p = pl.getNextPlayer(p)) {
            String_t extra;
            TurnLoader::PlayerStatusSet_t st = loader.getPlayerStatus(p->getId(), extra, tx);
            if (!st.empty()) {
                info.availablePlayers += p->getId();
                info.playerNames.set(p->getId(), p->getName(Player::ShortName, tx));
                info.playerExtra.set(p->getId(), extra);
            }
        }
    }

    /* Build information about child (selected folder), then inform user and finish task */
    class ChildBuilder : public Task_t {
     public:
        ChildBuilder(util::RequestSender<BrowserProxy> reply, game::browser::Session& session, std::auto_ptr<BrowserProxy::FolderInfo>& result)
            : m_reply(reply), m_session(session), m_result(result)
            { }
        virtual void call()
            {
                // Build information
                m_session.log().write(LogListener::Trace, LOG_NAME, "Task: ChildBuilder");
                Browser& b = m_session.browser();
                if (Folder* f = b.getSelectedChild()) {
                    Ptr<Root> root = b.getSelectedRoot();
                    Ptr<TurnLoader> loader = root.get() != 0 ? root->getTurnLoader() : 0;
                    if (loader.get() != 0) {
                        // Folder contains a game
                        const StringOption& name = root->hostConfiguration()[game::config::HostConfiguration::GameName];
                        m_result->title = name.wasSet() ? name() : f->getName();
                        m_result->subtitle = util::rich::Text(afl::string::Format(m_session.translator()("A %s game"), root->hostVersion().toString()));
                        buildPlayerList(*root, *loader, *m_result, m_session.translator());
                        m_result->canEnter = f->canEnter();
                        m_result->possibleActions = root->getPossibleActions();
                    } else {
                        // No game in this folder
                        buildFolderInfo(*f, *m_result);
                    }
                }

                // Inform user. getSelectedChildIndex() should always be set at this point.
                m_reply.postNewRequest(new UpdateInfoTask(b.getSelectedChildIndex().orElse(0), m_result));
                m_session.finishTask();
            }
     private:
        util::RequestSender<BrowserProxy> m_reply;
        game::browser::Session& m_session;
        std::auto_ptr<BrowserProxy::FolderInfo> m_result;
    };
}

class game::proxy::BrowserProxy::Trampoline : public game::browser::UserCallback,
                                              private afl::base::Uncopyable {
 public:
    Trampoline(game::browser::Session& session, util::RequestSender<BrowserProxy> reply)
        : m_session(session), m_reply(reply)
        { m_session.callback().setInstance(this); }

    ~Trampoline()
        { m_session.callback().setInstance(0); }

    virtual void askPassword(const PasswordRequest& req)
        {
            class Task : public util::Request<BrowserProxy> {
             public:
                Task(const PasswordRequest& req)
                    : m_req(req)
                    { }
                virtual void handle(BrowserProxy& proxy)
                    { proxy.m_callback.askPassword(m_req); }
             private:
                const PasswordRequest m_req;
            };
            m_reply.postNewRequest(new Task(req));
        }

    game::browser::Session& session()
        { return m_session; }

 private:
    game::browser::Session& m_session;
    util::RequestSender<BrowserProxy> m_reply;
};


/*
 *  TrampolineFromSession
 */

class game::proxy::BrowserProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(game::browser::Session&)> {
 public:
    TrampolineFromSession(util::RequestSender<BrowserProxy> reply)
        : m_reply(reply)
        { }
    virtual Trampoline* call(game::browser::Session& session)
        { return new Trampoline(session, m_reply); }
 private:
    util::RequestSender<BrowserProxy> m_reply;
};


/*
 *  BrowserProxy
 */

// Constructor.
game::proxy::BrowserProxy::BrowserProxy(util::RequestSender<game::browser::Session> sender, util::RequestDispatcher& reply, game::browser::UserCallback& callback)
    : m_callback(callback),
      m_reply(reply, *this),
      m_sender(sender.makeTemporary(new TrampolineFromSession(m_reply.getSender()))),
      conn_passwordResult(m_callback.sig_passwordResult.add(this, &BrowserProxy::onPasswordResult))
{ }

// Destructor.
game::proxy::BrowserProxy::~BrowserProxy()
{ }

// Load content of current position.
void
game::proxy::BrowserProxy::loadContent()
{
    class InitTask : public util::Request<Trampoline> {
     public:
        InitTask(util::RequestSender<BrowserProxy> reply)
            : m_reply(reply)
            { }
        void handle(Trampoline& t)
            { t.session().addTask(t.session().browser().loadContent(PostLoadTask::make(m_reply, t.session()))); }
     private:
        util::RequestSender<BrowserProxy> m_reply;
    };
    m_sender.postNewRequest(new InitTask(m_reply.getSender()));
}

// Open child folder.
void
game::proxy::BrowserProxy::openChild(size_t nr)
{
    class EnterTask : public util::Request<Trampoline> {
     public:
        EnterTask(size_t nr, util::RequestSender<BrowserProxy> reply)
            : m_number(nr),
              m_reply(reply)
            { }
        void handle(Trampoline& t)
            {
                Browser& b = t.session().browser();
                b.openChild(m_number);
                t.session().addTask(b.loadContent(PostLoadTask::make(m_reply, t.session())));
            }
     private:
        size_t m_number;
        util::RequestSender<BrowserProxy> m_reply;
    };
    m_sender.postNewRequest(new EnterTask(nr, m_reply.getSender()));
}

// Open parent folder.
void
game::proxy::BrowserProxy::openParent(size_t nr)
{
    class UpTask : public util::Request<Trampoline> {
     public:
        UpTask(size_t nr, util::RequestSender<BrowserProxy> reply)
            : m_number(nr),
              m_reply(reply)
            { }
        void handle(Trampoline& t)
            {
                Browser& b = t.session().browser();
                for (size_t i = 0; i < m_number; ++i) {
                    b.openParent();
                }
                t.session().addTask(b.loadContent(PostLoadTask::make(m_reply, t.session())));
            }
     private:
        size_t m_number;
        util::RequestSender<BrowserProxy> m_reply;
    };
    m_sender.postNewRequest(new UpTask(nr, m_reply.getSender()));
}

// Open folder by name/URL.
void
game::proxy::BrowserProxy::openFolder(String_t name)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const String_t& name, util::RequestSender<BrowserProxy> reply)
            : m_name(name),
              m_reply(reply)
            { }
        void handle(Trampoline& t)
            {
                game::browser::Browser& b = t.session().browser();
                b.openFolder(m_name);
                t.session().addTask(b.loadContent(PostLoadTask::make(m_reply, t.session())));
            }
     private:
        String_t m_name;
        util::RequestSender<BrowserProxy> m_reply;
    };
    m_sender.postNewRequest(new Task(name, m_reply.getSender()));
}

// Select folder and report information.
void
game::proxy::BrowserProxy::selectFolder(OptionalIndex_t index)
{
    class LoadTask : public util::Request<Trampoline> {
     public:
        LoadTask(OptionalIndex_t index, util::RequestSender<BrowserProxy> reply)
            : m_index(index),
              m_reply(reply)
            { }
        void handle(Trampoline& t)
            {
                Browser& b = t.session().browser();
                Folder* f;
                size_t pos = 0;
                bool current = !m_index.get(pos);
                if (current) {
                    f = &b.currentFolder();
                } else {
                    if (pos < b.content().size()) {
                        f = b.content()[pos];
                    } else {
                        f = 0;
                    }
                }

                std::auto_ptr<FolderInfo> result(new FolderInfo());
                if (f != 0) {
                    // Info
                    if (!current) {
                        b.selectChild(pos);
                        t.session().addTask(b.loadChildRoot(std::auto_ptr<Task_t>(new ChildBuilder(m_reply, t.session(), result))));
                    } else {
                        // FIXME: focusing on the browser, not on an item - unselect current item in t.browser()?
                        buildFolderInfo(*f, *result);
                        m_reply.postNewRequest(new UpdateInfoTask(m_index, result));
                    }
                } else {
                    m_reply.postNewRequest(new UpdateInfoTask(m_index, result));
                }
            }

     private:
        OptionalIndex_t m_index;
        util::RequestSender<BrowserProxy> m_reply;
    };
    m_sender.postNewRequest(new LoadTask(index, m_reply.getSender()));
}

// Check whether to suggest setting up a local folder.
bool
game::proxy::BrowserProxy::isSelectedFolderSetupSuggested(WaitIndicator& ind)
{
    struct Task : public util::Request<Trampoline> {
     public:
        Task()
            : m_result(false)
            { }
        virtual void handle(Trampoline& t)
            { m_result = t.session().browser().isSelectedFolderSetupSuggested(); }
        bool m_result;
    };
    Task q;
    ind.call(m_sender, q);
    return q.m_result;
}

// Set local directory, automatically.
void
game::proxy::BrowserProxy::setLocalDirectoryAutomatically()
{
    // TODO: this (and the other setLocalDirectory functions) will asynchronously reload
    // the root, but not signal the upper world that it did so.
    // If the reload actually occurs asynchronously, upper world has no way to know when to proceed.
    // Original code called this using call(), but that only synchronized this Task,
    // not the trail of browser tasks that follow it.
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& t)
            {
                Browser& b = t.session().browser();

                // Update configuration
                b.setSelectedLocalDirectoryAutomatically();

                // Save network.ini and pcc2.ini and reload
                std::auto_ptr<Task_t> t3(Task_t::makeBound(&t.session(), &game::browser::Session::finishTask));
                std::auto_ptr<Task_t> t2(new SaveAccountsTask(t.session(), t3));
                t.session().addTask(b.updateConfiguration(t2));
            }
    };
    m_sender.postNewRequest(new Task());
}

// Set local directory to given name.
void
game::proxy::BrowserProxy::setLocalDirectoryName(const String_t& dirName)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const String_t& dirName)
            : m_dirName(dirName)
            { }
        virtual void handle(Trampoline& t)
            {
                Browser& b = t.session().browser();

                // Update configuration
                b.setSelectedLocalDirectoryName(m_dirName);

                // Save network.ini and pcc2.ini and reload
                std::auto_ptr<Task_t> t3(Task_t::makeBound(&t.session(), &game::browser::Session::finishTask));
                std::auto_ptr<Task_t> t2(new SaveAccountsTask(t.session(), t3));
                t.session().addTask(b.updateConfiguration(t2));
            }
     private:
        String_t m_dirName;
    };
    m_sender.postNewRequest(new Task(dirName));
}

// Set local directory to none.
void
game::proxy::BrowserProxy::setLocalDirectoryNone()
{
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& t)
            {
                Browser& b = t.session().browser();
                if (UserConfiguration* config = b.getSelectedConfiguration()) {
                    (*config)[UserConfiguration::Game_ReadOnly].set(1);

                    // Save network.ini and pcc2.ini and reload
                    std::auto_ptr<Task_t> t3(Task_t::makeBound(&t.session(), &game::browser::Session::finishTask));
                    std::auto_ptr<Task_t> t2(new SaveAccountsTask(t.session(), t3));
                    t.session().addTask(b.updateConfiguration(t2));
                }
            }
    };
    m_sender.postNewRequest(new Task());
}

// Get current configuration.
void
game::proxy::BrowserProxy::getConfiguration(WaitIndicator& ind, Configuration& config)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Configuration& config)
            : m_config(config)
            { }
        virtual void handle(Trampoline& t)
            {
                const Browser& p = t.session().browser();
                const Root* root = p.getSelectedRoot().get();
                const UserConfiguration* config = p.getSelectedConfiguration();
                if (root != 0 && config != 0) {
                    Root::Actions_t as = root->getPossibleActions();
                    if (as.contains(Root::aConfigureCharset)) {
                        m_config.charsetId = (*config)[UserConfiguration::Game_Charset]();
                    }
                    if (as.contains(Root::aConfigureFinished)) {
                        m_config.finished = (*config)[UserConfiguration::Game_Finished]();
                    }
                    if (as.contains(Root::aConfigureReadOnly)) {
                        m_config.readOnly = (*config)[UserConfiguration::Game_ReadOnly]();
                    }
                }
            }
     private:
        Configuration& m_config;
    };
    Task t(config);
    ind.call(m_sender, t);
}

// Change configuration.
void
game::proxy::BrowserProxy::setConfiguration(WaitIndicator& ind, const Configuration& config)
{
    // TODO: should probably reload
    class Task : public util::Request<Trampoline> {
     public:
        explicit Task(const Configuration& config)
            : m_config(config)
            { }
        virtual void handle(Trampoline& t)
            {
                Browser& p = t.session().browser();
                if (UserConfiguration* config = p.getSelectedConfiguration()) {
                    if (const String_t* p = m_config.charsetId.get()) {
                        StringOption& opt = (*config)[UserConfiguration::Game_Charset];
                        opt.set(*p);
                        opt.setSource(ConfigurationOption::Game);
                    }
                    if (const bool* p = m_config.finished.get()) {
                        IntegerOption& opt = (*config)[UserConfiguration::Game_Finished];
                        opt.set(*p);
                        opt.setSource(ConfigurationOption::Game);
                    }
                    if (const bool* p = m_config.readOnly.get()) {
                        IntegerOption& opt = (*config)[UserConfiguration::Game_ReadOnly];
                        opt.set(*p);
                        opt.setSource(ConfigurationOption::Game);
                    }
                    t.session().addTask(p.updateConfiguration(std::auto_ptr<Task_t>(Task_t::makeBound(&t.session(), &game::browser::Session::finishTask))));
                }
            }
     private:
        const Configuration& m_config;
    };
    Task t(config);
    ind.call(m_sender, t);
}

// Add an account.
bool
game::proxy::BrowserProxy::addAccount(WaitIndicator& ind, String_t user, String_t type, String_t host)
{
    // TODO: should probably automatically reload
    class Task : public util::Request<Trampoline> {
     public:
        Task(String_t user, String_t type, String_t host)
            : m_user(user), m_type(type), m_host(host), m_result()
            { }
        void handle(Trampoline& t)
            {
                game::browser::AccountManager& mgr = t.session().accountManager();
                if (mgr.findAccount(m_user, m_type, m_host) != 0) {
                    // Duplicate
                    m_result = false;
                } else {
                    // New account
                    std::auto_ptr<game::browser::Account> acc(new game::browser::Account());
                    acc->setName(afl::string::Format("%s @ %s", m_user, m_host));
                    acc->setUser(m_user);
                    acc->setType(m_type);
                    acc->setHost(m_host);
                    mgr.addNewAccount(acc.release());
                    mgr.save();
                    m_result = true;
                }
            }
        bool get() const
            { return m_result; }
     private:
        String_t m_user;
        String_t m_type;
        String_t m_host;
        bool m_result;
    };
    Task t(user, type, host);
    ind.call(m_sender, t);
    return t.get();
}

// Access underlying file system.
util::RequestSender<afl::io::FileSystem>
game::proxy::BrowserProxy::fileSystem()
{
    class Adaptor : public afl::base::Closure<afl::io::FileSystem&(Trampoline&)> {
     public:
        virtual afl::io::FileSystem& call(Trampoline& t)
            { return t.session().browser().fileSystem(); }
    };
    return m_sender.convert(new Adaptor());
}

// Password result: forward into game thread.
void
game::proxy::BrowserProxy::onPasswordResult(game::browser::UserCallback::PasswordResponse resp)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(const game::browser::UserCallback::PasswordResponse& resp)
            : m_resp(resp)
            { }
        virtual void handle(Trampoline& t)
            { t.sig_passwordResult.raise(m_resp); }
     private:
        const game::browser::UserCallback::PasswordResponse m_resp;
    };
    m_sender.postNewRequest(new Task(resp));
}
