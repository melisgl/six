// Yo Emacs, this -*- C++ -*-
#ifndef KSIX_H
#define KSIX_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include "poi.h"
#include "hexmatch.h"
#include "khexwidget.h"
#include "slicedtask.h"

#include <qlabel.h>
#include <qvalidator.h>

#include <kapp.h>
#include <kmainwindow.h>
#include <kaction.h>
#include <kprinter.h>

class ClickableLabel : public QLabel
{
  Q_OBJECT
public:
  ClickableLabel(QWidget *parent) : QLabel(parent) {}
private:
  void mouseReleaseEvent(QMouseEvent *) {
    emit released();
  }
signals:
  void released();
};

/**
 * This class serves as the main window for six.  It handles the
 * menus, toolbars, status bars and acts as a classic bloated
 * controller.
 *
 * To avoid protability problems and dependency on qt compiled with thread
 * support, Connector is given the responsibility of invoking a SlicedTask
 * every now and then. This SlicedTask is a KSix object that does some
 * event processing and returns.
 */
class KSix : public KMainWindow, public SlicedTask
{
  Q_OBJECT
public:
  KSix();
  virtual ~KSix();
  void doSlice();
protected:
  /**
   * This function is called when it is time for the app to save its
   * properties for session management purposes.
   */
  void saveProperties(KConfig *);

  /**
   * This function is called when this app is restored.  The KConfig
   * object points to the session management config file that was saved
   * with @ref saveProperties
   */
  void readProperties(KConfig *);

  void saveStandardProperties(KConfig *);
  void readStandardProperties(KConfig *);
  void saveGameProperties(KConfig *);
  void readGameProperties(KConfig *);
private slots:
  bool queryClose();

  void fileOpen();
  void fileSave();
  void fileSaveAs();
  void fileImportPBEMGame();
  void filePrint();

  void showToolbar();
  void showStatusbar();
  void configureKeys();
  void configureToolbars();

  void updateBoard();
  void updateActions();
  void updateClock();
  void updateCaption();
  void updateMessage();
  void updateThinking();

  void gameNew();

  void gamePreviousMove();
  void gameNextMove();
  void gameFirstMove();
  void gameLastMove();

  void gameSuspend();
  void gameResume();

  void makeMove(const HexMove &);
  void gameMove();
  void gameSwap();
  void gameResign();

  void toggleSwap();
  void setBoardSize(int);
  void setBoardSizeByMenuItem(int item);
  void toggleResign();

  void toggleNumbering();
  void toggleShowThinking();
  void setSwapStyleByMenuItem(int);
  void setDisplayModeByMenuItem(int);

  void setBlackPlayerByMenuItem(int);
  void setWhitePlayerByMenuItem(int);

  void saveOptions();
  void readOptions();

  void clickLeft(HexField f);

  void flash();
public:
  bool importPBEMGame(istream &is);
  bool importPBEMGame(const QString &filename);
  bool importPBEMGame(istream &is, const QString &filename);

  enum PlayerTypeT { PLAYER_NONE = -1, PLAYER_HUMAN,
                     PLAYER_BEGINNER, PLAYER_INTERMEDIATE,
                     PLAYER_ADVANCED, PLAYER_EXPERT};
  typedef enum PlayerTypeT PlayerType;
  static QString playerToString(PlayerType p);
  static PlayerType stringToPlayer(const QString &s);
  void setBlack(PlayerType p);
  void setWhite(PlayerType p);
  bool loadGame(const QString &filename);
private:
  void setupActions();
  static QString secToQString(int sec);
  QString moveToString(const HexMove &m, int n, const HexBoard &b);

  HexPlayer *createPlayer(PlayerType p);
  bool killBlack();
  bool killWhite();
  HexMark onTurn();

  void setFilename(const QString &filename);
  bool verifyUnsavedGame(const QString &question);
  bool saveGame(const QString &filename);

  bool killMatch();
  void stopMatch();
  void newMatch(const HexGame &g);
  void newMatch();

  bool isTransverted();
  HexMark transvertMark(HexMark);
  HexField transvertField(HexField);

  void turn();
private:
  QString _filename;

  Poi<HexMatch> _oldMatch;
  Poi<HexPlayer> _oldBlack;
  Poi<HexPlayer> _oldWhite;
  bool _stoppingMatch;
  bool _killingBlackPlayer;
  bool _killingWhitePlayer;
  HexMark _thinking;
  HexMove _candidateMove;

  int _boardSize;
  PlayerType _blackPlayer, _whitePlayer;
  Poi<HexPlayer> _black;
  Poi<HexPlayer> _white;
  Poi<HexMatch> _match;
  KHexWidget *_hexWidget;
  
  KPrinter *_printer;

  // notable widgets on the statusbar
  ClickableLabel *_messageLabel;
  QLabel *_blackClockMark;
  QLabel *_whiteClockMark;
  QLabel *_blackClockLabel;
  QLabel *_whiteClockLabel;

  //
  KToggleAction *_toolbarAction;
  KToggleAction *_statusbarAction;

  // file
  KAction *_gameNewAction;

  // game
  KAction *_gamePreviousMoveAction;
  KAction *_gameNextMoveAction;
  KAction *_gameFirstMoveAction;
  KAction *_gameLastMoveAction;
  KAction *_gameSuspendAction;
  KAction *_gameResumeAction;
  KAction *_gameMoveAction;
  KAction *_gameSwapAction;
  KAction *_gameResignAction;

  // game options
  KToggleAction *_toggleSwapAction;
  KSelectAction *_boardSizeAction;
  KToggleAction *_toggleResignAction;
  
  // display options
  KToggleAction *_toggleNumberingAction;
  KToggleAction *_showThinkingAction;
  KSelectAction *_swapStyleAction;
  KSelectAction *_displayModeAction;

  // players
  KSelectAction *_blackPlayerAction;
  KSelectAction *_whitePlayerAction;
};

#endif
