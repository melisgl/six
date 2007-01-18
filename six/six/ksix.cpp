#include "ksix.h"

#include "sixplayer.h"
#include "asyncplayer.h"
#include "misc.h"
#include "config.h"

#include <unistd.h>

#include <cassert>
#include <fstream>

#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qbuttongroup.h>
#include <qhbox.h>

#include <kprinter.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmenubar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurlrequesterdlg.h>
#include <kcursor.h>
#include <kedittoolbar.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <klineeditdlg.h>

#include <sstream>
#include <fstream>

using std::endl;
using std::istringstream;
using std::ostringstream;
using std::ifstream;
using std::ofstream;

QString KSix::secToQString(int sec)
{
  int h = sec / 3600;
  int m = (sec % 3600) / 60;
  int s = (sec % 60);
  QString r;
  r.sprintf("%d:%.2d:%.2d", h, m, s);
  return r;
}

QString KSix::moveToString(const HexMove &m, int n, const HexBoard &b)
{
  assert(!m.isNull());
  QString r;
  if(m.isSwap()) {
    r = i18n("%1. Swap").arg(n);
  } else if(m.isResign()) {
    r = i18n("%1. Resign").arg(n);
  } else if(m.isForfeit()) {
    r = i18n("%1. Forfeit").arg(n);
  } else {
    int xc, yc;
    if(isTransverted() && (n > 2))
      b.field2Coords(m.field(), &yc, &xc);
    else
      b.field2Coords(m.field(), &xc, &yc);
    r = i18n("%1. %2%3").arg(n).arg(QChar('A' + xc)).arg(yc + 1);
  }
  return r;
}

QString KSix::playerToString(PlayerType p)
{
  switch(p) {
  case PLAYER_HUMAN:
    return "human";
  case PLAYER_BEGINNER:
    return "beginner";
  case PLAYER_INTERMEDIATE:
    return "intermediate";
  case PLAYER_ADVANCED:
    return "advanced";
  case PLAYER_EXPERT:
    return "expert";
  default:
    return QString::null;
  }
}

KSix::PlayerType KSix::stringToPlayer(const QString &s)
{
  if(s == "human") {
    return PLAYER_HUMAN;
  }
  if(s == "beginner") {
    return PLAYER_BEGINNER;
  }
  if(s == "intermediate") {
    return PLAYER_INTERMEDIATE;
  }
  if(s == "advanced") {
    return PLAYER_ADVANCED;
  }
  if(s == "expert") {
    return PLAYER_EXPERT;
  }
  return PLAYER_NONE;
}

HexPlayer *KSix::createPlayer(PlayerType p)
{
  bool allowResign = _toggleResignAction->isChecked();
  switch(p) {
  case PLAYER_HUMAN:
    return new AsyncPlayer();
  case PLAYER_BEGINNER:
    return new SixPlayer(SixPlayer::BEGINNER, allowResign, this);
  case PLAYER_INTERMEDIATE:
    return new SixPlayer(SixPlayer::INTERMEDIATE, allowResign, this);
  case PLAYER_ADVANCED:
    return new SixPlayer(SixPlayer::ADVANCED, allowResign, this);
  case PLAYER_EXPERT:
    return new SixPlayer(SixPlayer::EXPERT, allowResign, this);
  default:
    assert(0);
  }
  // suppress warning
  return 0;
}

KSix::KSix()
    : KMainWindow(0, "six"),
      _hexWidget(new KHexWidget(this)),
      _printer(0)
{
  _blackPlayer = PLAYER_NONE;
  _whitePlayer = PLAYER_NONE;

  _killingBlackPlayer = false;
  _killingWhitePlayer = false;
  _stoppingMatch = false;
  _thinking = HEX_MARK_EMPTY;

  // tell the KMainWindow that this is indeed the main widget
  setCentralWidget(_hexWidget);

  // then, setup our actions
  setupActions();

  // and a status bar
  statusBar()->show();
  _messageLabel = new ClickableLabel(statusBar());
  _messageLabel->setAlignment(AlignLeft | AlignVCenter);
  statusBar()->addWidget(_messageLabel, 1, 1);
  connect(_messageLabel, SIGNAL(released()), this, SLOT(flash()));

  // add a frame containing black's clock
  {
    QHBox *hbox= new QHBox(statusBar(), 0);
    _blackClockMark = new QLabel(hbox, 0);
    _blackClockMark->setPixmap(BarIcon("blackplayer"));
    _blackClockLabel = new QLabel(hbox);
    statusBar()->addWidget(hbox, 0, 1);
  }
  // add a frame containing white's clock
  {
    QHBox *hbox= new QHBox(statusBar(), 0);
    _whiteClockMark = new QLabel(hbox, 0);
    _whiteClockMark->setPixmap(BarIcon("whiteplayer"));
    _whiteClockLabel = new QLabel(hbox);
    statusBar()->addWidget(hbox, 0, 1);
  }

  connect(_hexWidget, SIGNAL(signalClickLeft(HexField)),
          this, SLOT(clickLeft(HexField)));

  readOptions();
}

KSix::~KSix()
{
}

void KSix::setupActions()
{
    _gameNewAction =
      new KAction(i18n("&New Game"), "filenew",
                  KStdAccel::key(KStdAccel::New),
                  this, SLOT(gameNew()),
                  actionCollection(), "fileNewGame");

    new KAction(i18n("&Open Game"), "fileopen",
                KStdAccel::key(KStdAccel::Open),
                this, SLOT(fileOpen()),
                actionCollection(), "fileOpenGame");

    new KAction(i18n("&Save Game"), "filesave",
                KStdAccel::key(KStdAccel::Save),
                this, SLOT(fileSave()),
                actionCollection(), "fileSaveGame");

    new KAction(i18n("&Save Game As"), "filesaveas",
                0,
                this, SLOT(fileSaveAs()),
                actionCollection(), "fileSaveGameAs");

    new KAction(i18n("&Import PBEM Game"),
                CTRL + Key_I,
                this, SLOT(fileImportPBEMGame()),
                actionCollection(), "fileImportPBEMGame");

    new KAction(i18n("&Print Position"), "fileprint",
                KStdAccel::key(KStdAccel::Print),
                this, SLOT(filePrint()),
                actionCollection(), "filePrintPosition");

    KStdAction::quit(this, SLOT(close()), actionCollection());

    // game
    _gamePreviousMoveAction =
      new KAction(i18n("&Previous Move"), "player_rew",
                  KStdAccel::key(KStdAccel::Prior),
                  this, SLOT(gamePreviousMove()),
                  actionCollection(), "gamePreviousMove");

    _gameNextMoveAction =
      new KAction(i18n("&Next Move"), "player_fwd",
                  KStdAccel::key(KStdAccel::Next),
                  this, SLOT(gameNextMove()),
                  actionCollection(), "gameNextMove");

    _gameFirstMoveAction =
      new KAction(i18n("&First Move"), "player_start",
                  Key_Home,
                  this, SLOT(gameFirstMove()),
                  actionCollection(), "gameFirstMove");

    _gameLastMoveAction =
      new KAction(i18n("&Last Move"), "player_end",
                  Key_End,
                  this, SLOT(gameLastMove()),
                  actionCollection(), "gameLastMove");

    _gameSuspendAction =
      new KAction(i18n("&Pause"), "player_pause",
                  Key_P, this, SLOT(gameSuspend()),
                  actionCollection(), "gameSuspend");

    _gameResumeAction =
      new KAction(i18n("&Continue"), "player_play",
                  Key_C, this, SLOT(gameResume()),
                  actionCollection(), "gameResume");

    _gameMoveAction =
      new KAction(i18n("&Move"), "move",
                  Key_M, this, SLOT(gameMove()),
                  actionCollection(), "gameMove");

    _gameSwapAction =
      new KAction(i18n("&Swap"), "swap",
                  Key_S, this, SLOT(gameSwap()),
                  actionCollection(), "gameSwap");

    _gameResignAction =
      new KAction(i18n("&Resign"), "button_cancel",
                  Key_R, this, SLOT(gameResign()),
                  actionCollection(), "gameResign");

    // bars
    _toolbarAction = KStdAction::showToolbar(this, SLOT(showToolbar()),
                                             actionCollection());
    _statusbarAction = KStdAction::showStatusbar(this, SLOT(showStatusbar()),
                                                 actionCollection());

    // swap allowed
    _toggleSwapAction =
      new KToggleAction(i18n("Swa&p Allowed"), 0, this, SLOT(toggleSwap()),
                        actionCollection(), "swapAllowed");

    // board size
    {
      QStringList itemList;
      _boardSizeAction = new KSelectAction(i18n("Board Si&ze"), 0,
                                             actionCollection(), "boardSize");
      itemList.append(i18n("&4"));
      itemList.append(i18n("&5"));
      itemList.append(i18n("&6"));
      itemList.append(i18n("&7"));
      itemList.append(i18n("&8"));
      itemList.append(i18n("&9"));
      itemList.append(i18n("&10"));
      itemList.append(i18n("&11"));
#ifndef OLYMPICS
      itemList.append(i18n("&12"));
      itemList.append(i18n("&13"));
      itemList.append(i18n("&14"));
      itemList.append(i18n("&15"));
#endif
      _boardSizeAction->setItems(itemList);
      connect(_boardSizeAction, SIGNAL(activated(int)),
              this, SLOT(setBoardSizeByMenuItem(int)));
    }

    // resign allowed
    _toggleResignAction =
      new KToggleAction(i18n("Resign Allowed"), 0, this, SLOT(toggleResign()),
                        actionCollection(), "resignAllowed");

    // numbering
    _toggleNumberingAction = 
      new KToggleAction(i18n("Move &Numbering"), 0,
                        this, SLOT(toggleNumbering()),
                        actionCollection(), "moveNumbering");

    // show thinking
    _showThinkingAction = 
      new KToggleAction(i18n("Show Th&inking"), 0,
                        this, SLOT(toggleShowThinking()),
                        actionCollection(), "showThinking");

    // swap style
    {
      QStringList itemList;
      _swapStyleAction =
        new KSelectAction(i18n("Swap St&yle"), 0,
                          actionCollection(), "swapStyle");
      itemList.append(i18n("&Board"));
      itemList.append(i18n("&Players"));
      _swapStyleAction->setItems(itemList);
      connect(_swapStyleAction, SIGNAL(activated(int)),
              this, SLOT(setSwapStyleByMenuItem(int)));
    }

    // display mode
    {
      QStringList itemList;
      _displayModeAction =
        new KSelectAction(i18n("&Display Mode"), 0,
                          actionCollection(), "displayMode");
      itemList.append(i18n("&Diagonal"));
      itemList.append(i18n("&Flat"));
      _displayModeAction->setItems(itemList);
      connect(_displayModeAction, SIGNAL(activated(int)),
              this, SLOT(setDisplayModeByMenuItem(int)));
    }

    // black player
    {
      QStringList itemList;
      _blackPlayerAction =
        new KSelectAction(i18n("&Black Player"), "blackplayer", 0,
                          actionCollection(), "blackPlayer");
      itemList.append(i18n("&Human"));
      itemList.append(i18n("&Beginner"));
      itemList.append(i18n("&Intermediate"));
      itemList.append(i18n("&Advanced"));
      itemList.append(i18n("&Expert"));
      _blackPlayerAction->setItems(itemList);
      connect(_blackPlayerAction, SIGNAL(activated(int)),
              this, SLOT(setBlackPlayerByMenuItem(int)));
    }

    // white player
    {
      QStringList itemList;
      _whitePlayerAction =
        new KSelectAction(i18n("&White Player"), "whiteplayer", 0,
                          actionCollection(), "whitePlayer");
      itemList.append(i18n("&Human"));
      itemList.append(i18n("&Beginner"));
      itemList.append(i18n("&Intermediate"));
      itemList.append(i18n("&Advanced"));
      itemList.append(i18n("&Expert"));
      _whitePlayerAction->setItems(itemList);
      connect(_whitePlayerAction, SIGNAL(activated(int)),
              this, SLOT(setWhitePlayerByMenuItem(int)));
    }

    // configure
    KStdAction::keyBindings(this, SLOT(configureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(configureToolbars()),
                                  actionCollection());
    KStdAction::saveOptions(this, SLOT(saveOptions()), actionCollection());

    createGUI();
}

void KSix::saveStandardProperties(KConfig *config)
{
  // the 'config' object points to the session managed
  // config file.  anything you write here will be available
  // later when this app is restored
  config->setGroup("General");
  config->writeEntry("version", "0.1");
  config->setGroup("Appearance");
  config->writeEntry("geometry", size());
  config->writeEntry("showToolBar", _toolbarAction->isChecked());
  config->writeEntry("showStatusBar", _statusbarAction->isChecked());
  config->setGroup("Game Options");
  config->writeEntry("swapAllowed", _toggleSwapAction->isChecked());
  config->writeEntry("boardSize", _boardSize);
  config->writeEntry("blackPlayer", playerToString(_blackPlayer));
  config->writeEntry("whitePlayer", playerToString(_whitePlayer));
  config->writeEntry("resignAllowed", _toggleResignAction->isChecked());
  config->setGroup("Display Options");
  config->writeEntry("moveNumbering", _toggleNumberingAction->isChecked());
  config->writeEntry("showThinking", _showThinkingAction->isChecked());
  if(_swapStyleAction->currentItem() == 0) {
    config->writeEntry("swapStyle", "board");
  } else {
    config->writeEntry("swapStyle", "players");
  }
  if(_displayModeAction->currentItem() == 0) {
    config->writeEntry("displayMode", "diagonal");
  } else {
    config->writeEntry("displayMode", "flat");
  }
}

void KSix::saveGameProperties(KConfig *config)
{
  Poi<HexMatch> m = ((!_match.null()) ? _match : _oldMatch);
  assert(!m.null());
  ostringstream os;
  (*m).game().save(os);
  config->setGroup("Game State");
  config->writeEntry("game", os.str().c_str());
  config->writeEntry("gameChanged", (*m).game().isChanged());
  config->writeEntry("gameBranched", (*m).game().isBranched());
  config->writeEntry("gameFilename", _filename);
}

void KSix::saveProperties(KConfig *config)
{
  gameSuspend();
  saveStandardProperties(config);
  saveGameProperties(config);
  config->sync();
}

void KSix::readStandardProperties(KConfig *config)
{
  // the 'config' object points to the session managed
  // config file.  this function is automatically called whenever
  // the app is being restored.  read in here whatever you wrote
  // in 'saveProperties'
  config->setGroup("Appearance");
  QSize defSize(600, 500);
  QSize size = config->readSizeEntry("geometry", &defSize);
  bool stb = config->readBoolEntry("showToolBar", true);
  bool ssb = config->readBoolEntry("showStatusBar", true);
  config->setGroup("Game Options");
  bool sa = config->readBoolEntry("swapAllowed", true);
  int bs = config->readNumEntry("boardSize", 11);
  PlayerType bp = stringToPlayer(config->readEntry("blackPlayer", "human"));
  PlayerType wp = stringToPlayer(config->readEntry("whitePlayer", "beginner"));
  bool ra = config->readBoolEntry("resignAllowed", true);
  config->setGroup("Display Options");
  bool mn = config->readBoolEntry("moveNumbering", true);
  bool st = config->readBoolEntry("showThinking", false);
  QString dm = config->readEntry("displayMode", "diagonal");
  QString ss = config->readEntry("swapStyle", "board");

  resize(size);

  if(_toolbarAction->isChecked() != stb)
    _toolbarAction->activate();
  if(_statusbarAction->isChecked() != ssb)
    _statusbarAction->activate();

  setBlack(bp);
  setWhite(wp);

  assert(4 <= bs && bs <= 15);
  _boardSizeAction->setCurrentItem(bs - 4);
  _boardSize = bs;

  if(_toggleSwapAction->isChecked() != sa)
    _toggleSwapAction->activate();
  if(_toggleResignAction->isChecked() != ra)
    _toggleResignAction->activate();
  if(_match.null())
    newMatch();

  if(_toggleNumberingAction->isChecked() != mn)
    _toggleNumberingAction->activate();
  if(_showThinkingAction->isChecked() != st)
    _showThinkingAction->activate();

  if(dm == "diagonal") {
    _displayModeAction->setCurrentItem(0);
    setDisplayModeByMenuItem(0);
  } else {
    _displayModeAction->setCurrentItem(1);
    setDisplayModeByMenuItem(1);
  }

  if(ss == "board") {
    _swapStyleAction->setCurrentItem(0);
    setSwapStyleByMenuItem(0);
  } else {
    _swapStyleAction->setCurrentItem(1);
    setSwapStyleByMenuItem(1);
  }
}

void KSix::readGameProperties(KConfig *config)
{
  config->setGroup("Game State");
  bool changed = config->readBoolEntry("gameChanged", false);
  bool branched = config->readBoolEntry("gameBranched", false);
  QString filename = config->readEntry("gameFilename", "");
  QString savedGame = config->readEntry("game", "");
  istringstream is((std::string)savedGame.latin1());
  HexGame g;
  g.load(is);
  g.setChanged(changed);
  g.setBranched(branched);
  setFilename(filename);
  if(is.fail()) {
    DBG << "Six could not restore session." << std::endl;
  } else {
    newMatch(g);
  }
}

void KSix::readProperties(KConfig *config)
{
  readStandardProperties(config);
  readGameProperties(config);
  //...
}

void KSix::saveOptions()
{
  KConfig *config = KGlobal::config();
  saveStandardProperties(config);
  //  saveProperties(config);
  config->sync();
}

void KSix::readOptions()
{
  KConfig *config = KGlobal::config();
  readStandardProperties(config);
  //readProperties(config);
}

bool KSix::verifyUnsavedGame(const QString &question)
{
  return (!(*_match).game().isChanged() ||
          KMessageBox::warningYesNo(this, question)
          == KMessageBox::Yes);
}

bool KSix::loadGame(const QString &filename)
{
  ifstream is(filename.latin1());
  HexGame g;
  g.load(is);
  if(!is.fail() && 4 <= g.board().xs() && g.board().xs() <= 15 &&
     g.board().xs() == g.board().ys()) {
    setFilename(filename);
    // sync actions with reality
    int bs = g.board().xs();
    _boardSizeAction->setCurrentItem(bs - 4);
    _boardSize = bs;
    _toggleSwapAction->setChecked(g.initialState().swappable());
    // create new match
    newMatch(g);
    return true;
  } else {
    KMessageBox::error(this, i18n("Loading '%1' failed.").arg(filename));    
    return false;
  }
}

bool KSix::saveGame(const QString &filename)
{
  ofstream os(filename.latin1());
  (*_match).game().save(os);
  if(!os.fail()) {
    setFilename(filename);
    (*_match).setChanged(false);
    (*_match).setBranched(false);
    return true;
  } else {
    KMessageBox::error(this, i18n("Saving '%1' failed.").arg(filename));
    return false;
  }
}

bool KSix::importPBEMGame(istream &is, const QString &filename)
{
  HexGame g;
  g.importPBEMGame(is);
  if(!is.fail() && 4 <= g.board().xs() && g.board().xs() <= 15 &&
     g.board().xs() == g.board().ys()) {
    setFilename("");
    // sync actions with reality
    int bs = g.board().xs();
    _boardSizeAction->setCurrentItem(bs - 4);
    _boardSize = bs;
    _toggleSwapAction->setChecked(g.initialState().swappable());
    // create new match
    newMatch(g);
    return true;
  } else {
    KMessageBox::error(this, i18n("Importing '%1' failed.").arg(filename));
    return false;
  }
}

bool KSix::importPBEMGame(const QString &filename)
{
  ifstream is(filename.latin1());
  return importPBEMGame(is, filename);
}

void KSix::fileOpen()
{
  const QString q =
    i18n("This game has not been saved. Continue loading?");
  QString filename = KFileDialog::getOpenFileName(QString::null, "*.six\n*",
                                                  0, i18n("Open Game"));
  if(!filename.isEmpty()) {
    if(verifyUnsavedGame(q)) {
      loadGame(filename);
    }
  }
}

void KSix::fileSave()
{
  if(_filename.isEmpty()) {
    fileSaveAs();
  } else {
    if((*_match).game().isBranched()) {
      QString w = i18n("This game has been branched by moving backwards\n"
                       "in move history and starting play from that position.\n"
                       "Saving will overwrite the original game. Continue?");
      if(KMessageBox::warningYesNo(this, w) != KMessageBox::Yes)
        return;
    }
    saveGame(_filename);
  }
}

void KSix::fileSaveAs()
{
  QString filename = KFileDialog::getSaveFileName(QString::null, "*.six\n*",
                                                  0, i18n("Save Game"));
  if(!filename.isEmpty()) {
    if(filename.right(4) != ".six") {
      filename.append(".six");
    }
    FILE *fp;
    if((fp = fopen(filename.latin1(), "r")) != NULL) {
      fclose(fp);
      if(KMessageBox::warningYesNo(this,
                                   i18n("File '%1' exists. Overwrite?")
                                   .arg(filename))
          != KMessageBox::Yes)
        return;
    }
    saveGame(filename);
  }
}

void KSix::fileImportPBEMGame()
{
  const QString q =
    i18n("This game has not been saved. Continue importing?");
  QString filename =
    KFileDialog::getOpenFileName(QString::null, "*", 0,
                                 i18n("Import PBEM Game"));
  if(!filename.isEmpty()) {
    if(verifyUnsavedGame(q)){
      importPBEMGame(filename);
    }
  }
}

void KSix::filePrint()
{
  // this slot is called whenever the File->Print menu is selected,
  // the Print shortcut is pressed (usually CTRL+P) or the Print toolbar
  // button is clicked
  if(!_printer) 
    _printer = new KPrinter;
  if(_printer->setup(this)) {
    QPainter p;
    p.begin(_printer);

    // we let our view do the actual printing
    QPaintDeviceMetrics metrics(_printer);
    _hexWidget->print(&p, QColor("black"), metrics.width(), metrics.height());

    // and send the result to the printer
    p.end();
  }
}

bool KSix::queryClose()
{
  const QString q =
    i18n("This game has not been saved. Quit anyway?");
  if(!verifyUnsavedGame(q)) {
    return false;
  } else {
    stopMatch();
    return true;
  }
}

void KSix::showToolbar()
{
  if(_toolbarAction->isChecked())
    toolBar()->show();
  else
    toolBar()->hide();
}

void KSix::showStatusbar()
{
  if(_statusbarAction->isChecked())
    statusBar()->show();
  else
    statusBar()->hide();
}

void KSix::configureKeys()
{
  KKeyDialog::configureKeys(actionCollection(), "sixui.rc");
}

void KSix::configureToolbars()
{
  // use the standard toolbar editor
  KEditToolbar dlg(actionCollection());
  if(dlg.exec()) {
    // recreate our GUI
    createGUI();
  } 
}

void KSix::updateBoard()
{
  if(!_match.null()) {
    HexBoard b((*_match).game().board());
    Grouping bg;
    bg.init(b, HEX_MARK_VERT);
    Grouping wg;
    wg.init(b, HEX_MARK_HORI);
    HexBoard deadVert(b.xs(), b.ys());
    HexBoard deadHori(b.xs(), b.ys());
    HexBoard uselessVert(b.xs(), b.ys());
    HexBoard uselessHori(b.xs(), b.ys());
#if 0
    for(HexField f = HexBoard::FIRST_NORMAL_FIELD; f < b.size(); f++) {
      if(bg(f).null())
        deadVert.set(f, HEX_MARK_VERT);
      if(bg.uselessFields().has(f))
        uselessVert.set(f, HEX_MARK_VERT);
      if(wg(f).null())
        deadHori.set(f, HEX_MARK_HORI);
      if(wg.uselessFields().has(f))
        uselessHori.set(f, HEX_MARK_HORI);
    }
#endif
    if(isTransverted())
      _hexWidget->setGame((*_match).game().transvert(), deadHori.transvert(),
                          deadVert.transvert(), uselessVert.transvert(),
                          uselessHori.transvert());
    else
      _hexWidget->setGame((*_match).game(), deadVert, deadHori,
                          uselessVert, uselessHori);
    if((*_match).status() != HexMatch::MATCH_OFF)
      flash();
  }
}

void KSix::updateActions()
{
  if(!_match.null()) {
    const HexGame &g = (*_match).game();
    _gamePreviousMoveAction->setEnabled((*_match).canBack());
    _gameNextMoveAction->setEnabled((*_match).canForward());
    _gameFirstMoveAction->setEnabled((*_match).canBack());
    _gameLastMoveAction->setEnabled((*_match).canForward());
    _gameSuspendAction->setEnabled((*_match).status() == HexMatch::MATCH_ON);
    _gameResumeAction->setEnabled((*_match).status() == HexMatch::MATCH_OFF);
    bool e = g.board().isEmpty() && !g.isChanged() && _filename.isEmpty() &&
      (_thinking == HEX_MARK_EMPTY);
    _toggleSwapAction->setEnabled(e);
    _boardSizeAction->setEnabled(e);
    bool m = (*_match).status() != HexMatch::MATCH_FINISHED &&
      ((g.next() == HEX_MARK_VERT && _blackPlayer == PLAYER_HUMAN) ||
       (g.next() == HEX_MARK_HORI && _whitePlayer == PLAYER_HUMAN));
    bool s = m && g.isValidMove(HexMove::createSwap(g.next()));
    _gameMoveAction->setEnabled(m);
    _gameResignAction->setEnabled(m);
    _gameSwapAction->setEnabled(s);
  } else {
    _gamePreviousMoveAction->setEnabled(false);
    _gameNextMoveAction->setEnabled(false);
    _gameFirstMoveAction->setEnabled(false);
    _gameLastMoveAction->setEnabled(false);
    _gameSuspendAction->setEnabled(false);
    _gameResumeAction->setEnabled(false);
    _gameMoveAction->setEnabled(false);
    _gameSwapAction->setEnabled(false);
  }
  if(isTransverted()) {
    _blackPlayerAction->setText(i18n("&White Player (swapped)"));
    _whitePlayerAction->setText(i18n("&Black Player (swapped)"));
  } else {
    _blackPlayerAction->setText(i18n("&Black Player"));
    _whitePlayerAction->setText(i18n("&White Player"));
  }
}

void KSix::updateClock()
{
  if(!_match.null()) {
    long bc = (isTransverted() ?
               (*_match).horiClockTotal() : (*_match).vertClockTotal());
    long wc = (isTransverted() ?
               (*_match).vertClockTotal() : (*_match).horiClockTotal());
    _blackClockLabel->setText(secToQString(bc / 1000));
    _whiteClockLabel->setText(secToQString(wc / 1000));
    if((*_match).status() != HexMatch::MATCH_ON) {
      _blackClockMark->setEnabled(false);
      _whiteClockMark->setEnabled(false);
    } else if((*_match).game().next() == transvertMark(HEX_MARK_VERT)) {
      _blackClockMark->setEnabled(true);
      _whiteClockMark->setEnabled(false);
    } else {
      _blackClockMark->setEnabled(false);
      _whiteClockMark->setEnabled(true);
    }
  } else {
    _blackClockLabel->setText(secToQString(0));
    _whiteClockLabel->setText(secToQString(0));
    _blackClockMark->setEnabled(false);
    _whiteClockMark->setEnabled(false);
  }
}

void KSix::updateCaption()
{
  QString caption;
  if(!_match.null()) {
    caption = _filename;
    if((*_match).game().isBranched())
      caption += i18n("(**)");
    else if((*_match).game().isChanged())
      caption += i18n("(*)");
  }
  setCaption(caption);
}

void KSix::updateMessage()
{
  if(!_match.null()) {
    QString statusMessage;
    if((*_match).status() == HexMatch::MATCH_FINISHED) {
      if((*_match).game().winner() == transvertMark(HEX_MARK_VERT)) {
        statusMessage = i18n("Black has won.");
      } else {
        statusMessage = i18n("White has won.");
      }
    } else {
      if((*_match).game().next() == transvertMark(HEX_MARK_VERT)) {
        statusMessage = i18n("Black's turn.");
      } else {
        statusMessage = i18n("White's turn.");
      }
    }

    HexGame::ReverseIterator rend = (*_match).game().rend();
    HexGame::ReverseIterator last = (*_match).game().rbegin();
    HexGame::ReverseIterator lastButOne = (*_match).game().rbegin();
    if(lastButOne != rend)
      lastButOne++;

    int n = (*_match).game().initialState().board().nMark();
    QString m0;
    if(last != rend) {
      const HexMove &m = (*last).move();
      m0 = moveToString(m, n + rend - last, (*_match).game().board());
    }
    QString m1;
    if(lastButOne != rend) {
      const HexMove &m = (*lastButOne).move();
      m1 = moveToString(m, n + rend - lastButOne, (*_match).game().board());
    }

    QString s;
    if(!m1.isEmpty() && !m0.isEmpty()) {
      s = i18n("%1  %2").arg(m1).arg(m0);
    } else if(!m0.isEmpty()) {
      s = m0;
    }
    QString message(i18n("%1   %2"));
    _messageLabel->setText(message.arg(s).arg(statusMessage));
  }
}

void KSix::updateThinking()
{
  if(_showThinkingAction->isChecked() && _thinking != HEX_MARK_EMPTY &&
     !_stoppingMatch && !_killingBlackPlayer && !_killingWhitePlayer) {
    HexMove newCandidateMove;
    if((*_match).status() == HexMatch::MATCH_OFF) {
      _hexWidget->stopFlash();
    } else {
      if(onTurn() == HEX_MARK_VERT)
        newCandidateMove = ((SixPlayer *)&*_black)->candidateMove();
      else
        newCandidateMove = ((SixPlayer *)&*_white)->candidateMove();
      if(!_hexWidget->isFlashing() || _candidateMove != newCandidateMove) {
        _candidateMove = newCandidateMove;
        vector<int> f;
        if(_candidateMove.isNormal()) {
          f.push_back(transvertField(_candidateMove.field()) -
                      HexBoard::FIRST_NORMAL_FIELD);
          _hexWidget->flashHexagons(f, 800, 1000000);
        }
      }
    }
  }
}

void KSix::turn()
{
  assert(!_stoppingMatch || _thinking != HEX_MARK_EMPTY);
  if(!_stoppingMatch && _thinking == HEX_MARK_EMPTY) {
    while(!_match.null() && (*_match).status() == HexMatch::MATCH_ON &&
          !_stoppingMatch && _thinking == HEX_MARK_EMPTY &&
          ((onTurn() == HEX_MARK_VERT && _blackPlayer != PLAYER_HUMAN) ||
           (onTurn() == HEX_MARK_HORI && _whitePlayer != PLAYER_HUMAN))) {
      _thinking = onTurn();
      _hexWidget->setCursor(KCursor::waitCursor());
      _candidateMove = HexMove();
      (*_match).doSome();
      _thinking = HEX_MARK_EMPTY;
      _hexWidget->setCursor(KCursor::arrowCursor());
      _candidateMove = HexMove();
      if(_killingBlackPlayer || _killingWhitePlayer) {
        _oldBlack = 0;
        _oldWhite = 0;
        _killingBlackPlayer = false;
        _killingWhitePlayer = false;
        _hexWidget->stopFlash();
      }
    }
    if(_stoppingMatch) {
      _oldMatch = 0;
      _stoppingMatch = false;
      _hexWidget->stopFlash();
    }
    updateActions();
    updateMessage();
    updateClock();
  }
}


//

void KSix::gameNew()
{
  const QString q =
    i18n("This game has not been saved. Start a new one?");
  if(verifyUnsavedGame(q))
    newMatch();
}

void KSix::gamePreviousMove()
{
  if(!_match.null() && (*_match).canBack() && !_stoppingMatch) {
    _stoppingMatch = killMatch();
    (*_match).back();
  }
}

void KSix::gameNextMove()
{
  if(!_match.null() && (*_match).canForward() && !_stoppingMatch) {
    _stoppingMatch = killMatch();
    (*_match).forward();
  }
}

void KSix::gameFirstMove()
{
  if(!_match.null() && (*_match).canBack() && !_stoppingMatch) {
    _stoppingMatch = killMatch();
    (*_match).backAll();
  }
}

void KSix::gameLastMove()
{
  if(!_match.null() && (*_match).canForward() && !_stoppingMatch) {
    _stoppingMatch = killMatch();
    (*_match).forwardAll();
  }
}

void KSix::gameSuspend()
{
  if(!_match.null() && (*_match).status() == HexMatch::MATCH_ON &&
     !_stoppingMatch) {
    (*_match).off();
  }
}

void KSix::gameResume()
{
  if(!_match.null() && (*_match).status() == HexMatch::MATCH_OFF &&
     !_stoppingMatch) {
    (*_match).on();
    turn();
  }
}

class MoveValidator : public QValidator
{
public:
  MoveValidator(const HexGame &g, QObject *parent)
    : QValidator(parent), _game(g) {};
  QValidator::State validate(QString &input, int &) const
  {
    HexMove m;
    if(input.contains(' '))
      return QValidator::Invalid;
    else {
      return ((_game.parseMove(&m, (const char *)input) &&
               _game.isValidMove(m)) ?
              QValidator::Acceptable : QValidator::Intermediate);
    }
  };
private:
  const HexGame &_game;
};


void KSix::makeMove(const HexMove &move)
{
  if(!_match.null() && !_stoppingMatch) {
    if((*_match).status() != HexMatch::MATCH_FINISHED) {
      const HexGame &g = (*_match).game();
      assert(g.isValidMove(move));
      if(g.next() == HEX_MARK_VERT && _blackPlayer == PLAYER_HUMAN) {
        ((AsyncPlayer *)(&*_black))->play(move);
        (*_match).doSome();
        turn();
      } else if(g.next() == HEX_MARK_HORI && _whitePlayer == PLAYER_HUMAN) {
        ((AsyncPlayer *)(&*_white))->play(move);
        (*_match).doSome();
        turn();
      }
    }
  }
}

void KSix::gameMove()
{
  if(!_match.null() && !_stoppingMatch) {
    const HexGame &g = (*_match).game();
    const HexGame &tg = (isTransverted() ? g.transvert() : g);
    MoveValidator validator(tg, this);
    bool ok;
    QString result =
      KLineEditDlg::getText(i18n("Enter move:"), "", &ok, this, &validator);
    if (ok) {
      HexMove move;
      tg.parseMove(&move, (const char *)result);
      tg.printMove(DBG, move); DBG << std::endl;
      if (isTransverted())
        move = tg.transvert(move);
      assert(g.isValidMove(move));
      makeMove(move);
    }
  }
}

void KSix::gameSwap()
{
  if(!_match.null() && !_stoppingMatch) {
    if((*_match).status() != HexMatch::MATCH_FINISHED) {
      const HexGame &g = (*_match).game();
      HexMove move = HexMove::createSwap(g.next());
      makeMove(move);
    }
  }
}

void KSix::gameResign()
{
  if(!_match.null() && !_stoppingMatch) {
    if((*_match).status() != HexMatch::MATCH_FINISHED) {
      const HexGame &g = (*_match).game();
      HexMove move = HexMove::createResign(g.next());
      makeMove(move);
    }
  }
}

void KSix::setBoardSize(int s)
{
  if(_boardSize != s) {
    _boardSize = s;
    newMatch();
  }
}

void KSix::setBoardSizeByMenuItem(int item)
{
  setBoardSize(item + 4);
}

void KSix::toggleSwap()
{
  newMatch();
}

void KSix::toggleResign()
{
  if(!_black.null() && _blackPlayer != PLAYER_HUMAN) {
    ((SixPlayer *)&*_black)->allowResign(_toggleResignAction->isChecked());
  }
  if(!_white.null() && _whitePlayer != PLAYER_HUMAN) {
    ((SixPlayer *)&*_white)->allowResign(_toggleResignAction->isChecked());
  }
}

void KSix::toggleNumbering()
{
  if(_toggleNumberingAction->isChecked()) {
    _hexWidget->setNumbering(true);
  } else {
    _hexWidget->setNumbering(false);
  }
}

void KSix::toggleShowThinking()
{
  updateThinking();
}

void KSix::setSwapStyleByMenuItem(int i)
{
  // prevent warning
  i = 0;
  updateBoard();
  updateActions();
  updateClock();
  updateCaption();
  updateMessage();
  updateThinking();
}

void KSix::setDisplayModeByMenuItem(int i)
{
  if(i == 0)
    _hexWidget->setDisplayMode(KHexWidget::DISPLAY_DIAGONAL);
  else
    _hexWidget->setDisplayMode(KHexWidget::DISPLAY_FLAT);
}

void KSix::clickLeft(HexField f)
{
  if(!_match.null() && !_stoppingMatch) {
    if((*_match).status() != HexMatch::MATCH_FINISHED) {
      const HexGame &g = (*_match).game();
      if(isTransverted())
        f = g.board().transvert(f);
      HexMove move(g.next(), f);
      if(g.next() == HEX_MARK_VERT && _blackPlayer == PLAYER_HUMAN) {
        ((AsyncPlayer *)(&*_black))->play(move);
        (*_match).doSome();
        turn();
      } else if(g.next() == HEX_MARK_HORI && _whitePlayer == PLAYER_HUMAN) {
        ((AsyncPlayer *)(&*_white))->play(move);
        (*_match).doSome();
        turn();
      }
    }
    // silently ignore the click
  }
}

bool KSix::killBlack()
{
  DBG << "Killing black" << std::endl;
  if((_thinking == HEX_MARK_VERT) && !_black.null()) {
    ((SixPlayer *)&*_black)->cancelMove();
    return true;
  }
  return false;
}

bool KSix::killWhite()
{
  DBG << "Killing white" << std::endl;
  if((_thinking == HEX_MARK_HORI) && !_white.null()) {
    ((SixPlayer *)&*_white)->cancelMove();
    return true;
  }
  return false;
}

void KSix::setBlack(PlayerType p)
{
  if(!_killingBlackPlayer && killBlack()) {
    _oldBlack = _black;
    _killingBlackPlayer = true;
  }
  DBG << "Setting black to " << playerToString(p) << std::endl;
  _blackPlayer = p;
  _black = createPlayer(p);
  _blackPlayerAction->setCurrentItem(p);
  if(!_match.null()) {
    (*_match).setVerticalPlayer(_black);
  }
}

void KSix::setWhite(PlayerType p)
{
  if(!_killingWhitePlayer && killWhite()) {
    _oldWhite = _white;
    _killingWhitePlayer = true;
  }
  DBG << "Setting white to " << playerToString(p) << std::endl;
  _whitePlayer = p;
  _white = createPlayer(p);
  _whitePlayerAction->setCurrentItem(p);
  if(!_match.null()) {
    (*_match).setHorizontalPlayer(_white);
  }
}

void KSix::setBlackPlayerByMenuItem(int i)
{
  if(_blackPlayer != (PlayerType)i)
    setBlack((PlayerType)i);
  turn();
}

void KSix::setWhitePlayerByMenuItem(int i)
{
  if(_whitePlayer != (PlayerType)i)
    setWhite((PlayerType)i);
  turn();
}

//
//
//

void KSix::setFilename(const QString &filename)
{
  _filename = filename;
  updateCaption();
}

bool KSix::killMatch()
{
  if(_thinking != HEX_MARK_EMPTY) {
    killBlack();
    killWhite();
    return true;
  }
  return false;
}

void KSix::stopMatch()
{
  if(!_match.null() && !_stoppingMatch) {
    // We're not interested in this old match anymore,
    disconnect(&*_match, 0, 0, 0);
    if(killMatch()) {
      // but it must be kept alive until control returns from it.
      _oldMatch = _match;
      _match = 0;
      _stoppingMatch = true;
    }
  }
}

void KSix::newMatch(const HexGame &g)
{
  if(!_stoppingMatch) {
    stopMatch();
    setBlack(_blackPlayer);
    setWhite(_whitePlayer);
    _match = new HexMatch(g, _black, _white);

    updateBoard();
    updateActions();
    updateClock();
    updateCaption();
    updateMessage();
    connect(&*_match, SIGNAL(signalPlayerChange()),
            this, SLOT(updateMessage()));
    connect(&*_match, SIGNAL(signalPlayerChange()),
            this, SLOT(updateThinking()));

    connect(&*_match, SIGNAL(signalPositionChange()),
            this, SLOT(updateBoard()));
    connect(&*_match, SIGNAL(signalPositionChange()),
            this, SLOT(updateActions()));
    connect(&*_match, SIGNAL(signalPositionChange()),
            this, SLOT(updateMessage()));

    connect(&*_match, SIGNAL(signalStatusChange()),
            this, SLOT(updateActions()));
    connect(&*_match, SIGNAL(signalStatusChange()),
            this, SLOT(updateMessage()));
    connect(&*_match, SIGNAL(signalStatusChange()),
            this, SLOT(updateClock()));

    connect(&*_match, SIGNAL(signalClockChange()),
            this, SLOT(updateClock()));

    connect(&*_match, SIGNAL(signalChangedGameStatus()),
            this, SLOT(updateCaption()));
  }
}

void KSix::newMatch()
{
  if(!_stoppingMatch) {
    setFilename("");
    HexGame g(HexBoard(_boardSize, _boardSize), HEX_MARK_VERT,
              _toggleSwapAction->isChecked());
    newMatch(g);
  }
}

bool KSix::isTransverted()
{
  if(!_match.null()) {
    const HexGame &g = (*_match).game();
    return ((_swapStyleAction->currentItem() == 1) &&
            g.initialState().swappable() && !g.swappable());
  } else {
    return false;
  }
}

HexMark KSix::transvertMark(HexMark m)
{
  if(isTransverted())
    return INVERT_HEX_MARK(m);
  else return m;
}

HexField KSix::transvertField(HexField f)
{
  if(isTransverted())
    return (*_match).game().board().transvert(f);
  else return f;
}

void KSix::flash()
{
  if((*_match).status() == HexMatch::MATCH_FINISHED) {
    pair<HexMark, vector<HexField> > w =
      (*_match).game().board().winningPath();
    // is it a normal win (not a resignation or forfeit)?
    if(w.first != HEX_MARK_EMPTY) {
      vector<int> wp;
      for(unsigned i = 0; i < w.second.size(); i++) {
        if(w.second[i] >= HexBoard::FIRST_NORMAL_FIELD) {
          wp.push_back(transvertField(w.second[i]) - 
                       HexBoard::FIRST_NORMAL_FIELD);
        }
      }
      _hexWidget->flashHexagons(wp, 200, 2);
    }
  } else {
    HexGame::ReverseIterator rbegin = (*_match).game().rbegin();
    HexGame::ReverseIterator rend = (*_match).game().rend();
    if(rbegin != rend) {
      vector<HexField> fl;
      const HexMove &m = (*(*_match).game().rbegin()).move();
      const HexBoard &b = (*_match).game().board();
      if(m.isNormal()) {
        fl.push_back(transvertField(m.field()) - HexBoard::FIRST_NORMAL_FIELD);
      } else if(m.isSwap()) {
        for(HexField f = HexBoard::FIRST_NORMAL_FIELD; f < b.size(); f++) {
          if(b.get(f) != HEX_MARK_EMPTY)
            fl.push_back(transvertField(f) - HexBoard::FIRST_NORMAL_FIELD);
        }
      }
      _hexWidget->flashHexagons(fl, 100, 2);
    }
  }
}

HexMark KSix::onTurn()
{
  return (*_match).game().next();
}

void KSix::doSlice()
{
  updateThinking();
  int n = 0;
  do {
    if(n > 0) {
      if(n == 1)
        _hexWidget->stopFlash();
      // the game is suspended, let's wait a 0.1s to reduce CPU load
      usleep(100000L);
    }
    kapp->processEvents();
    ++n;
  } while(!_match.null() && (*_match).status() == HexMatch::MATCH_OFF &&
          !_stoppingMatch && !_killingBlackPlayer && !_killingWhitePlayer);
}

#include "ksix.moc"
