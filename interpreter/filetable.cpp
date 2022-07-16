/**
  *  \file interpreter/filetable.cpp
  *  \brief Class interpreter::FileTable
  */

#include "interpreter/filetable.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "interpreter/error.hpp"
#include "interpreter/filevalue.hpp"

const char*const LOG_NAME = "interpreter";

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
    // This is a last-resort flush to try to prevent data loss.
    // Normally, files are flushed by closeFile(), closeAllFiles() which can report errors.
    try {
        textFile.flush();
    }
    catch (afl::except::FileProblemException& e) {
        (void) e;
    }
}


/******************************* FileTable *******************************/

// Constructor.
interpreter::FileTable::FileTable()
    : m_files()
{ }

// Destructor.
interpreter::FileTable::~FileTable()
{ }

// Set maximum number of files.
void
interpreter::FileTable::setMaxFiles(size_t n)
{
    m_files.resize(n);
}

// Open new file.
void
interpreter::FileTable::openFile(size_t fd, afl::base::Ref<afl::io::Stream> ps)
{
    // ex int/file.cc:openNewFile (sort-of)
    if (fd >= m_files.size()) {
        throw Error::rangeError();
    }

    m_files.replaceElementNew(fd, new State(ps));
}

// Close a file.
void
interpreter::FileTable::closeFile(size_t fd)
{
    if (fd < m_files.size() && m_files[fd] != 0) {
        // Extract the file, then flush it, so it will be guaranteed closed even if the flush fails.
        std::auto_ptr<State> p(m_files.extractElement(fd));
        p->textFile.flush();
    }
}

void
interpreter::FileTable::closeAllFiles(afl::sys::LogListener& log, afl::string::Translator& tx)
{
    bool hadErrors = false;
    for (size_t i = 0, n = m_files.size(); i < n; ++i) {
        std::auto_ptr<State> p(m_files.extractElement(i));
        if (p.get() != 0) {
            try {
                p->textFile.flush();
            }
            catch (std::exception& e) {
                log.write(afl::sys::LogListener::Error, LOG_NAME, String_t(), e);
                hadErrors = true;
            }
        }
    }
    if (hadErrors) {
        log.write(afl::sys::LogListener::Error, LOG_NAME, tx("Error while closing files; written data may have been lost."));
    }
}

// Get file by number.
afl::io::TextFile*
interpreter::FileTable::getFile(size_t fd) const
{
    if (fd < m_files.size() && m_files[fd] != 0) {
        return &m_files[fd]->textFile;
    } else {
        return 0;
    }
}

// Prepare a file for appending.
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


// Check file argument, produce file number.
bool
interpreter::FileTable::checkFileArg(size_t& fd, const afl::data::Value* arg, bool mustBeOpen)
{
    // ex int/file.cc:checkFileArg
    // ex fileint.pas:EvalFD
    // Check for null
    if (!arg) {
        return false;
    }

    // Check for file number
    int32_t value;
    if (const afl::data::ScalarValue* sv = dynamic_cast<const afl::data::ScalarValue*>(arg)) {
        value = sv->getValue();
    } else if (const FileValue* fv = dynamic_cast<const FileValue*>(arg)) {
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

// Check file argument, produce text file pointer.
bool
interpreter::FileTable::checkFileArg(afl::io::TextFile*& tf, const afl::data::Value* arg)
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

// Get a currently-unused slot.
size_t
interpreter::FileTable::getFreeFile() const
{
    // ex IFFreeFile (part)
    // 0 means no slot; slot 0 is never reported as usable slot!
    for (size_t i = 1, n = m_files.size(); i < n; ++i) {
        if (m_files[i] == 0) {
            return i;
        }
    }
    return 0;
}
