/**
  *  \file util/configurationfile.hpp
  *  \brief Class util::ConfigurationFile
  */
#ifndef C2NG_UTIL_CONFIGURATIONFILE_HPP
#define C2NG_UTIL_CONFIGURATIONFILE_HPP

#include "afl/string/string.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/textreader.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/base/optional.hpp"

namespace util {

    /** Editable configuration file.
        This represents the textual content of a configuration file.
        It allows updating the file, while attempting to preserve its structure (layout, comments) as good as possible.

        The in-memory representation is a list of typed Element objects.
        Each piece of text is transformed into an Element.

        Elements are addressed using keys.
        Keys are case-insensitive. */
    class ConfigurationFile {
     public:
        /** Type of an element. */
        enum ElementType {
            Generic,            ///< Anything.
            Section,            ///< Section delimiter. key is section name.
            Assignment          ///< Assignment. key is section name plus value name.
        };

        /** Part of configuration file.
            The textual representation of the element is always "prefix + value" plus newline. */
        struct Element {
            ElementType type;   ///< Type.
            String_t    key;    ///< Key for locating an element.
            String_t    prefix; ///< Prefix text.
            String_t    value;  ///< Value text.
            Element(ElementType type, String_t key, String_t prefix, String_t value)
                : type(type), key(key), prefix(prefix), value(value)
                { }
        };

        /** Constructor.
            Makes an empty object. */
        ConfigurationFile();

        /** Destructor. */
        ~ConfigurationFile();

        /** Load from file.
            Loads the given file.
            The ConfigurationFile should be empty before calling this function.

            After the function call, the content of this object is identical to the file content.
            Non-canonical input (e.g. duplicate assignments) will be preserved.

            \param in Input */
        void load(afl::io::TextReader& in);

        /** Save to file.
            \param out Output */
        void save(afl::io::TextWriter& out) const;

        /** Merge from another ConfigurationFile.
            If the other file contains assignments that this one contains as well, the values will be changed.
            If the other file contains new sections or assignments, those will be taken over to this file,
            attempting to preserve format.
            \param other Input */
        void merge(const ConfigurationFile& other);

        /** Set single value.
            Updates the value if it exists, creates it otherwise.
            \param key Key; either just a single key, or "section.key"
            \param value Value */
        void set(String_t key, String_t value);

        /** Set single value, sectioned.
            Updates the value if it exists, creates it otherwise.
            \param section Target section
            \param key Key
            \param value Value */
        void set(String_t section, String_t key, String_t value);

        /** Remove value.
            Removes at most one instance of the key.

            If the input is non-canonical (=duplicate assignments), another assignment may now get active.
            Call remove() in a loop to remove all instances.

            \param key Key
            \retval true Item was removed
            \retval false No such element found */
        bool remove(String_t key);

        /** Get number of elements.
            \return number of elements */
        size_t getNumElements() const;

        /** Get element, given an index.
            \param index Index [0,getNumElements())
            \return element; null if index out of range */
        const Element* getElementByIndex(size_t index) const;

        /** Find index of an element.
            \param type Desired element type
            \param key Desired key. For Assignment, "section.key" or "key"; for Section, "section".
            \return Index if any */
        afl::base::Optional<size_t> findIndex(ElementType type, String_t key) const;

        /** Find an element.
            \param type Desired element type
            \param key Desired key. For Assignment, "section.key" or "key"; for Section, "section".
            \return Element if any */
        const Element* findElement(ElementType type, String_t key) const;

        /** Find end of section.
            Assuming \c startIndex points at an element of a section, locates the end of the section (=next delimiter or end).
            \param startIndex Starting index [0,getNumElements()]
            \return Ending index [0,getNumElements()] */
        size_t findSectionEnd(size_t startIndex) const;

     private:
        afl::container::PtrVector<Element> m_elements;
    };

}

#endif
