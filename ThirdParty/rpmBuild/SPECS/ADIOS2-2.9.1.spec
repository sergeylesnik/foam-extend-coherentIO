#------------------------------------------------------------------------------
# =========                 |
# \\      /  F ield         | foam-extend: Open Source CFD
#  \\    /   O peration     |
#   \\  /    A nd           | For copyright notice see file Copyright
#    \\/     M anipulation  |
#------------------------------------------------------------------------------
# License
#     This file is part of foam-extend.
#
#     foam-extend is free software: you can redistribute it and/or modify it
#     under the terms of the GNU General Public License as published by the
#     Free Software Foundation, either version 3 of the License, or (at your
#     option) any later version.
#
#     foam-extend is distributed in the hope that it will be useful, but
#     WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.
#
# Script
#     RPM spec file for ADIOS2-2.9.1
#
# Description
#     RPM spec file for creating a relocatable RPM
#
# Authors:
#     Gregor Weiss, HLRS (2023)
#
#------------------------------------------------------------------------------

# We grab the value of WM_THIRD_PARTY and WM_OPTIONS from the environment variable
%{expand:%%define _WM_THIRD_PARTY_DIR %(echo $WM_THIRD_PARTY_DIR)}
%{expand:%%define _WM_OPTIONS         %(echo $WM_OPTIONS)}

# Disable the generation of debuginfo packages
%define debug_package %{nil}

# The topdir needs to point to the $WM_THIRD_PARTY/rpmbuild directory
%define _topdir	 	%{_WM_THIRD_PARTY_DIR}/rpmBuild
%define _tmppath	%{_topdir}/tmp

# Will install the package directly $WM_THIRD_PARTY_DIR
#   Some comments about package relocation:
#   By using this prefix for the Prefix:  parameter in this file, you will make this
#   package relocatable.
#
#   This is fine, as long as your software is itself relocatable.
#
#   Simply take note that libraries built with libtool are not relocatable because the
#   prefix we specify will be hard-coded in the library .la files.
#   Ref: http://sourceware.org/autobook/autobook/autobook_80.html
#
#   In that case, if you ever change the value of the $WM_THIRD_PARTY_DIR, you will
#   not be able to reutilize this RPM, even though it is relocatable. You will need to
#   regenerate the RPM.
#
%define _prefix         %{_WM_THIRD_PARTY_DIR}

%define name		ADIOS2
%define release		%{_WM_OPTIONS}
%define version 	2.9.1

%define buildroot       %{_topdir}/BUILD/%{name}-%{version}-root

BuildRoot:	        %{buildroot}
Summary: 		%{name}
License: 		Unkown
Name: 			%{name}
Version: 		%{version}
Release: 		%{release}
URL:                    https://github.com/ornladios/ADIOS2/archive/
Source: 		%url/v%{version}.tar.gz
Prefix: 		%{_prefix}
Group: 			Development/Tools

%define _installPrefix  %{_prefix}/packages/%{name}-%{version}/platforms/%{_WM_OPTIONS}


%description
%{summary}

%prep
%setup -q

%build
#
# set CMake cache variables
#
    addCMakeVariable()
    {
        while [ -n "$1" ]
        do
            CMAKE_VARIABLES="$CMAKE_VARIABLES -D$1"
            shift
        done
   }

    # export WM settings in a form that GNU configure recognizes
    [ -n "$WM_CC" ]         &&  export CC="$WM_CC"
    [ -n "$WM_CXX" ]        &&  export CXX="$WM_CXX"
    [ -n "$WM_CFLAGS" ]     &&  export CFLAGS="$WM_CFLAGS"
    [ -n "$WM_CXXFLAGS" ]   &&  export CXXFLAGS="$WM_CXXFLAGS"
    [ -n "$WM_LDFLAGS" ]    &&  export LDFLAGS="$WM_LDFLAGS"

    # start with these general settings
    addCMakeVariable  BUILD_SHARED_LIBS:BOOL=ON
    addCMakeVariable  CMAKE_BUILD_TYPE:STRING=Release

    echo "CMAKE_VARIABLES: $CMAKE_VARIABLES"

    mkdir -p ./buildObj
    cd ./buildObj

    cmake \
        -DCMAKE_INSTALL_PREFIX:PATH=%{_installPrefix} \
        $CMAKE_VARIABLES \
	..

    [ -z "$WM_NCOMPPROCS" ] && WM_NCOMPPROCS=1
    make -j $WM_NCOMPPROCS

%install

    cd buildObj
    make install

    # Creation of foam-extend specific .csh and .sh files"

    echo ""
    echo "Generating foam-extend specific .csh and .sh files for the package %{name}-%{version}"
    echo ""
    #
    # Generate package specific .sh file for foam-extend
    #
mkdir -p %{_installPrefix}/etc
cat << DOT_SH_EOF > %{_installPrefix}/etc/%{name}-%{version}.sh
# Load %{name}-%{version} binaries if available
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

export ADIOS2_DIR=\$WM_THIRD_PARTY_DIR/packages/%{name}-%{version}/platforms/\$WM_OPTIONS
export ADIOS2_BIN_DIR=\$ADIOS2_DIR/bin
export ADIOS2_LIB_DIR=\$ADIOS2_DIR/lib
export ADIOS2_INCLUDE_DIR=\$ADIOS2_DIR/include
export ADIOS2_INCLUDE_CXX11_DIR=\$ADIOS2_DIR/include/adios2/cxx11/
export ADIOS2_INCLUDE_COMMON_DIR=\$ADIOS2_DIR/include/adios2/common/

export ADIOS2_VERSION=%{version}

# Enable access to the package applications if present
[ -d \$ADIOS2_BIN_DIR ] && _foamAddPath \$ADIOS2_BIN_DIR
[ -d \$ADIOS2_LIB_DIR ] && _foamAddLib \$ADIOS2_LIB_DIR

export ADIOS2_LIBS=\$(adios2-config --cxx-libs)
export ADIOS2_FLAGS=\$(adios2-config --cxx-flags)

DOT_SH_EOF

    #
    # Generate package specific .csh file for foam-extend
    #
cat << DOT_CSH_EOF > %{_installPrefix}/etc/%{name}-%{version}.csh
# Load %{name}-%{version} binaries if available
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
setenv ADIOS2_DIR \$WM_THIRD_PARTY_DIR/packages/%{name}-%{version}/platforms/\$WM_OPTIONS
setenv ADIOS2_BIN_DIR \$ADIOS2_DIR/bin
setenv ADIOS2_LIB_DIR \$ADIOS2_DIR/lib
setenv ADIOS2_INCLUDE_DIR \$ADIOS2_DIR/include
setenv ADIOS2_INCLUDE_CXX11_DIR \$ADIOS2_DIR/include/adios2/cxx11/
setenv ADIOS2_INCLUDE_COMMON_DIR \$ADIOS2_DIR/include/adios2/common/

setenv ADIOS2_VERSION %{version}

if ( -e \$ADIOS2_BIN_DIR ) then
    _foamAddPath \$ADIOS2_BIN_DIR
endif

if ( -e \$ADIOS2_LIB_DIR ) then
    _foamAddLib \$ADIOS2_LIB_DIR
endif

setenv ADIOS2_LIBS $(adios2-config --cxx-libs)
setenv ADIOS2_FLAGS $(adios2-config --cxx-flags)

DOT_CSH_EOF

    #finally, generate a .tgz file for systems where using rpm for installing packages
    # as a non-root user might be a problem.
    (mkdir -p  %{_topdir}/TGZS/%{_target_cpu}; cd $RPM_BUILD_ROOT/%{_prefix}; tar -zcvf %{_topdir}/TGZS/%{_target_cpu}/%{name}-%{version}.tgz  packages/%{name}-%{version})

%clean

%files
%defattr(-,root,root)
%{_installPrefix}


