/**
  *  \file interpreter/vmio/filesavecontext.hpp
  *  \brief Class interpreter::vmio::FileSaveContext
  */
#ifndef C2NG_INTERPRETER_VMIO_FILESAVECONTEXT_HPP
#define C2NG_INTERPRETER_VMIO_FILESAVECONTEXT_HPP

#include <map>
#include "interpreter/savecontext.hpp"
#include "afl/io/stream.hpp"
#include "afl/base/uncopyable.hpp"
#include "interpreter/process.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/charset/charset.hpp"

namespace interpreter { namespace vmio {

    /** Save context, full version.
        A FileSaveContext allows saving data into a file in the form of a sequence of binary objects.
        Saving interconnected data will preserve sharing, i.e. two pointers to the same object will save that object just once.
        Therefore, the save process is two-step:
        - add all objects using "add" functions (addProcess, functions from SaveContext) to build a plan
        - save everything using save(). */
    class FileSaveContext : public SaveContext, private afl::base::Uncopyable {
     public:
        /** Constructor.
            \param cs Character set */
        explicit FileSaveContext(afl::charset::Charset& cs);

        /** Destructor. */
        ~FileSaveContext();

        /** Enable/disable debug information.
            When enabled (default), debug information from bytecode objects will be saved.
            \param enable flag */
        void setDebugInformation(bool enable);

        /** Get number of prepared objects.
            \return number of objects */
        size_t getNumPreparedObjects() const;

        /** Add process object.
            Note that, unlike bytecode or data, a process can be saved many times.
            There is no way to reference a process within an object pool file,
            hence we don't offer a way to unify processes.
            Loading will then create multiple copies of it.
            \param proc The process object */
        void addProcess(const Process& proc);

        /** Save as object file.
            An object file is just an object pool with a minimum header, with no reference to game data.
            It cannot contain processes, just compiled code.
            As of 2018, object files are mostly used for testing;
            our compiler is fast enough that scripts are compiled each time when used.
            \param out Stream to save to
            \param entry ID of entry-point BCO (obtained by adding it with addBCO()) */
        void saveObjectFile(afl::io::Stream& out, uint32_t entry);

        /** Save all pending objects.
            \param out Stream to save to */
        void save(afl::io::Stream& out);

        // SaveContext:
        virtual uint32_t addBCO(const BytecodeObject& bco);
        virtual uint32_t addHash(const afl::data::Hash& hash);
        virtual uint32_t addArray(const ArrayData& array);
        virtual uint32_t addStructureType(const StructureTypeData& type);
        virtual uint32_t addStructureValue(const StructureValueData& value);
        virtual bool isCurrentProcess(const Process* p);

     private:
        afl::charset::Charset& m_charset;
        bool m_debugInformationEnabled;

        // Map of object to Id. The object can be anything (process, bytecode, array, hash).
        std::map<const void*,uint32_t> m_objectToId;

        // Object Id counter. Contains the last Id assigned.
        uint32_t m_objectIdCounter;

        // Save plan.
        class Saver {
         public:
            virtual ~Saver()
                { }
            virtual void save(afl::io::Stream& out, FileSaveContext& parent) = 0;
        };
        afl::container::PtrVector<Saver> m_plan;

        /** Add to plan.
            \param p Newly-allocated saver. Never null. */
        void addPlanNew(Saver* p);

        /** Save a bytecode object.
            \param out  Stream to write to
            \param bco  Bytecode object to save
            \param id   Id to assign this BCO */
        void saveBCO(afl::io::Stream& out, const BytecodeObject& bco, uint32_t id);

        /** Save a hash object.
            \param out Stream to save to
            \param hash Hash to save
            \param id Id to use */
        void saveHash(afl::io::Stream& out, const afl::data::Hash& hash, uint32_t id);

        /** Save an array object.
            \param out Stream to save to
            \param array Array to save
            \param id Id to use */
        void saveArray(afl::io::Stream& out, const ArrayData& array, uint32_t id);

        /** Save a structure type.
            \param out Stream to save to
            \param type Type to save
            \param id Id to use */
        void saveStructureType(afl::io::Stream& out, const StructureTypeData& type, uint32_t id);

        /** Save a structure value.
            \param out Stream to save to
            \param value Value to save
            \param id Id to use */
        void saveStructureValue(afl::io::Stream& out, const StructureValueData& value, uint32_t id);

        /** Save a stack frame object.
            \param out Stream to save to
            \param fr  Stack frame to save */
        void saveFrame(afl::io::Stream& out, const Process::Frame& fr);

        /** Save process.
            \param out Stream to save to
            \param exc Process to save */
        void saveProcess(afl::io::Stream& out, const Process& proc);
    };

} }

#endif
