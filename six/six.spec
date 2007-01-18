Name: six
Summary: six -- Hex playing program for KDE
Version: 0.5.2
Release: 1
Copyright: GPL
Group: X11/KDE/Games/Board
Source: http://six.retes.hu/download/six-0.5.2.tar.gz
Packager: Gabor Melis <mega@hotpop.com>
BuildRoot: /tmp/six-0.5.2
Prefix: /usr

%description
Hex playing program for KDE.

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q
make -f Makefile.cvs

%build
env "CXXFLAGS=$RPM_OPT_FLAGS" ./configure --disable-debug --enable-final --prefix=%{prefix}
make

%install
make DESTDIR=$RPM_BUILD_ROOT install-strip
cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(-\,root\,root) \%dir ,' > \
	$RPM_BUILD_DIR/master.list
find . -type f | sed -e 's,^\.,\%attr(-\,root\,root) ,' \
	-e '/\/config\//s|^|%config|' >> \
	$RPM_BUILD_DIR/master.list
find . -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> \
	$RPM_BUILD_DIR/master.list
echo "%docdir /usr/doc/kde" >> $RPM_BUILD_DIR/master.list

%clean
rm -rf $RPM_BUILD_ROOT $RPM_BUILD_DIR/master.list


%files -f ../master.list

%changelog
