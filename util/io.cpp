/**
  *  \file util/io.cpp
  *  \brief I/O-related utilities
  */

#include "util/io.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/data/defaultvaluefactory.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/bufferedstream.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/json/parser.hpp"
#include "afl/io/multidirectory.hpp"
#include "util/stringparser.hpp"

bool
util::storePascalString(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset)
{
    // Encode
    afl::base::GrowableBytes_t encoded = charset.encode(afl::string::toMemory(str));

    // Can we represent the size?
    uint8_t size = uint8_t(encoded.size());
    if (size == encoded.size()) {
        out.handleFullData(afl::base::fromObject(size));
        out.handleFullData(encoded);
        return true;
    } else {
        return false;
    }
}

bool
util::storePascalStringTruncate(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset)
{
    // Encode
    afl::base::GrowableBytes_t encoded = charset.encode(afl::string::toMemory(str));

    // Can we represent the size?
    bool ok;
    if (encoded.size() <= 255) {
        ok = true;
    } else {
        encoded.trim(255);
        ok = false;
    }
    uint8_t size = uint8_t(encoded.size());

    out.handleFullData(afl::base::fromObject(size));
    out.handleFullData(encoded);
    return ok;
}

String_t
util::loadPascalString(afl::io::Stream& in, afl::charset::Charset& charset)
{
    // Read size
    uint8_t size;
    in.fullRead(afl::base::fromObject(size));

    // Read body
    afl::base::GrowableBytes_t encodedChars;
    encodedChars.resize(size);
    in.fullRead(encodedChars);

    return charset.decode(encodedChars);
}

String_t
util::appendFileNameExtension(afl::io::FileSystem& fs, String_t pathName, String_t ext, bool force)
{
    // ex io/dirs.cc:appendFileNameExtension
    String_t fileName = fs.getFileName(pathName);
    String_t dirName  = fs.getDirectoryName(pathName);
    if (fileName.size() == 0) {
        // pathological case
        return fs.makePathName(dirName, "." + ext);
    } else {
        // do not accept index 0 to avoid identifying ".emacs" as zero-length basename with extension EMACS
        String_t::size_type n = fileName.rfind('.');
        if (n == String_t::npos || n == 0) {
            return fs.makePathName(dirName, fileName + "." + ext);
        } else if (force) {
            return fs.makePathName(dirName, fileName.substr(0, n+1) + ext);
        } else {
            return pathName;
        }
    }
}

String_t
util::getFileNameExtension(afl::io::FileSystem& fs, String_t pathName)
{
    String_t fileName = fs.getFileName(pathName);
    String_t::size_type dot = fileName.rfind('.');
    if (dot != String_t::npos && dot != 0) {
        return fileName.substr(dot);
    } else {
        return String_t();
    }
}

void
util::createDirectoryTree(afl::io::FileSystem& fs, const String_t dirName)
{
    const String_t parentName = fs.getDirectoryName(dirName);
    const String_t childName  = fs.getFileName(dirName);

    // If parentName is the same as dirName, this means that dirName does not have a parent.
    // In this case, we don't do anything.
    if (parentName != dirName) {
        // Try enumerating the parent's content. If that fails, try to create it.
        // (openDir alone does not check whether the directory actually exists.)
        try {
            afl::base::Ref<afl::io::Directory> parent = fs.openDirectory(parentName);
            parent->getDirectoryEntries();
        }
        catch (afl::except::FileProblemException&) {
            createDirectoryTree(fs, parentName);
        }

        // Parent should now exist. Try creating child in it unless it already exists.
        try {
            afl::base::Ref<afl::io::Directory> parent = fs.openDirectory(parentName);
            afl::base::Ref<afl::io::DirectoryEntry> entry = parent->getDirectoryEntryByName(childName);
            if (entry->getFileType() != afl::io::DirectoryEntry::tDirectory) {
                entry->createAsDirectory();
            }
        }
        catch (afl::except::FileProblemException&) { }
    }
}

afl::base::Ref<afl::io::Directory>
util::makeSearchDirectory(afl::io::FileSystem& fs, afl::base::Memory<const String_t> dirNames)
{
    if (dirNames.size() == 1U) {
        return fs.openDirectory(*dirNames.at(0));
    } else {
        afl::base::Ref<afl::io::MultiDirectory> dir = afl::io::MultiDirectory::create();
        while (const String_t* p = dirNames.eat()) {
            dir->addDirectory(fs.openDirectory(*p));
        }
        return dir;
    }
}

std::auto_ptr<afl::data::Value>
util::parseJSON(afl::base::ConstBytes_t data)
{
    afl::data::DefaultValueFactory factory;
    afl::io::ConstMemoryStream cms(data);
    afl::io::BufferedStream buf(cms);
    return std::auto_ptr<afl::data::Value>(afl::io::json::Parser(buf, factory).parseComplete());
}

afl::data::Access
util::findArrayItemById(afl::data::Access array, String_t key, int value)
{
    for (size_t i = 0, n = array.getArraySize(); i < n; ++i) {
        afl::data::Access ele = array[i], thisKey = ele(key);
        if (thisKey.getValue() != 0 && thisKey.toInteger() == value) {
            return ele;
        }
    }
    return afl::data::Access();
}

void
util::toIntegerList(afl::data::IntegerList_t& list, afl::data::Access value)
{
    if (size_t n = value.getArraySize()) {
        // Array
        for (size_t i = 0; i < n; ++i) {
            list.push_back(value[i].toInteger());
        }
    } else {
        // String (also handles integer case)
        StringParser p(value.toString());
        while (!p.parseEnd()) {
            int i = 0;
            if (p.parseInt(i)) {
                list.push_back(i);
            } else {
                p.consumeCharacter();
            }
        }
    }
}
