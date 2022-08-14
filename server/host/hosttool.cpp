/**
  *  \file server/host/hosttool.cpp
  *  \brief Class server::host::HostTool
  */

#include <stdexcept>
#include "server/host/hosttool.hpp"
#include "afl/base/countof.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "afl/string/format.hpp"
#include "game/maint/difficultyrater.hpp"
#include "server/errors.hpp"
#include "server/file/clientdirectory.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "util/math.hpp"

namespace {
    const char LOG_NAME[] = "host.tool";

    /** Validate a tool identifier. Since these are used in shell variable
        names and URLs, they'd better be sane. We allow alphanumerics,
        ".-+_", which should be safe. Although they only come from
        controlled sources (admin installing new components), catching
        errors is a good idea.

        \param id Id to check
        \param shellVar true if this is going to be a shell variable (for "kind"), false otherwise
        \return true if Id is valid */
    bool isValidIdentifier(const String_t& id, bool shellVar)
    {
        if (id.empty()) {
            return false;
        }
        for (String_t::size_type i = 0; i < id.size(); ++i) {
            const char ch = id[i];
            if (!((ch >= 'A' && ch <= 'Z')
                  || (ch >= 'a' && ch <= 'z')
                  || (ch >= '0' && ch <= '9')
                  || (ch == '_')
                  || (!shellVar && (ch == '-' || ch == '.' || ch == '+' ))))
            {
                return false;
            }
        }
        return true;
    }

    /** Validate executable.

        @change c2host-classic would set the "+x" bit directly in the filespace here.
        We are doing that on export (Exporter::exportTool).
        We therefore only need to verify that the executable exists at this point.
        Note that this is just a gratuitous sanity check; nothing stops the user from deleting the file after adding the tool.

        \param filer HostFile instance
        \param name Name of executable
        \param log Logger

        \retval true Executable is valid
        \retval false Executable did not exist or is not a file */
    bool validateExecutable(afl::net::CommandHandler& filer, const String_t name, afl::sys::LogListener& log)
    {
        server::interface::BaseClient(filer).setUserContext(String_t());
        try {
            server::interface::FileBase::Info info = server::interface::FileBaseClient(filer).getFileInformation(name);
            return (info.type == server::interface::FileBase::IsFile);
        }
        catch (std::exception& e) {
            log.write(afl::sys::LogListener::Warn, LOG_NAME, name, e);
            return false;
        }
    }

    /** Compute rating of a tool, given a directory.
        \param filer HostFile instance
        \param dir Directory in that file
        \param log Logger
        \return rating, 1..100 */
    int computeToolRating(afl::net::CommandHandler& filer, String_t dir, afl::sys::LogListener& log)
    {
        // ex planetscentral/host/rating.cc:computeToolRating
        try {
            // Set unrestricted context on filer.
            // computeToolRating() is only permitted for admin users, thus this is fine.
            server::interface::BaseClient(filer).setUserContext(String_t());

            // Create a Directory
            afl::base::Ref<server::file::ClientDirectory> dirWrapper = server::file::ClientDirectory::create(filer, dir);

            // Assume it is a ship list and rate as such
            game::maint::DifficultyRater rater;
            rater.addShipList(*dirWrapper);

            // Configuration files. Note that this is a close relative to addConfigDirectory().
            // It is important that we process the ".frag" after the original files.
            // It is not a problem if we process both AMaster and PMaster config files, as
            // they're expected to have the same content. A problem would be a tool containing
            // both amaster.src and pmaster.cfg.frag (but no amaster.src.frag and pmaster.cfg),
            // but this doesn't happen with any sane tool.
            static const char*const files[] = {
                "amaster.src",  "amaster.src.frag",
                "pconfig.src",  "pconfig.src.frag",
                "pmaster.cfg",  "pmaster.cfg.frag",
                "shiplist.txt", "shiplist.txt.frag",
            };
            for (size_t i = 0; i < countof(files); ++i) {
                afl::base::Ptr<afl::io::Stream> s = dirWrapper->openFileNT(files[i], afl::io::FileSystem::OpenRead);
                if (s.get() != 0) {
                    rater.addConfigurationFile(*s);
                }
            }
            return util::roundToInt(100 * rater.getTotalRating());
        }
        catch (afl::except::FileProblemException& e) {
            log.write(log.Warn, LOG_NAME, afl::string::Format("tool rating '%s'", dir), e);
            return 100;
        }
    }
}

server::host::HostTool::HostTool(const Session& session, Root& root, Root::ToolTree tree)
    : m_session(session),
      m_root(root),
      m_tree(tree)
{ }

void
server::host::HostTool::add(String_t id, String_t path, String_t program, String_t kind)
{
    // ex doHostAdd
    m_session.checkAdmin();
    if (!isValidIdentifier(id, false)) {
        // invalid id
        throw std::runtime_error(INVALID_IDENTIFIER);
    } else if (!isValidIdentifier(kind, true)) {
        // invalid kind
        throw std::runtime_error(INVALID_IDENTIFIER);
    } else if (!program.empty() && !validateExecutable(m_root.hostFile(), path + "/" + program, m_root.log())) {
        // invalid executable
        throw std::runtime_error(INVALID_EXECUTABLE);
    } else {
        // all tests pass, add it
        afl::net::redis::HashKey h(m_tree.byName(id));
        h.stringField("path").set(path);
        h.stringField("program").set(program);
        h.stringField("kind").set(kind);
        m_tree.all().add(id);
        if (m_tree.defaultName().get().empty()) {
            m_tree.defaultName().set(id);
        }
    }
}

void
server::host::HostTool::set(String_t id, String_t key, String_t value)
{
    // doHostSet
    // FIXME: validate that <id> exists? validate <key>?
    m_session.checkAdmin();
    if (!isValidIdentifier(id, false)) {
        throw std::runtime_error(INVALID_IDENTIFIER);
    }
    m_tree.byName(id).stringField(key).set(value);
}

String_t
server::host::HostTool::get(String_t id, String_t key)
{
    // ex doHostGet
    // FIXME: validate that <id> exists? validate <key>?
    // Not critical; although users can fetch arbitrary keys using
    // "toolinfo.cgi/host/some-random-string", they cannot do anything
    // evil with it.
    return m_tree.byName(id).stringField(key).get();
}

bool
server::host::HostTool::remove(String_t id)
{
    // ex doHostRemove
    m_session.checkAdmin();

    bool result = m_tree.all().remove(id);
    if (result) {
        m_tree.byName(id).remove();
    }

    if (m_tree.defaultName().get() == id) {
        // This was the default, pick another one
        m_tree.defaultName().set(m_tree.all().getRandom());
    }

    return result;
}

void
server::host::HostTool::getAll(std::vector<Info>& result)
{
    // ex doHostList
    String_t def = m_tree.defaultName().get();

    afl::data::StringList_t hostList;
    m_tree.all().getAll(hostList);

    for (size_t i = 0, n = hostList.size(); i < n; ++i) {
        const String_t& id = hostList[i];
        afl::net::redis::HashKey h(m_tree.byName(id));
        result.push_back(Info(id, h.stringField("description").get(), h.stringField("kind").get(), def == id));
    }
}

void
server::host::HostTool::copy(String_t sourceId, String_t destinationId)
{
    // ex doHostCopy
    m_session.checkAdmin();

    afl::net::redis::StringSetKey list(m_tree.all());

    // Validate source
    if (!list.contains(sourceId)) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }
    if (!isValidIdentifier(destinationId, false)) {
        throw std::runtime_error(INVALID_IDENTIFIER);
    }

    // Create copy
    afl::data::StringList_t data;
    m_tree.byName(sourceId).getAll(data);
    m_tree.byName(destinationId).remove();
    m_tree.byName(destinationId).setAll(data);

    list.add(destinationId);
}

void
server::host::HostTool::setDefault(String_t id)
{
    // ex doHostDefault
    m_session.checkAdmin();

    // Validate
    if (!m_tree.all().contains(id)) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    // Set default
    m_tree.defaultName().set(id);
}

int32_t
server::host::HostTool::getDifficulty(String_t id)
{
    // ex doHostRating
    m_session.checkAdmin();     // FIXME: needed?

    if (!m_tree.all().contains(id)) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    afl::net::redis::IntegerField field(m_tree.byName(id).intField("difficulty"));
    return field.exists() ? field.get() : 0;
}

void
server::host::HostTool::clearDifficulty(String_t id)
{
    // ex doHostRating
    m_session.checkAdmin();

    if (!m_tree.all().contains(id)) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    afl::net::redis::HashKey tool(m_tree.byName(id));
    tool.field("difficulty").remove();
    tool.field("useDifficulty").remove();
}

int32_t
server::host::HostTool::setDifficulty(String_t id, afl::base::Optional<int32_t> value, bool use)
{
    m_session.checkAdmin();

    if (!m_tree.all().contains(id)) {
        throw std::runtime_error(ITEM_NOT_FOUND);
    }

    afl::net::redis::HashKey tool(m_tree.byName(id));

    int32_t actualValue;
    if (const int32_t* p = value.get()) {
        // Value has been given
        actualValue = *p;
    } else {
        // Compute the value
        actualValue = computeToolRating(m_root.hostFile(), tool.stringField("path").get(), m_root.log());
    }
    if (actualValue < 1) {
        actualValue = 1;
    }
    if (actualValue > 1000) {
        actualValue = 1000;
    }

    tool.intField("difficulty").set(actualValue);
    tool.intField("useDifficulty").set(use);

    return actualValue;
}
