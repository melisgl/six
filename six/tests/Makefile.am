## Makefile.am for six tests
check_PROGRAMS = 	hexboardtest hexgametest \
			grouptest carriertest connectortest circuittest

EXTRA_PROGRAMS =	auto-player

EXTRA_DIST = pbem.txt strengthtest.sh start-positions start-positions-big

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
TESTS = $(check_PROGRAMS)

# set the include path for X, qt and KDE
INCLUDES         = -I$(top_srcdir)/six $(all_includes)


hexboardtest_LDADD =	$(top_srcdir)/six/hexmark.o \
			$(top_srcdir)/six/hexboard.o

hexboardtest_SOURCES = hexboardtest.cpp

hexgametest_LDADD =	$(top_srcdir)/six/hexmark.o \
			$(top_srcdir)/six/hexmove.o \
			$(top_srcdir)/six/hexboard.o \
			$(top_srcdir)/six/hexgamestate.o \
			$(top_srcdir)/six/hexgame.o

hexgametest_SOURCES = hexgametest.cpp

grouptest_LDADD =	$(top_srcdir)/six/hexmark.o \
			$(top_srcdir)/six/hexmove.o \
			$(top_srcdir)/six/hexboard.o \
			$(top_srcdir)/six/carrier.o \
			$(top_srcdir)/six/group.o

grouptest_SOURCES = grouptest.cpp

carriertest_LDADD = $(top_srcdir)/six/carrier.o

carriertest_SOURCES = carriertest.cpp

connectortest_LDADD =	$(top_srcdir)/six/hexmark.o \
			$(top_srcdir)/six/hexmove.o \
			$(top_srcdir)/six/hexboard.o \
			$(top_srcdir)/six/hexgamestate.o \
			$(top_srcdir)/six/hexgame.o \
			$(top_srcdir)/six/group.o \
			$(top_srcdir)/six/carrier.o \
			$(top_srcdir)/six/batch.o \
			$(top_srcdir)/six/connector.o

connectortest_SOURCES = testutil.h testutil.cpp \
			connectortest.cpp


circuittest_LDADD =	$(top_srcdir)/six/hexmark.o \
			$(top_srcdir)/six/hexmove.o \
			$(top_srcdir)/six/hexboard.o \
			$(top_srcdir)/six/group.o \
			$(top_srcdir)/six/carrier.o \
			$(top_srcdir)/six/batch.o \
			$(top_srcdir)/six/connector.o \
			$(top_srcdir)/six/lssolve.o \
			$(top_srcdir)/six/circuit.o

circuittest_SOURCES = 	testutil.h testutil.cpp \
				circuittest.cpp

auto_player_LDFLAGS = $(KDE_RPATH) $(all_libraries)

auto_player_LDADD =	$(top_srcdir)/six/hexmark.o \
			$(top_srcdir)/six/hexmove.o \
			$(top_srcdir)/six/hexboard.o \
			$(top_srcdir)/six/hexgamestate.o \
			$(top_srcdir)/six/hexgame.o \
			$(top_srcdir)/six/hexmatch.o \
			$(top_srcdir)/six/group.o \
			$(top_srcdir)/six/carrier.o \
			$(top_srcdir)/six/batch.o \
			$(top_srcdir)/six/connector.o \
			$(top_srcdir)/six/lssolve.o \
			$(top_srcdir)/six/circuit.o \
			$(top_srcdir)/six/sixplayer.o \
			-lm $(LIB_KFILE)

auto_player_SOURCES = 	auto-player.cpp
