/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.1
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::IOstream

Description
    An IOstream is an abstract base class for all input/output systems; be
    they streams, files, token lists etc.

    The basic operations are construct, close, read token, read primitive
    and read binary block.  In addition version control and line number
    counting is incorporated.  Usually one would use the read primitive
    member functions, but if one were reading a stream on unknown data
    sequence one can read token by token, and then analyse.

SourceFiles
    IOstream.C

\*---------------------------------------------------------------------------*/

#ifndef IOstream_H
#define IOstream_H

#include "fileName.H"
#include "InfoProxy.H"
#include "IOstreamOption.H"

#include <iostream>


#if __GNUC__ < 3
#   define ios_base ios
#endif

using std::ios_base;
using std::istream;
using std::ostream;

using std::cin;
using std::cout;
using std::cerr;

typedef char parIOType;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class IOstream Declaration
\*---------------------------------------------------------------------------*/

class IOstream
:
    public IOstreamOption
{

public:

    // Public data types

        //- Enumeration for whether the stream open or closed
        enum streamAccess : char
        {
            OPENED,
            CLOSED
        };


    // Public static data

        //- Default precision
        static debug::infoSwitch precision_;


private:

    // Private data

        //- Name of the stream
        static fileName name_;

        streamAccess openClosed_;
        ios_base::iostate ioState_;


protected:

    // Protected data

        label lineNumber_;


    // Protected member functions

        // Access

            //- Set stream opened
            void setOpened()
            {
                openClosed_ = OPENED;
            }

            //- Set stream closed
            void setClosed()
            {
                openClosed_ = CLOSED;
            }

            //- Set stream state
            void setState(ios_base::iostate state)
            {
                ioState_ = state;
            }

            //- Set stream to be good
            void setGood()
            {
                ioState_ = ios_base::iostate(0);
            }


public:

    // Constructors

        //- Default construct
        IOstream(IOstreamOption streamOpt = IOstreamOption())
        :
            IOstreamOption(streamOpt),
            openClosed_(CLOSED),
            ioState_(ios_base::iostate(0)),
            lineNumber_(0)
        {
            setBad();
        }

        //- Construct setting format and version
        IOstream
        (
            IOstreamOption::streamFormat fmt,
            IOstreamOption::versionNumber ver,
            IOstreamOption::compressionType cmp = IOstreamOption::UNCOMPRESSED
        )
        :
            IOstream(IOstreamOption(fmt, ver, cmp))
        {}


    // Destructor

        virtual ~IOstream()
        {}


    // Member functions

        // Access

            //- Return the name of the stream
            //  Useful for Fstream to return the filename
            virtual const fileName& name() const
            {
                return name_;
            }

            //- Return non-const access to the name of the stream
            //  Useful to alter the stream name
            virtual fileName& name()
            {
                return name_;
            }


        // Check

            //- Check IOstream status for given operation
            //  print IOstream state if error has occured
            virtual bool check(const char* operation) const;

            //- Check IOstream status for given operation
            //  print IOstream state if error has occured and exit
            void fatalCheck(const char* operation) const;

            //- Return true if stream has been opened
            bool opened() const
            {
                return openClosed_ == OPENED;
            }

            //- Return true if stream is closed
            bool closed() const
            {
                return openClosed_ == CLOSED;
            }

            //- Return true if next operation might succeed
            bool good() const
            {
                return ioState_ == 0;
            }

            //- Return true if end of input seen
            bool eof() const
            {
                return ioState_ & ios_base::eofbit;
            }

            //- Return true if next operation will fail
            bool fail() const
            {
                return ioState_ & (ios_base::badbit | ios_base::failbit);
            }

            //- Return true if stream is corrupted
            bool bad() const
            {
                return ioState_ & ios_base::badbit;
            }

            //- Return non-zero if the stream has not failed
            operator void*() const
            {
                return fail()
                    ? reinterpret_cast<void*>(0)
                    : reinterpret_cast<void*>(-1);
            }

            //- Return true if the stream has failed
            bool operator!() const
            {
                return fail();
            }


        // Stream state functions

            //- Return current stream line number
            label lineNumber() const
            {
                return lineNumber_;
            }

            //- Return current stream line number
            label& lineNumber()
            {
                return lineNumber_;
            }

            //- Set the stream line number
            label lineNumber(const label ln)
            {
                label ln0 = lineNumber_;
                lineNumber_ = ln;
                return ln0;
            }

            //- Return flags of stream
            virtual ios_base::fmtflags flags() const = 0;

            //- Return the default precision
            static unsigned int defaultPrecision()
            {
                return precision_();
            }

            //- Reset the default precision (and return old precision)
            static unsigned int defaultPrecision(unsigned int p)
            {
                unsigned int precision0 = precision_();
                precision_ = p;
                return precision0;
            }

            //- Set stream to have reached eof
            void setEof()
            {
                ioState_ |= ios_base::eofbit;
            }

            //- Set stream to have failed
            void setFail()
            {
                ioState_ |= ios_base::failbit;
            }

            //- Set stream to be bad
            void setBad()
            {
                ioState_ |= ios_base::badbit;
            }

            //- Set flags of stream
            virtual ios_base::fmtflags flags(const ios_base::fmtflags f) = 0;

            //- Set flags of stream
            ios_base::fmtflags setf(const ios_base::fmtflags f)
            {
                return flags(flags() | f);
            }

            //- Set flags of given field of stream
            ios_base::fmtflags setf
            (
                const ios_base::fmtflags f,
                const ios_base::fmtflags mask
            )
            {
                return flags((flags() & ~mask) | (f & mask));
            }

            //- Unset flags of stream
            void unsetf(const ios_base::fmtflags uf)
            {
                flags(flags()&~uf);
            }


        // Print

            //- Print description of IOstream to Ostream
            virtual void print(Ostream&) const;

            //- Check given stream state bits
            void print(Ostream&, const int streamState) const;


        // Info

            //- Return info proxy.
            //  Used to print IOstream information to a stream
            InfoProxy<IOstream> info() const
            {
                return *this;
            }
};


// --------------------------------------------------------------------
// ------ Manipulators (not taking arguments)
// --------------------------------------------------------------------

typedef IOstream& (*IOstreamManip)(IOstream&);

//- operator<< handling for manipulators without arguments
inline IOstream& operator<<(IOstream& io, IOstreamManip f)
{
    return f(io);
}


inline IOstream& dec(IOstream& io)
{
    io.setf(ios_base::dec, ios_base::dec|ios_base::hex|ios_base::oct);
    return io;
}

inline IOstream& hex(IOstream& io)
{
    io.setf(ios_base::hex, ios_base::dec|ios_base::hex|ios_base::oct);
    return io;
}

inline IOstream& oct(IOstream& io)
{
    io.setf(ios_base::oct, ios_base::dec|ios_base::hex|ios_base::oct);
    return io;
}

inline IOstream& fixed(IOstream& io)
{
    io.setf(ios_base::fixed, ios_base::floatfield);
    return io;
}

inline IOstream& scientific(IOstream& io)
{
    io.setf(ios_base::scientific, ios_base::floatfield);
    return io;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
