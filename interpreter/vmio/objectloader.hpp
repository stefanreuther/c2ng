/**
  *  \file interpreter/vmio/objectloader.hpp
  *  \brief Class interpreter::vmio::ObjectLoader
  */
#ifndef C2NG_INTERPRETER_VMIO_OBJECTLOADER_HPP
#define C2NG_INTERPRETER_VMIO_OBJECTLOADER_HPP

#include "afl/base/ref.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/charset/charset.hpp"
#include "afl/container/ptrmap.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/hashvalue.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/vmio/loadcontext.hpp"

namespace interpreter { namespace vmio {

    class LoadContext;

    /** Context to load a virtual machine state subset.
        An ObjectLoader loads a structured object file as used for VM and compiled-object files.
        It can resolve interdependencies between objects (e.g. BytecodeObject referencing a StructureValue object).
        It uses a LoadContext to resolve further objects (e.g. create processes, contexts).
        The LoadContext defines whether ObjectLoader can load processes.

        This class holds state (object Ids), and an instance can therefore be used to load only one file.
        - construct object
        - call loadObjectFile() or load()

        Forward references can be resolved (e.g. a cyclic references: structure values pointing at each other).

        Undefined references (e.g. an object Id that is not defined in the file, or defined with the wrong type)
        are implicitly treated as forward references and are resolved by using a dummy object in place of the actual object.
        For now, this is not treated as an error. */
    class ObjectLoader : public afl::base::Uncopyable, private LoadContext {
     public:
        /** Constructor.
            \param cs Game charset
            \param tx Translator (for error messages)
            \param ctx LoadContext for structured values */
        ObjectLoader(afl::charset::Charset& cs, afl::string::Translator& tx, LoadContext& ctx);

        /** Destructor. */
        ~ObjectLoader();

        /** Load object (*.qc) file.
            \param s Stream
            \return reference to entry-point object (i.e. to run this object file, use Process::pushFrame with this object). */
        BCORef_t loadObjectFile(afl::base::Ref<afl::io::Stream> s);

        /** Load virtual-machine file.
            The file should contain process objects; the LoadContext should implement createProcess.
            The side effect of this call will be that those processes are created.
            \param s Stream */
        void load(afl::base::Ref<afl::io::Stream> s);

        /** Get bytecode object by Id.
            If there is a BCO with the given Id, return it; otherwise, create a blank one.
            \param id Object Id
            \return BCO */
        BCORef_t getBCO(uint32_t id);

        /** Get hash object by Id.
            If there is a hash object with the given Id, return it; otherwise, create a blank one.
            \param id Object Id
            \return hash object */
        afl::data::Hash::Ref_t getHash(uint32_t id);

        /** Get array object by Id.
            If there is an array object with the given Id, return it; otherwise, create a blank one.
            \param id Object Id
            \return array object */
        afl::base::Ref<ArrayData> getArray(uint32_t id);

        /** Get structure value object by Id.
            If there is a structure value with the given Id, return it; otherwise, create a blank one.
            \param id Object Id
            \return structure value */
        afl::base::Ref<StructureValueData> getStructureValue(uint32_t id);

        /** Get structure type object by Id.
            If there is a structure type with the given Id, return it; otherwise, create a blank one.
            \param id Object Id
            \return structure type */
        afl::base::Ref<StructureTypeData> getStructureType(uint32_t id);

     private:
        class ChunkLoader;

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
        afl::container::PtrMap<uint32_t, SubroutineValue> m_BCOsById;
        afl::container::PtrMap<uint32_t, HashValue> m_HashById;
        afl::container::PtrMap<uint32_t, ArrayValue> m_ArrayById;
        afl::container::PtrMap<uint32_t, StructureValue> m_StructureValueById;
        afl::container::PtrMap<uint32_t, StructureType> m_StructureTypeById;

        void loadBCO(ChunkLoader& ldr, uint32_t id);
        void loadHash(ChunkLoader& ldr, uint32_t id);
        void loadArray(ChunkLoader& ldr, uint32_t id);
        void loadStructureValue(ChunkLoader& ldr, uint32_t id);
        void loadStructureType(ChunkLoader& ldr, uint32_t id);
        void loadProcess(ChunkLoader& ldr, afl::io::Stream& outerStream);
        void loadFrames(Process& proc, LoadContext& ctx, afl::io::Stream& s, uint32_t count);

        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;
        LoadContext& m_context;
    };

} }

#endif
