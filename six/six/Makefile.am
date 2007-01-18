## Makefile.am for six

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
#SUBDIRS = .

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = six

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# the library search path. 
six_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
six_LDADD   = -lm $(LIB_KFILE) -lkdeprint

# which sources should be compiled for six
six_SOURCES =	misc.h misc.cpp \
		poi.h \
		vec.h \
		mat.h \
		lssolve.h lssolve.cpp \
		hexmark.h hexmark.cpp \
		hexfield.h \
		hexmove.h hexmove.cpp \
		hexboard.h hexboard.cpp \
		hexgamestate.h hexgamestate.cpp \
		hexgame.h hexgame.cpp \
		hexmatch.h hexmatch.cpp \
		hexplayer.h \
		group.h group.cpp \
		carrier.h carrier.cpp \
		slicedtask.h \
		batch.h batch.cpp \
		connector.h connector.cpp \
		circuit.h circuit.cpp \
		sixplayer.h sixplayer.cpp \
		asyncplayer.h asyncplayer.cpp \
		khexwidget.h khexwidget.cpp \
		ksix.h ksix.cpp \
		main.cpp

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/six.pot

KDE_ICON = six swap blackplayer whiteplayer

# this is where the kdelnk file will go 
kdelnkdir   = $(kde_appsdir)/Games/Board
kdelnk_DATA = six.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/six
rc_DATA = sixui.rc

mimedir = $(kde_mimedir)/application
mime_DATA = vnd.kde.six.desktop
EXTRA_DIST = $(mime_DATA)
