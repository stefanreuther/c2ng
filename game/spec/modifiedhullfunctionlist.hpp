/**
  *  \file game/spec/modifiedhullfunctionlist.hpp
  */
#ifndef C2NG_GAME_SPEC_MODIFIEDHULLFUNCTIONLIST_HPP
#define C2NG_GAME_SPEC_MODIFIEDHULLFUNCTIONLIST_HPP

#include "afl/container/ptrvector.hpp"
#include "game/spec/hullfunction.hpp"

namespace game { namespace spec {

    class ModifiedHullFunctionList {
     public:
        // Use this type definition to check for type mismatches:
        // enum Function_t { dev_t_min = -1, dev_t_max = 999999 };
        typedef int32_t Function_t;

        ModifiedHullFunctionList();

        ~ModifiedHullFunctionList();

        /** Clear.
            This forgets all content and invalidates all previously-stored Function_t values. */
        void clear();

        /** Given a host-supplied device Id, return equivalent internal Id. */
        Function_t getFunctionIdFromHostId(int hostFunctionId) const;

        /** Given a function definition, return equivalent internal Id.
            If there is not yet an internal Id for that definition, allocate one.
            If that definition reports a Host Id for an existing definition,
            register that one as well.
            \param def Function definition with basicFunctionId, levels, and hostId fields valid
            \return internal Id */
        Function_t getFunctionIdFromDefinition(const HullFunction& def);

        /** Return definition of a hull function.
            \param id   [in] Internal Id
            \param asgn [out] Definition of the function. This is always filled in; if the
                        internal Id is invalid, it is filled with an invalid function
                        definition. Note that it is possible to have valid function assignments
                        that refer to a basic function whose definition we don't know, i.e.
                        getBasicFunctionById(asgn.getBasicFunctionId()) == 0.
            \return true if valid internal Id */
        bool getFunctionDefinition(Function_t id, HullFunction& def) const;

     private:
        /** Modified hull functions.
            This defines the modified (=level-restricted) hull functions.
            The \c players and \c kind fields are irrelevant here and remain at defaults. */
        afl::container::PtrVector<HullFunction> m_modifiedFunctions;
    };

} }

#endif
