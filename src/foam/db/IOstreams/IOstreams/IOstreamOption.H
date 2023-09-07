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
    Foam::IOstreamOption

Description
    A simple container for options IOstream can normally have.

SourceFiles
    IOstreamOption.C

\*---------------------------------------------------------------------------*/

#ifndef IOstreamOption_H
#define IOstreamOption_H

#include "char.H"
#include "bool.H"
#include "label.H"
#include "uLabel.H"
#include "scalar.H"
#include "word.H"
#include "infoSwitch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class IOstreamOption Declaration
\*---------------------------------------------------------------------------*/

class IOstreamOption
{

public:

    // Public data types

        //- Enumeration for the format of data in the stream
        enum streamFormat : char
        {
            ASCII,
            BINARY,
            COHERENT
        };

        //- Enumeration for the format of data in the stream
        enum compressionType : char
        {
            UNCOMPRESSED,
            COMPRESSED
        };

        //- Enumeration for the I/O mode of the stream
        enum streamMode : char
        {
            SYNC,
            DEFERRED
        };

        //- Enumeration for the destination of heavy data files (data.bp)
        enum dataDestination : char
        {
            TIME,
            CASE
        };

        //- Ostream operator
        friend Ostream& operator<<(Ostream& os, const streamFormat& sf);

        //- Version number type
        class versionNumber
        {
            //- The version number
            scalar versionNumber_;

            //- The version number as an integer
            int index_;


        public:

            // Constructors

                //- Construct from number
                versionNumber(const scalar num)
                :
                    versionNumber_(num),
                    index_(numberToIndex(num))
                {}

                //- Construct from Istream
                versionNumber(Istream& is)
                :
                    versionNumber_(readScalar(is)),
                    index_(numberToIndex(versionNumber_))
                {}


            // Member functions

                //- Convert a version number into an index
                int numberToIndex(const scalar num) const
                {
                    return int(10*num + SMALL);
                }

                //- Return major version
                int majorVersion() const
                {
                    return int(versionNumber_);
                }

                //- Return minor version
                int minorVersion() const
                {
                    return int(10.0*(versionNumber_ - majorVersion()));
                }

                //- Return the versionNumber as a character string
                string str() const;


            // Member operators

                //- Are these versionNumbers the same?
                bool operator==(const versionNumber& vn)
                {
                    return index_ == vn.index_;
                }

                //- Are these versionNumbers different?
                bool operator!=(const versionNumber& vn)
                {
                    return index_ != vn.index_;
                }

                //- Is this version older than the one given
                bool operator<(const versionNumber& vn)
                {
                    return index_ < vn.index_;
                }

                //- Is this version the same as or older than the one given
                bool operator<=(const versionNumber& vn)
                {
                    return index_ <= vn.index_;
                }

                //- Is this version newer than the one given
                bool operator>(const versionNumber& vn)
                {
                    return index_ > vn.index_;
                }

                //- this version the same as or newer than the one given
                bool operator>=(const versionNumber& vn)
                {
                    return index_ >= vn.index_;
                }


            //- Ostream operator
            friend Ostream& operator<<(Ostream& os, const versionNumber& vn);
        };


    // Public static data

        //- Original version number
        static const versionNumber originalVersion;

        //- Current version number
        static const versionNumber currentVersion;


private:

    // Private data

        //- Format: (ascii | binary)
        streamFormat format_;

        //- Stream version number (e.g., 2.0)
        versionNumber version_;

        //- Compression: (uncompressed | compressed)
        compressionType compression_;

        //- I/O stream mode (sync | deferred)
        streamMode mode_;

        //- Heavy data file (data.bp) destination (time | case)
        dataDestination destination_;

public:

    // Constructors

        //- Default construct
        IOstreamOption
        (
            streamFormat format = streamFormat::ASCII,
            versionNumber version = versionNumber(2.0),
            compressionType compression = compressionType::UNCOMPRESSED,
            streamMode mode = streamMode::SYNC,
            dataDestination destination = dataDestination::TIME
        )
        :
            format_(format),
            version_(version),
            compression_(compression),
            mode_(mode),
            destination_(destination)
        {}


    // Static Member Functions

        //- Return stream mode of given mode name
        static streamMode modeEnum(const word&);

        static dataDestination destinationEnum(const word&);


    // Member Functions

        //- Return stream format of given format name
        static streamFormat formatEnum(const word&);

        //- Return current stream format
        streamFormat format() const
        {
            return format_;
        }

        //- Set the stream format
        streamFormat format(const streamFormat fmt)
        {
            streamFormat fmt0 = format_;
            format_ = fmt;
            return fmt0;
        }

        //- Set the stream format from word
        streamFormat format(const word& fmt)
        {
            streamFormat fmt0 = format_;
            format_ = formatEnum(fmt);
            return fmt0;
        }

        //- Return the stream version
        versionNumber version() const
        {
            return version_;
        }

        //- Set the stream version
        versionNumber version(const versionNumber ver)
        {
            versionNumber ver0 = version_;
            version_ = ver;
            return ver0;
        }

        //- Return compression of given compression name
        static compressionType compressionEnum(const word&);

        //- Return the stream compression
        compressionType compression() const
        {
            return compression_;
        }

        //- Set the stream compression
        compressionType compression(const compressionType cmp)
        {
            compressionType cmp0 = compression_;
            compression_ = cmp;
            return cmp0;
        }

        //- Set the stream compression from word
        compressionType compression(const word& cmp)
        {
            compressionType cmp0 = compression_;
            compression_ = compressionEnum(cmp);
            return cmp0;
        }

        //- Return current stream mode
        streamMode mode() const
        {
            return mode_;
        }

        //- Set the stream mode
        streamMode mode(const streamMode md)
        {
            streamMode md0 = mode_;
            mode_ = md;
            return md0;
        }

        //- Set the stream mode from word
        streamMode mode(const word& md)
        {
            streamMode md0 = mode_;
            mode_ = modeEnum(md);
            return md0;
        }

        //- Return current stream mode
        dataDestination destination() const
        {
            return destination_;
        }

        //- Set the data destination
        dataDestination destination(const dataDestination dest)
        {
            dataDestination dest0 = destination_;
            destination_ = dest;
            return dest0;
        }

        //- Set the data destination from word
        dataDestination destination(const word& dest)
        {
            dataDestination dest0 = destination_;
            destination_ = destinationEnum(dest);
            return dest0;
        }
};


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

Ostream& operator<<(Ostream& os, const IOstreamOption::streamFormat& sf);
Ostream& operator<<(Ostream& os, const IOstreamOption::versionNumber& vn);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
