#ifndef GENERS_IOEXCEPTION_HH_
#define GENERS_IOEXCEPTION_HH_

#include <string>
#include <exception>

namespace gs {
    /** Base class for the exceptions specific to the I/O library */
    class IOException : public std::exception
    {
    public:
        inline IOException() : descr_("gs::IOException") {}

        inline explicit IOException(const std::string& description)
            : descr_(description) {}

        inline virtual ~IOException() throw() {}

        virtual const char* what() const throw() {return descr_.c_str();}

    private:
        std::string descr_;
    };

    /**
    // Throw this exception to indicate failure of various stream
    // opening methods if it is difficult or impossible to clean up
    // after the failure in the normal flow of control
    */
    class IOOpeningFailure : public IOException
    {
        inline static std::string fileOpeningFailure(
            const std::string& whereInTheCode,
            const std::string& filename)
        {
            std::string msg("In ");
            msg += whereInTheCode;
            msg += ": failed to open file \"";
            msg += filename;
            msg += "\"";
            return msg;
        }

    public:
        inline IOOpeningFailure() : IOException("gs::IOOpeningFailure") {}

        inline explicit IOOpeningFailure(const std::string& description)
            : IOException(description) {}

        inline IOOpeningFailure(const std::string& whereInTheCode,
                                const std::string& filename)
            : IOException(fileOpeningFailure(whereInTheCode, filename)) {}

        inline virtual ~IOOpeningFailure() throw() {}
    };

    /**
    // Throw this exception to indicate failure in the writing process.
    // For example, fail() method of the output stream returns "true",
    // and the function is unable to handle this situation locally.
    */
    struct IOWriteFailure : public IOException
    {
        inline IOWriteFailure() : IOException("gs::IOWriteFailure") {}

        inline explicit IOWriteFailure(const std::string& description)
            : IOException(description) {}

        inline virtual ~IOWriteFailure() throw() {}
    };

    /**
    // Throw this exception to indicate failure in the reading process.
    // For example, fail() method of the input stream returns "true",
    // and the function is unable to handle this situation locally.
    */
    struct IOReadFailure : public IOException
    {
        inline IOReadFailure() : IOException("gs::IOReadFailure") {}

        inline explicit IOReadFailure(const std::string& description)
            : IOException(description) {}

        inline virtual ~IOReadFailure() throw() {}
    };

    /**
    // Throw this exception when improperly formatted or invalid data
    // is detected
    */
    struct IOInvalidData : public IOException
    {
        inline IOInvalidData() : IOException("gs::IOInvalidData") {}

        inline explicit IOInvalidData(const std::string& description)
            : IOException(description) {}

        inline virtual ~IOInvalidData() throw() {}
    };
}

#endif // GENERS_IOEXCEPTION_HH_
