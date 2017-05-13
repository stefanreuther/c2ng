/**
  *  \file game/spec/modifiedhullfunctionlist.hpp
  *  \brief Class game::spec::ModifiedHullFunctionList
  */
#ifndef C2NG_GAME_SPEC_MODIFIEDHULLFUNCTIONLIST_HPP
#define C2NG_GAME_SPEC_MODIFIEDHULLFUNCTIONLIST_HPP

#include "afl/container/ptrvector.hpp"
#include "game/spec/hullfunction.hpp"

namespace game { namespace spec {

    /** List of modified hull functions.
        A modified hull function is a basic hull function with a level restriction.
        The host uses integer values to represent a modified hull function, and transmits its mapping in util.dat record 57.
        We also use integer values (Function_t) to represent modified hull functions.
        This class' job is to track host's mapping as well as our own.

        We guarantee that for an unrestricted function, the basicFunctionId directly maps to the Function_t;
        this is convenient at several places. */
    class ModifiedHullFunctionList {
     public:
        // Use this type definition to check for type mismatches:
        // enum Function_t { dev_t_min = -1, dev_t_max = 999999 };
        typedef int32_t Function_t;

        /** Constructor. */
        ModifiedHullFunctionList();

        /** Destructor. */
        ~ModifiedHullFunctionList();

        /** Clear.
            This forgets all content and invalidates all previously-stored Function_t values. */
        void clear();

        /** Given a host-supplied device Id, return equivalent internal Id.
            If the hostFunctionId represents a modified hull function,
            the host's mapping must already have been processed by getFunctionIdFromDefinition().
            \param hostFunctionId Function Id given by host
            \return equivalent internal Id */
        Function_t getFunctionIdFromHostId(int hostFunctionId) const;

        /** Given a function definition, return equivalent internal Id (and update state).
            If there is not yet an internal Id for that definition, allocate one.
            If that definition reports a Host Id for an existing definition, register that one as well.
            \param def Function definition with basicFunctionId, levels, and hostId fields valid
            \return internal Id */
        Function_t getFunctionIdFromDefinition(const HullFunction& def);

        /** Return definition of a hull function.
            \param id   [in] Internal Id
            \param def  [out] Definition of the function. This is always filled in; if the
                        internal Id is invalid, it is filled with an invalid function
                        definition. Note that it is possible to have valid function assignments
                        that refer to a basic function whose definition we don't know, i.e.
                        getBasicFunctionById(asgn.getBasicFunctionId()) == 0.
            \retval true  id was valid; def was set to a valid definition
            \retval false id was not valid; def not set to a valid definition */
        bool getFunctionDefinition(Function_t id, HullFunction& def) const;

     private:
        /** Modified hull functions.
            This defines the modified (=level-restricted) hull functions.
            The \c players and \c kind fields are irrelevant here and remain at defaults. */
        afl::container::PtrVector<HullFunction> m_modifiedFunctions;
    };

} }

#endif
