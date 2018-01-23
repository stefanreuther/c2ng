/**
  *  \file server/interface/hosttool.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTOOL_HPP
#define C2NG_SERVER_INTERFACE_HOSTTOOL_HPP

#include <vector>
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"

namespace server { namespace interface {

    class HostTool : public afl::base::Deletable {
     public:
        struct Info {
            String_t id;
            String_t description;
            String_t kind;
            bool isDefault;

            Info(String_t id, String_t description, String_t kind, bool isDefault)
                : id(id), description(description), kind(kind), isDefault(isDefault)
                { }
        };

        enum Area {
            Host,
            ShipList,
            Master,
            Tool
        };

        // HOSTADD id:Str path:FileName program:Str kind:Str (Host Command)
        virtual void add(String_t id, String_t path, String_t program, String_t kind) = 0;

        // HOSTSET id:Str key:Str value:Str
        virtual void set(String_t id, String_t key, String_t value) = 0;

        // HOSTGET id:Str key:Str
        virtual String_t get(String_t id, String_t key) = 0;

        // HOSTRM id:Str
        virtual bool remove(String_t id) = 0;

        // HOSTLS
        virtual void getAll(std::vector<Info>& result) = 0;

        // HOSTCP src:Str dest:Str
        virtual void copy(String_t sourceId, String_t destinationId) = 0;

        // HOSTDEFAULT id:Str
        virtual void setDefault(String_t id) = 0;

        // HOSTRATING id:Str {{SET n:Int | AUTO} {USE|SHOW}}|NONE|GET
        // - get
        virtual int32_t getDifficulty(String_t id) = 0;
        // - none
        virtual void clearDifficulty(String_t id) = 0;
        // - set/auto use/show
        virtual int32_t setDifficulty(String_t id, afl::base::Optional<int32_t> value, bool use) = 0;

        static const char* toString(Area a);
    };

} }

#endif
