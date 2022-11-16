/**
  *  \file game/specificationloader.hpp
  *  \brief Interface game::SpecificationLoader
  */
#ifndef C2NG_GAME_SPECIFICATIONLOADER_HPP
#define C2NG_GAME_SPECIFICATIONLOADER_HPP

#include <memory>
#include "afl/base/deletable.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/io/stream.hpp"
#include "game/spec/shiplist.hpp"
#include "game/task.hpp"
#include "afl/base/ref.hpp"

namespace game {

    class Root;

    /** Interface to load specification files. */
    class SpecificationLoader : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Load ship list.
            \param [out]     list   Ship list
            \param [in,out]  root   Root
            \param [in]      then   Task to execute after loading

            This function is passed a Root, mainly for consulatation of configuration stuff.
            However, loading the specification may provide new information (e.g. Nu provides server-side race names). */
        virtual std::auto_ptr<Task_t> loadShipList(game::spec::ShipList& list, Root& root, std::auto_ptr<StatusTask_t> then) = 0;

        /** Open file in the specification directory.
            Looks for the file in all appropriate places, in correct order.
            Opens the file for reading.
            \param fileName  Name
            \return file stream
            \throw FileProblemException if file does not exist */
        virtual afl::base::Ref<afl::io::Stream> openSpecificationFile(const String_t& fileName) = 0;
    };

}

#endif
