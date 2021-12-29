Summary:	tasker - a parallel execution command
Name:		tasker
Version:	0.9
Release:	1%{?dist}
License:	GPLv2+
URL:		https://github.com/stevegrubb/tasker
Packager:	Steve Grubb <sgrubb@redhat.com>
Source:		%{name}-%{version}.tar.gz
BuildRequires:	gcc make


%description
Tasker is a program that takes a list from stdin and runs the given command
passing one line from stdin to the command. It determines how many cores to
use, sets affinity to specific hyperthreads, and keeps all hyperthreads busy
until the input pipeline is complete.

%prep
%setup -q

%build
%configure
make CFLAGS="%{optflags}" %{?_smp_mflags}

%install
make DESTDIR="%{buildroot}" INSTALL='install -p' install

%files
%defattr(755, root, root)
%{_bindir}/*
%{_mandir}/*

%changelog
* Wed Dec 29 2021 Steve Grubb <sgrubb@redhat.com> 0.9-1
- Created initial package.

