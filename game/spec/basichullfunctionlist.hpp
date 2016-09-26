/**
  *  \file game/spec/basichullfunctionlist.hpp
  */
#ifndef C2NG_GAME_SPEC_BASICHULLFUNCTIONLIST_HPP
#define C2NG_GAME_SPEC_BASICHULLFUNCTIONLIST_HPP

#include <vector>
#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/spec/componentvector.hpp"

namespace game { namespace spec {

    class Hull;

    class BasicHullFunctionList {
     public:
        /** Constructor. */
        BasicHullFunctionList();

        /** Destructor. */
        ~BasicHullFunctionList();

        /** Clear hull functions definitions. */
        void clear();

        /** Load from file. */
        void load(afl::io::Stream& in, afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Get definition of a basic function.
            \param id [in] Identifier of basic function (e.g. 3=HeatsTo50)
            \return definition, or null if none known
            \post return != 0 => return->getId() == id */
        const BasicHullFunction* getFunctionById(int id) const;

        /** Get definition of a basic function by name.
            \param name    [in] Name of function
            \param partial [in] true to accept partial matches (i.e. "cool" matches "CoolsTo50")
            \return definition, or null if none matches */
        const BasicHullFunction* getFunctionByName(String_t name, bool acceptPartialMatch) const;

        /** Add basic function definition.
            \param id    [in] Identifier of function (e.g. 3)
            \param name  [in] Name of function (e.g. "HeatsTo50")
            \return new definition
            \pre !getBasicFunctionById(id)
            \post getBasicFunctionById(id) == return */
        BasicHullFunction* addFunction(int id, String_t name);

        /** Check whether two basic hull function identifiers match.
            \param requestedFunctionId [in] The function we're looking for, e.g. hf_Cloak
            \param foundFunctionId     [in] Function found on the ship, e.g. hf_AdvancedCloak.
            \return true iff requestedFunctionId is equal to foundFunctionId or one of the functions it implies */
        bool matchFunction(int requestedFunctionId, int foundFunctionId) const;

        /** Add a default assignment.
            This models the host's built-in defaults, and therefore cannot handle restricted functions.
            \param hullId Hull that receives a function
            \param basicFunctionId Basic function to assign */
        void addDefaultAssignment(int hullId, int basicFunctionId);

        /** Perform default assignments on a set of hulls.
            \param hulls [in/out] Hulls
            \see addDefaultAssignment */
        void performDefaultAssignments(ComponentVector<Hull>& hulls) const;

     private:
        afl::container::PtrVector<BasicHullFunction> m_functions;

        /** Default assignments. Contains a list of pairs (hull,basicFunctionId). */
        std::vector<std::pair<int,int> > m_defaultAssignments;
    };

} }

#endif
