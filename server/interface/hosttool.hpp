/**
  *  \file server/interface/hosttool.hpp
  *  \brief Interface server::interface::HostTool
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTOOL_HPP
#define C2NG_SERVER_INTERFACE_HOSTTOOL_HPP

#include <vector>
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    /** Interface to access tools.
        These operations manipulate the set of host versions, master versions, ship lists, or add-ons
        available on the server. */
    class HostTool : public afl::base::Deletable {
     public:
        /** Information about a tool. */
        struct Info {
            String_t id;                    ///< Id (sometimes referred to as name).
            String_t description;           ///< Description (human-readable short name).
            String_t kind;                  ///< Tool kind (machine-readable group name).
            bool isDefault;                 ///< true if this tool is marked as default.

            Info(String_t id, String_t description, String_t kind, bool isDefault)
                : id(id), description(description), kind(kind), isDefault(isDefault)
                { }
        };

        /** Tool area. */
        enum Area {
            Host,                           ///< Host version (HOSTxxx commands).
            ShipList,                       ///< Shiplist (SHIPLISTxxx commands).
            Master,                         ///< Master version (MASTERxxx commands).
            Tool                            ///< Generic add-on (TOOLxxx commands).
        };

        /** Add a tool (HOSTADD etc.).
            @param id         Id
            @param path       Path on host filer
            @param program    Program name
            @param kind       Tool kind */
        virtual void add(String_t id, String_t path, String_t program, String_t kind) = 0;

        /** Set tool property (HOSTSET etc.).
            @param id         Id
            @param key        Property name
            @param value      New property value */
        virtual void set(String_t id, String_t key, String_t value) = 0;

        /** Get tool property (HOSTGET etc.).
            @param id         Id
            @param key        Property name
            @return Property value */
        virtual String_t get(String_t id, String_t key) = 0;

        /** Remove a tool (HOSTRM etc.).
            @param id         Id
            @return true if tool removed, false if did not exist */
        virtual bool remove(String_t id) = 0;

        /** Get list of tools (HOSTLS etc.).
            @param [out] result  List */
        virtual void getAll(std::vector<Info>& result) = 0;

        /** Copy a tool (HOSTCP etc.).
            @param sourceId      Copy this tool definition...
            @param destinationId ...to this name */
        virtual void copy(String_t sourceId, String_t destinationId) = 0;

        /** Set default version (HOSTDEFAULT etc.).
            @param id   Id of new default host/master/ship list */
        virtual void setDefault(String_t id) = 0;

        /** Get tool difficulty (HOSTRATING...GET etc.).
            @param id   Id
            @return difficulty rating */
        virtual int32_t getDifficulty(String_t id) = 0;

        /** Clear fixed tool difficulty, use default (HOSTRATING...NONE etc.).
            @param id   Id */
        virtual void clearDifficulty(String_t id) = 0;

        /** Set fixed tool difficulty (HOSTRATING...AUTO, HOSTRATING...SET).
            @param id     Id
            @param value  Difficulty to set; empty to set default
            @param use    true to use difficulty in game rating; false to show it but use computed default
            @return new difficulty ragin */
        virtual int32_t setDifficulty(String_t id, afl::base::Optional<int32_t> value, bool use) = 0;

        /** Format Area as string.
            @param a Area
            @return formatted value (command name prefix) or null */
        static const char* toString(Area a);
    };

} }

#endif
