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
#     RPM spec file for petsc-release
#
# Description
#     RPM spec file for creating a relocatable RPM
#
# Authors:
#     Gregor Weiss, HLRS, (2022)
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

%define name		petsc
%define release		%{_WM_OPTIONS}
%define version 	release

%define buildroot       %{_topdir}/BUILD/%{name}-%{version}

BuildRoot:	        %{buildroot}
Summary: 		petsc
License: 		Unkown
Name: 			%{name}
Version: 		%{version}
Release: 		%{release}
URL:                    https://github.com/petsc/petsc/archive/
Source: 		%url/release.tar.gz
Prefix: 		%{_prefix}
Group: 			Development/Tools

%define _installPrefix  %{_prefix}/packages/%{name}-%{version}/platforms/%{_WM_OPTIONS}


%description
%{summary}

%prep
%setup -q 

%build
    # export WM settings in a form that GNU configure recognizes
    [ -n "$WM_CC" ]         &&  export CC="$WM_CC"
    [ -n "$WM_CXX" ]        &&  export CXX="$WM_CXX"
    [ -n "$WM_CFLAGS" ]     &&  export CFLAGS="$WM_CFLAGS"
    [ -n "$WM_CXXFLAGS" ]   &&  export CXXFLAGS="$WM_CXXFLAGS"
    [ -n "$WM_LDFLAGS" ]    &&  export LDFLAGS="$WM_LDFLAGS"

    ./configure     \
        --prefix=%{_installPrefix} --with-cc=mpicc --with-cxx=mpicxx --with-fc=mpif90 --with-mpi=1 CFLAGS="-O3 -Wall" CC_LINKER_FLAGS="-O3 -Wall" CXXFLAGS="-O3 -Wall" CXX_LINKER_FLAGS="-O3 -Wall" LDFLAGS=-O3 FFLAGS="-O3 -Wall" FC_LINKER_FLAGS="-O3 -Wall" --with-debugging=0 --download-openblas --download-scalapack

    #[ -z "$WM_NCOMPPROCS" ] && WM_NCOMPPROCS=1
    make PETS_DIR=%{buildroot} PETSC_ARCH=arch-linux-c-opt all

%install
    make PETS_DIR=%{buildroot} PETSC_ARCH=arch-linux-c-opt install

    # Creation of foam-extend specific .csh and .sh files"

    echo ""
    echo "Generating foam-extend specific .csh and .sh files for the package %{name}-%{version}"
    echo ""
    #
    # Generate package specific .sh file for foam-extend
    #
mkdir -p $RPM_BUILD_ROOT/%{_installPrefix}/etc
cat << DOT_SH_EOF > $RPM_BUILD_ROOT/%{_installPrefix}/etc/%{name}-%{version}.sh
# Load %{name}-%{version} binaries if available
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

export PETSC_DIR=\$WM_THIRD_PARTY_DIR/packages/%{name}-%{version}/platforms/\$WM_OPTIONS
export PETSC_BIN_DIR=\$PETSC_DIR/bin
export PETSC_LIB_DIR=\$PETSC_DIR/lib
export PETSC_INCLUDE_DIR=\$PETSC_DIR/include

export PETSC_VERSION=%{version}

# Enable access to the package applications if present
[ -d \$PETSC_BIN_DIR ] && _foamAddPath \$PETSC_BIN_DIR
[ -d \$PETSC_LIB_DIR ] && _foamAddLib \$PETSC_LIB_DIR

DOT_SH_EOF

    #
    # Generate package specific .csh file for foam-extend
    #
cat << DOT_CSH_EOF > $RPM_BUILD_ROOT/%{_installPrefix}/etc/%{name}-%{version}.csh
# Load %{name}-%{version} binaries if available
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
setenv PETSC_DIR \$WM_THIRD_PARTY_DIR/packages/%{name}-%{version}/platforms/\$WM_OPTIONS
setenv PETSC_BIN_DIR \$PETSC_DIR/bin
setenv PETSC_LIB_DIR \$PETSC_DIR/lib
setenv PETSC_INCLUDE_DIR \$PETSC_DIR/include

setenv PETSC_VERSION %{version}

if ( -e \$PETSC_BIN_DIR ) then
    _foamAddPath \$PETSC_BIN_DIR
endif

if ( -e \$PETSC_LIB_DIR ) then
    _foamAddLib \$PETSC_LIB_DIR
endif

DOT_CSH_EOF

    #finally, generate a .tgz file for systems where using rpm for installing packages
    # as a non-root user might be a problem.
    (mkdir -p  %{_topdir}/TGZS/%{_target_cpu}; cd $RPM_BUILD_ROOT/%{_prefix}; tar -zcvf %{_topdir}/TGZS/%{_target_cpu}/%{name}-%{version}.tgz  packages/%{name}-%{version})

%clean

%files
%defattr(-,root,root)
%{_installPrefix}


