/**
  *  \file game/spec/costsummary.hpp
  *  \brief Class game::spec::CostSummary
  */
#ifndef C2NG_GAME_SPEC_COSTSUMMARY_HPP
#define C2NG_GAME_SPEC_COSTSUMMARY_HPP

#include <vector>
#include "game/spec/cost.hpp"
#include "game/types.hpp"

namespace game { namespace spec {

    /** Itemized cost breakdown list.
        Contains a list of items that represent individual items of a cost or cargo amount. */
    class CostSummary {
     public:
        struct Item {
            // ex WBillItem
            Id_t id;            ///< User-specified item identifier. Not further interpreted by CostSummary.
            int multiplier;     ///< Multiplier to display. Not further interpreted by CostSummary.
            String_t name;      ///< Name of item.
            Cost cost;          ///< Cost of these items.

            Item(Id_t id, int multiplier, String_t name, Cost cost)
                : id(id), multiplier(multiplier), name(name), cost(cost)
                { }
        };

        /** Constructor. Makes empty object. */
        CostSummary();
        ~CostSummary();

        /** Clear object.
            \post getNumItems() == 0 */
        void clear();

        /** Add item.
            \param item Item to add (will be copied) */
        void add(const Item& item);

        /** Get number of items.
            \return number */
        size_t getNumItems() const;

        /** Get item by index.
            \param index Index, [0,getNumItems())
            \return item, valid until next modifying call; null if index was invalid */
        const Item* get(size_t index) const;

        /** Find item by Id.
            Locates the first item with the given Id.
            \param [in]  id     Id to find
            \param [out] pIndex If given as non-null, the index is returned here
            \return item Item if any, valid until next modifying call; null if none found */
        const Item* find(Id_t id, size_t* pIndex) const;

        /** Get total cost.
            \return sum of all costs */
        Cost getTotalCost() const;

     private:
        std::vector<Item> m_items;
    };

} }

#endif
