#
# spec file for package open-rp
#
# Copyright (c) 2012 Darryl Sokoloski <darryl242@gmail.com>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.
 
# Please submit bugfixes or comments via http://code.google.com/p/open-rp/
#
Name: open-rp
Version: 1.4
Release: 1%{dist}
License: GPL
Group: Amusements/Games/Other
Source: %{name}-%{version}.tar.gz
BuildRoot: %_tmppath/%name-%version-build
Summary: Open Remote Play
BuildRequires: curl-devel
BuildRequires: ffmpeg-devel
BuildRequires: openssl-devel
BuildRequires: SDL-devel
BuildRequires: SDL_image-devel
BuildRequires: SDL_net-devel
BuildRequires: SDL_ttf-devel

%description
Open Remote Play is an open source implementation of Sony Computer
Entertainment's Remote Play protocol. Remote Play is a feature on the
PlayStation 3 and PlayStation Portable which allows a PlayStation
Portable user to interact with their PlayStation 3's Xross Media Bar.
Users can access music, videos, photos, PlayStation games, The
PlayStation Store and various applications stored on the
PlayStation 3's HDD, or external flash drives and optical media
attached to the PlayStation 3.
Please report bugs to: http://code.google.com/p/open-rp/

# Build
%prep
%setup -q
%{configure}

%build
make %{?_smp_mflags}

# Install
%install
make install DESTDIR=%{buildroot}

# Clean-up
%clean
[ "%{buildroot}" != "/" ] && rm -rf %{buildroot}

# Files
%files
%defattr(-,root,root)
%{_bindir}/orp
%{_bindir}/wxorp
