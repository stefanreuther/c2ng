/**
  *  \file interpreter/directoryfunctions.cpp
  *  \brief Interpreter: File System Directory Access
  */

#include "interpreter/directoryfunctions.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/io/directoryentry.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplecontext.hpp"
#include "interpreter/simplefunction.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using interpreter::Arguments;
using interpreter::Error;
using interpreter::World;
using interpreter::checkStringArg;

namespace {
    /*
     *  DirectoryState - state of a directory iteration operation
     */
    struct DirectoryState : public afl::base::RefCounted {
        Ref<afl::base::Enumerator<Ptr<DirectoryEntry> > > iter;
        Ptr<DirectoryEntry> current;

        DirectoryState(const Ref<afl::base::Enumerator<Ptr<DirectoryEntry> > >& iter,
                       const Ptr<DirectoryEntry>& current)
            : iter(iter),
              current(current)
            {
                // ex IntDirectoryDescriptor::IntDirectoryDescriptor
            }
    };


    /*
     *  DirectoryContext - script binding of a directory iteration
     */

    class DirectoryContext : public interpreter::SimpleContext,
                             public interpreter::Context::ReadOnlyAccessor
    {
     public:
        explicit DirectoryContext(const Ref<DirectoryState>& state);

        // Context:
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual bool next();
        virtual interpreter::Context* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

     private:
        Ref<DirectoryState> m_state;
    };


    /*
     *  DirectoryCallable - result of "DirectoryEntry" function
     *
     *  This is a callable to support iteration, but is not actually callable.
     */

    class DirectoryCallable : public interpreter::CallableValue {
     public:
        DirectoryCallable(Ref<Directory> dir);

        virtual void call(interpreter::Process& proc, afl::data::Segment& args, bool want_result);
        virtual bool isProcedureCall() const;
        virtual int32_t getDimension(int32_t which) const;
        virtual interpreter::Context* makeFirstContext();

        String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;
        virtual DirectoryCallable* clone() const;

     private:
        Ref<Directory> m_dir;
    };


    /*
     *  DirectoryContext
     */

    enum DirectoryProperty {
        idpName,                // Name:String: basename
        idpSize,                // Size:Int: size
        idpType,                // Type:String: 'd', 'f'
        idpPath                 // Path:String: full name
    };

    const interpreter::NameTable DIR_MAP[] = {
        { "NAME", idpName, 0, interpreter::thString },
        { "PATH", idpPath, 0, interpreter::thString },
        { "SIZE", idpSize, 0, interpreter::thInt },
        { "TYPE", idpType, 0, interpreter::thString },
    };

    DirectoryContext::DirectoryContext(const Ref<DirectoryState>& state)
        : m_state(state)
    {
        // ex IntDirectoryContext::IntDirectoryContext
    }

    interpreter::Context::PropertyAccessor* DirectoryContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
    {
        // ex IntDirectoryContext::lookup(const IntNameQuery& name)
        return lookupName(name, DIR_MAP, result) ? this : 0;
    }

    afl::data::Value* DirectoryContext::get(PropertyIndex_t index)
    {
        // ex IntDirectoryContext::get(int32_t index)
        if (m_state->current.get() == 0) {
            // Cannot normally happen with a well-defined Directory implementation
            return 0;
        } else {
            DirectoryEntry* e = m_state->current.get();
            switch (DirectoryProperty(DIR_MAP[index].index)) {
             case idpName:
                /* @q Name:Str (File Property)
                   Name of the item (file), for example, "player9.rst".
                   @since PCC2 2.0.4, PCC2 2.41.1 */
                return interpreter::makeStringValue(e->getTitle());

             case idpSize:
                /* @q Size:Int (File Property)
                   Size of the item (file) in bytes.
                   EMPTY if the property is requested for an item that does not have a size (e.g. a directory).
                   @see FSize()
                   @since PCC2 2.0.4, PCC2 2.41.1 */
                switch (e->getFileType()) {
                 case DirectoryEntry::tFile:
                 case DirectoryEntry::tArchive:
                    return interpreter::makeFileSizeValue(e->getFileSize());

                 case DirectoryEntry::tUnknown:
                 case DirectoryEntry::tDirectory:
                 case DirectoryEntry::tRoot:
                 case DirectoryEntry::tDevice:
                 case DirectoryEntry::tOther:
                    break;
                }
                return 0;

             case idpType:
                /* @q Type:Str (File Property)
                   Type of this item.
                   Contains "f" for regular files, "d" for directories.
                   Other items can produce other values, or EMPTY.
                   @since PCC2 2.0.4, PCC2 2.41.1 */
                switch (e->getFileType()) {
                 case DirectoryEntry::tFile:
                 case DirectoryEntry::tArchive:
                    return interpreter::makeStringValue("f");

                 case DirectoryEntry::tDirectory:
                 case DirectoryEntry::tRoot:
                    return interpreter::makeStringValue("d");

                 case DirectoryEntry::tUnknown:
                 case DirectoryEntry::tDevice:
                 case DirectoryEntry::tOther:
                    break;
                }
                return 0;

             case idpPath:
                /* @q Path:Str (File Property)
                   Path of this item.
                   This is the full name of the file that can be used with {Open} or {DirectoryEntry},
                   for example, "/home/user/game/player3.rst".
                   @since PCC2 2.0.4, PCC2 2.41.1 */
                return interpreter::makeStringValue(e->getPathName());
            }
            return 0;
        }
    }

    bool DirectoryContext::next()
    {
        // IntDirectoryContext::next()
        Ptr<DirectoryEntry> nextEntry;
        if (m_state->iter->getNextElement(nextEntry)) {
            m_state->current = nextEntry;
            return true;
        } else {
            return false;
        }
    }

    interpreter::Context* DirectoryContext::clone() const
    {
        // ex IntDirectoryContext::clone()
        return new DirectoryContext(m_state);
    }

    afl::base::Deletable* DirectoryContext::getObject()
    {
        // ex IntDirectoryContext::getObject()
        return 0;
    }

    void DirectoryContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
    {
        // ex IntDirectoryContext::enumProperties(IntPropertyAcceptor& acceptor)
        acceptor.enumTable(DIR_MAP);
    }

    String_t DirectoryContext::toString(bool /*readable*/) const
    {
        // ex IntDirectoryContext::toString(bool /*readable*/)
        return "#<dir>";
    }

    void DirectoryContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
    {
        rejectStore(out, aux, ctx);
    }


    /*
     *  DirectoryCallable
     */

    DirectoryCallable::DirectoryCallable(Ref<Directory> dir)
        : m_dir(dir)
    {
        // ex IntDirectoryCallable::IntDirectoryCallable
    }

    void DirectoryCallable::call(interpreter::Process& /*proc*/, afl::data::Segment& /*args*/, bool /*want_result*/)
    {
        throw Error::typeError(Error::ExpectCallable);
    }

    bool DirectoryCallable::isProcedureCall() const
    {
        return false;
    }

    int32_t DirectoryCallable::getDimension(int32_t /*which*/) const
    {
        return 0;
    }

    interpreter::Context* DirectoryCallable::makeFirstContext()
    {
        Ref<afl::base::Enumerator<Ptr<DirectoryEntry> > > iter = m_dir->getDirectoryEntries();
        Ptr<DirectoryEntry> current;
        if (iter->getNextElement(current)) {
            return new DirectoryContext(*new DirectoryState(iter, current));
        } else {
            return 0;
        }
    }

    String_t DirectoryCallable::toString(bool /*readable*/) const
    {
        return "#<directory>";
    }

    void DirectoryCallable::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
    {
        rejectStore(out, aux, ctx);
    }

    DirectoryCallable* DirectoryCallable::clone() const
    {
        return new DirectoryCallable(m_dir);
    }


    /*
     *  User Entry Point
     */

    /* @q DirectoryEntry(n:Str):Obj (Function)
       Access directory content.
       Use as
       | ForEach DirectoryEntry(Name) Do ...
       passing a directory name as %Name.

       This call will return all files and directories contained in the given directory,
       one per loop iteration, where you can access {int:index:group:fileproperty|its properties}.
       The files and directories are returned in an arbitrary order.
       The "." and ".." entries are not returned.

       @see int:index:group:fileproperty|File Properties
       @since PCC2 2.0.4, PCC2 2.41.1 */
    afl::data::Value* IFDirectoryEntry(World& world, Arguments& args)
    {
        args.checkArgumentCount(1);

        String_t dirName;
        if (!checkStringArg(dirName, args.getNext())) {
            return 0;
        }

        return new DirectoryCallable(world.fileSystem().openDirectory(dirName));
    }
}

// Register directory-related functions on a World instance.
void
interpreter::registerDirectoryFunctions(World& world)
{
    typedef SimpleFunction<World&> FunctionValue_t;
    world.setNewGlobalValue("DIRECTORYENTRY", new FunctionValue_t(world, IFDirectoryEntry));
}
