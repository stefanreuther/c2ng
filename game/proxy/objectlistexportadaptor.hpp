/**
  *  \file game/proxy/objectlistexportadaptor.hpp
  *  \brief Class game::proxy::ObjectListExportAdaptor
  */
#ifndef C2NG_GAME_PROXY_OBJECTLISTEXPORTADAPTOR_HPP
#define C2NG_GAME_PROXY_OBJECTLISTEXPORTADAPTOR_HPP

#include "afl/base/ref.hpp"
#include "game/config/stringoption.hpp"
#include "game/proxy/exportadaptor.hpp"
#include "game/session.hpp"

namespace game { namespace proxy {

    /** ExportAdaptor for a list of objects.
        Allows exporting of a subset of objects of a given type.
        The subset is given as a list of Ids.

        Depending on the object type, the exporter configuration is persisted in different UserConfiguration keys (initConfiguration, saveConfiguration).
        FileSystem and Translator are forwarded from the given Session.
        The Context is a custom context publishing the requested subset of objects. */
    class ObjectListExportAdaptor : public ExportAdaptor {
     public:
        class Context;

        /** Mode (object type). */
        enum Mode {
            Ships,              ///< Publish ships; use ExportShipFields.
            Planets             ///< Publish planets; use ExportPlanetFields.
        };

        /** Constructor.
            @param session  Session
            @param mode     Mode
            @param ids      List of Ids. Should not be empty.
                            If the list is sorted and has no duplicates, we guarantee to provide content in the same order.
                            If the list is not sorted, or duplicates appear, we make no guarantees other than every
                            mentioned object will be reported at least once.
                            If Ids of non-existant objects are mentioned, we make no guarantees. */
        ObjectListExportAdaptor(Session& session, Mode mode, const std::vector<Id_t>& ids);

        // ExportAdaptor:
        virtual void initConfiguration(interpreter::exporter::Configuration& config);
        virtual void saveConfiguration(const interpreter::exporter::Configuration& config);
        virtual interpreter::Context* createContext();
        virtual afl::io::FileSystem& fileSystem();
        virtual afl::string::Translator& translator();

     private:
        struct Data : public afl::base::RefCounted {
            Session& session;
            const Mode mode;
            const std::vector<Id_t> ids;
            Data(Session& session, Mode mode, const std::vector<Id_t>& ids)
                : session(session), mode(mode), ids(ids)
                { }
        };
        afl::base::Ref<Data> m_data;

        game::config::StringOption* getOption() const;
    };

} }

#endif
