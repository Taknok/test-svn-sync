# This is a spec file for the installation of XFLR5 v5
%define name xflr5
%define release 1
%define version 5.00

BuildRoot: %{_tmppath}/%{name}-%{version}
Summary: XFLR5 is an analysis tool for airfoils and planes operating at low Re numbers
License: GPL
Name: %{name}
Version: %{version}
Release: %{release}
Source: %{name}-%{version}.tar.gz
Packager: x-andre
Group: Applications/Engineering
%if 0%{?fedora_version}  
BuildRequires: qt-devel >= 4.4.3 gcc-c++  , Mesa-devel
%else  
BuildRequires: libqt4-devel >= 4.4.3 gcc-c++  , Mesa-devel
%endif  

%description
XFLR5 is a design tool intended for the design and analysis of reduced scaled model airplanes.
It includes the XFoil program for the analysis of airfoils.
It includes a non-linear Lifting Line method (LLT), two Vortex Lattice Methods (VLM),
and a 3D panel first-order method for the analysis of wings and planes.
XFLR5 v5 is a re-write of XFLR5 v4 using Qt4 libraries instead of Microsoft's MFC.

%prep
#%{name}         = xflr5
#%{_docdir}      = /usr/share/doc/packages
#%{_datadir}     = /usr/share
#%{_bindir}      = /usr/bin
#%{_tmppath}    = /home/windsoarer/Qt/rpmbuild/tmp
#%{buildroot}    = /home/windsoarer/Qt/rpmbuild/BUILDROOT/xflr5-5.00-1.x86_64

%setup -q

%build 
%if 0%{?fedora_version}  
qmake-qt4 -makefile %{name}.pro 'target.path = %{buildroot}%{_bindir}'  
qmake-qt4 'target.path = %{buildroot}%{_bindir}'  
%else  
qmake -makefile %{name}.pro 'target.path = %{buildroot}%{_bindir}'  
qmake 'target.path = %{buildroot}%{_bindir}'   
%endif  

make 

echo $RPM_BUILD_ROOT 
 
%install
%__mkdir -p %{buildroot}%{_bindir} 
%__mkdir -p %{buildroot}/usr/share/%{name}/translations 
%__mkdir -p %{buildroot}/usr/share/applications
%__mkdir -p %{buildroot}/usr/share/pixmaps 
%__mkdir -p %{buildroot}%{_docdir}/%{name}  
make install DESTDIR=$RPM_BUILD_ROOT $INSTALL_TARGET 
#make install  DESTDIR=%{buildroot}%{_bindir} $INSTALL_TARGET
%__install -m 644 images/%{name}.png %{buildroot}%{_datadir}/pixmaps/%{name}.png  
%__install -m 644 suse/%{name}.desktop %{buildroot}%{_datadir}/applications  
%__install -m 644 translations/*.qm   %{buildroot}/usr/share/%{name}/translations 


%files
%defattr(-,root,root)
  
#%if 0%{suse_version} > 1110 || 0%{fedora_version} || 0%{mandriva_version}  
%{_datadir}/applications/%{name}.desktop  
#%endif  
  
%{_datadir}/pixmaps/%{name}.png  
%{_datadir}/%{name}  
%docdir %{buildroot}/usr/share/%{name}/translations

%doc License.txt
%{_bindir}/%{name} 

%clean
%{__rm} -rf $RPM_BUILD_ROOT 

echo %{_bindir}/%{name} 


%changelog
*Fri Apr 3  2010 A. Deperrois
First rpm release
