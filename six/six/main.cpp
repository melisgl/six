#include "config.h"

#include "ksix.h"
#include "carrier.h"
#include "misc.h"

#include <kapp.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>

using std::cin;

static const char *description = I18N_NOOP("Six is a Hex playing program.");

static const char *version = "v" VERSION;

static KCmdLineOptions options[] =
{
    { "black ", I18N_NOOP("Set black player. Possible values are:\n"
                          "\"human\", \"beginner\", \"intermediate\",\n"
                          "\"advanced\", \"expert\"."), ""},
    { "white ", I18N_NOOP("Set white player. Possible values are:\n"
                          "\"human\", \"beginner\", \"intermediate\",\n"
                          "\"advanced\", \"expert\"."), ""},
    { "pbem-filter", I18N_NOOP("Import PBEM game from standard input."), 0 },
    { "+[FILE]", I18N_NOOP("Start with the game loaded from FILE."), 0 },
    { 0, 0, 0 }
};

int main(int argc, char **argv)
{
  RANDOMIZE();
  Carrier::init();
  KAboutData about("six", I18N_NOOP("Six"), version, description,
                   KAboutData::License_GPL, "(C) 2002-2004 Gábor Melis",
                     0, "http://six.retes.hu/", "mega@hotpop.com");
  about.addAuthor("Gábor Melis", 0, "mega@hotpop.com");
  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions(options);
  KApplication app;

  // register ourselves as a dcop client
  app.dcopClient()->registerAs(app.name(), false);

  // see if we are starting with session management
  if (app.isRestored()) {
    RESTORE(KSix);
  } else {
    // no session.. just start up normally
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    KSix *widget = new KSix;
    widget->show();
    if(args->isSet("pbem-filter")) {
      widget->importPBEMGame(cin, i18n("standard input"));
    }
    if(args->getOption("black") != "") {
      KSix::PlayerType black = KSix::stringToPlayer(args->getOption("black"));
      if(black == KSix::PLAYER_NONE) {
        KCmdLineArgs::usage(i18n("Invalid value for black player: %1")
                            .arg(args->getOption("black")));
      } else {
        widget->setBlack(black);
      }
    }
    if(args->getOption("white") != "") {
      KSix::PlayerType white = KSix::stringToPlayer(args->getOption("white"));
      if(white == KSix::PLAYER_NONE) {
        KCmdLineArgs::usage(i18n("Invalid value for white player: %1")
                            .arg(args->getOption("white")));
      } else {
        widget->setWhite(white);
      }
    }
    for(int i = 0; i < args->count(); i++) {
      widget->loadGame(QFile::decodeName(args->arg(i)));
    }
    args->clear();
  }

  return app.exec();
}
