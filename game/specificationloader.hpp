/**
  *  \file game/specificationloader.hpp
  *  \brief Interface game::SpecificationLoader
  */
#ifndef C2NG_GAME_SPECIFICATIONLOADER_HPP
#define C2NG_GAME_SPECIFICATIONLOADER_HPP

#include <memory>
#include "afl/base/deletable.hpp"
#include "afl/base/refcounted.hpp"
#include "game/spec/shiplist.hpp"
#include "game/task.hpp"

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
    };

}

#endif
