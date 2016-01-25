#
# spec file for package xflr5
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


Name:           xflr5
Version:        6.13
Release:        0
%define short_version 6.13
Summary:        Analysis tool for airfoils and planes operating at low Re numbers
License:        GPL-2.0
Group:          Productivity/Scientific/Other
Url:            http://www.xflr5.com
# Source0:        http://sourceforge.net/projects/%{name}/files/%{version}/%{name}-%{version}_src.tar.gz
Source0:        %{name}-%{version}_src.tar.gz
Source1:        %{name}.desktop
#BuildRequires:  Mesa-devel
BuildRequires:  gcc-c++
BuildRequires:  libqt4-devel >= 4.8.0
BuildRequires:  libQt5OpenGL-devel >= 5.2.0
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
XFLR5 is a design tool intended for the design and analysis of reduced scaled model airplanes.
It includes the XFoil program for the analysis of airfoils.
It includes a non-linear Lifting Line method (LLT), two Vortex Lattice Methods (VLM),
and a 3D panel flat panel,first-order method for the analysis of wings and planes.
XFLR5 v5 is a re-write of XFLR5 v4 using Qt4 libraries instead of Microsoft's MFC.
XFLR5 v6 introduces stability and control analysis

%prep
%setup -q -n %{name}-%{short_version}
cp %{SOURCE1} .
# fix wrong-file-end-of-line-encoding
sed -i 's/\r$//' {License.txt,doc/ReleaseNotes.txt}

%build
qmake -o Makefile %{name}-%{short_version}.pro
make %{?_smp_mflags}
lrelease %{name}_%{short_version}.pro

%install
install -Dm 755 %{name}               %{buildroot}%{_bindir}/%{name}
install -Dm 644 images/%{name}.png    %{buildroot}%{_datadir}/pixmaps/%{name}.png
install -Dm 644 %{name}.desktop       %{buildroot}%{_datadir}/applications/%{name}.desktop
install -d %{buildroot}%{_datadir}/%{name}/translations
install -m 644 translations/*.qm      %{buildroot}%{_datadir}/%{name}/translations

%files
%defattr(-,root,root)
%doc License.txt doc/ReleaseNotes.txt doc/*.odt
%{_bindir}/%{name}
%{_datadir}/%{name}/
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.png

%changelog
