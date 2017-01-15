/**
  *  \file interpreter/filetable.cpp
  */

#include "interpreter/filetable.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "interpreter/error.hpp"
#include "afl/data/scalarvalue.hpp"
#include "interpreter/filevalue.hpp"

struct interpreter::FileTable::State {
    afl::base::Ref<afl::io::Stream> stream;
    afl::io::TextFile textFile;

    State(afl::base::Ref<afl::io::Stream> stream);
    ~State();
};

interpreter::FileTable::State::State(afl::base::Ref<afl::io::Stream> stream)
    : stream(stream),
      textFile(*stream)
{
    // ex IntFileDescriptor::IntFileDescriptor
    // FIXME: how to do this? text_file.setCharacterSet(getGameCharacterSet());
}

interpreter::FileTable::State::~State()
{
    // ex IntFileDescriptor::~IntFileDescriptor
    try {
        textFile.flush();
    }
    catch (afl::except::FileProblemException& e) {
        // console.write(LOG_ERROR, format("%s: %s", e.getFileName(), e.what()));
        // console.write(LOG_ERROR, format(_("%s: Error closing this file, possible data loss."), text_file.getName()));
        (void) e;
    }
}





interpreter::FileTable::FileTable()
    : m_files()
{ }

interpreter::FileTable::~FileTable()
{ }

void
interpreter::FileTable::setMaxFiles(size_t n)
{
    m_files.resize(n);
}

// /** Open a new file.
//     \param fd  File number
//     \param sfd Newly-allocated IntFileDescriptor object */
void
interpreter::FileTable::openFile(size_t fd, afl::base::Ref<afl::io::Stream> ps)
{
    // ex int/file.cc:openNewFile (sort-of)
    if (fd >= m_files.size()) {
        throw Error::rangeError();
    }

    m_files.replaceElementNew(fd, new State(ps));
}

void
interpreter::FileTable::closeFile(size_t fd)
{
    if (fd < m_files.size() && m_files[fd] != 0) {
        m_files[fd]->textFile.flush();
        m_files.replaceElementNew(fd, 0);
    }
}

void
interpreter::FileTable::prepareForAppend(size_t fd)
{
    // Read one line to detect encoding;
    // UTF-8 files will thus keep their encoding.
    if (fd < m_files.size() && m_files[fd] != 0) {
        String_t tmp;
        m_files[fd]->textFile.readLine(tmp);
        m_files[fd]->textFile.setPos(m_files[fd]->textFile.getSize());
    }
}



// /** Check file number argument.
//     \param fd [out] File number, guaranteed to be in range
//     \param arg [in] Argument received from user
//     \param mustBeOpen [in] If true, accept only open files
//     \return true iff argument was given, false if it was null
//     \throw IntError on type error */
bool
interpreter::FileTable::checkFileArg(size_t& fd, afl::data::Value* arg, bool mustBeOpen)
{
    // ex int/file.cc:checkFileArg
    // Check for null
    if (!arg) {
        return false;
    }

    // Check for file number
    int32_t value;
    if (afl::data::ScalarValue* sv = dynamic_cast<afl::data::ScalarValue*>(arg)) {
        value = sv->getValue();
    } else if (FileValue* fv = dynamic_cast<FileValue*>(arg)) {
        value = fv->getFileNumber();
    } else {
        throw Error::typeError(Error::ExpectFile);
    }

    // Check range
    if (value < 0 || int32_t(size_t(value)) != value || size_t(value) >= m_files.size()) {
        throw Error::rangeError();
    }
    if (mustBeOpen && m_files[value] == 0) {
        throw Error("File not open");
    }

    fd = value;
    return true;
}
        
// /** Check file number argument, public interface.
//     \param tf [out] File handle, guaranteed to be valid if return value is true
//     \param arg [in] Argument received from user
//     \return true iff argument was given, false if it was null
//     \throw IntError on type error */
bool
interpreter::FileTable::checkFileArg(afl::io::TextFile*& tf, afl::data::Value* arg)
{
    // ex int/file.cc:checkFileArg
    size_t fd;
    if (checkFileArg(fd, arg, true)) {
        tf = &m_files[fd]->textFile;
        return true;
    } else {
        tf = 0;
        return false;
    }
}

size_t
interpreter::FileTable::getFreeFile() const
{
    // ex IFFreeFile (part)
    // We never return 0!
    for (size_t i = 1, n = m_files.size(); i < n; ++i) {
        if (m_files[i] == 0) {
            return i;
        }
    }
    return 0;
}
