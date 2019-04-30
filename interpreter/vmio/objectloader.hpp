/**
  *  \file interpreter/vmio/objectloader.hpp
  */
#ifndef C2NG_INTERPRETER_VMIO_OBJECTLOADER_HPP
#define C2NG_INTERPRETER_VMIO_OBJECTLOADER_HPP

#include "afl/base/uncopyable.hpp"
#include "afl/io/stream.hpp"
#include "afl/charset/charset.hpp"
#include "afl/container/ptrmap.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/hashvalue.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/vmio/loadcontext.hpp"
#include "afl/base/ref.hpp"

namespace interpreter { namespace vmio {

    class LoadContext;

    // /** Context to load a virtual machine state subset.

    //     Note that this stores a global instance pointer for use by
    //     serialisation functions; thus is is reentrant, but not
    //     thread-safe. */
    class ObjectLoader : public afl::base::Uncopyable, private LoadContext {
     public:
        ObjectLoader(afl::charset::Charset& cs, LoadContext& ctx);
        ~ObjectLoader();

        BCORef_t loadObjectFile(afl::base::Ref<afl::io::Stream> s);

        void load(afl::base::Ref<afl::io::Stream> s);

        BCORef_t getBCO(uint32_t id);
        afl::data::Hash::Ref_t getHash(uint32_t id);
        afl::base::Ref<ArrayData> getArray(uint32_t id);
        afl::base::Ref<StructureValueData> getStructureValue(uint32_t id);
        afl::base::Ref<StructureTypeData> getStructureType(uint32_t id);

     private:

        class LoadObject;

        // LoadContext:
        virtual afl::data::Value* loadBCO(uint32_t id);
        virtual afl::data::Value* loadArray(uint32_t id);
        virtual afl::data::Value* loadHash(uint32_t id);
        virtual afl::data::Value* loadStructureValue(uint32_t id);
        virtual afl::data::Value* loadStructureType(uint32_t id);
        virtual Context* loadContext(const TagNode& tag, afl::io::Stream& aux);
        virtual Context* loadMutex(const String_t& name, const String_t& note, Process* owner);
        virtual Process* createProcess();
        virtual void finishProcess(Process& proc);

        // Loaded objects. Indexed by object Id.
        afl::container::PtrMap<uint32_t, SubroutineValue> bco_map;
        afl::container::PtrMap<uint32_t, HashValue> hash_map;
        afl::container::PtrMap<uint32_t, ArrayValue> array_map;
        afl::container::PtrMap<uint32_t, StructureValue> struct_value_map;
        afl::container::PtrMap<uint32_t, StructureType> struct_type_map;

        void loadBCO(LoadObject& ldr, uint32_t id);
        void loadHash(LoadObject& ldr, uint32_t id);
        void loadArray(LoadObject& ldr, uint32_t id);
        void loadStructureValue(LoadObject& ldr, uint32_t id);
        void loadStructureType(LoadObject& ldr, uint32_t id);
        void loadProcess(LoadObject& ldr);
        void loadFrames(Process& proc, LoadContext& ctx, afl::io::Stream& s, uint32_t count);

        afl::charset::Charset& m_charset;
        LoadContext& m_context;
    };

} }

#endif
