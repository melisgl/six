// Yo Emacs, this -*- C++ -*-
#ifndef KHEXWIDGET_H
#define KHEXWIDGET_H

#include "hexgame.h"

#include <vector>

#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>


/**
 * This widget displays a hex board with numbering, flashing marks,
 * and all the bells and whistles.
 */
class KHexWidget : public QWidget
{
  Q_OBJECT
public:
  enum DisplayModeT { DISPLAY_DIAGONAL, DISPLAY_FLAT };
  typedef DisplayModeT DisplayMode;
  KHexWidget(QWidget *parent);
  virtual ~KHexWidget();
  QSizePolicy sizePolicy() const;
  /**
   * Print this view to any medium -- paper or not
   */
  void print(QPainter *, const QColor &fg,
             int width, int height, bool storeHexagons = false);
  void flashHexagons(const vector<HexField> &hexagons,
                     int interval, int nFlashes);
  bool isFlashing() const;
  void stopFlash();
public slots:
  void setGame(const HexGame &,
               const HexBoard &deadVert,
               const HexBoard &deadHori,
               const HexBoard &uselessVert,
               const HexBoard &uselessHori);
  void setDisplayMode(DisplayMode);
  void setNumbering(bool);
  void flash();
signals:
  void signalChangeGame(const HexGame &);
  void signalClickLeft(HexField f);
protected:
  void paintEvent(QPaintEvent *);
  void mouseReleaseEvent(QMouseEvent *);
private:
  static void drawCenteredText(QPainter *p, int x, int y, const QString &s);
  static void setUpEdge(QPointArray &edge, int n,
                        int x0, int y0, int x1, int y1, int x2, int y2,
                        int dx, int dy);
  void drawNumbering(QPainter *p, const HexGame &g,
                     int hxs, int hys,
                     int colDx, int colDy, int rowDx, int rowDy);
  void drawEdges(QPainter *p, const QColor &fg,
                 const QPointArray &e1, const QPointArray &e2);
  void drawDiagonalEdges(QPainter *p, const QColor &fg,
                         int bxs, int bys,
                         int hxs, int hys,
                         int colDx, int colDy, int rowDx, int rowDy,
                         int xoff, int yoff);
  void drawFlatEdges(QPainter *p, const QColor &fg,
                     int bxs, int bys,
                     int hxs, int hys,
                     int colDx, int colDy, int rowDx, int rowDy,
                     int xoff, int yoff);
  void drawCoords(QPainter *p, const QColor &fg,
                  int bxs, int bys,
                  int hxs, int hys,
                  int colDx, int colDy, int rowDx, int rowDy);
  void drawBoard(QPainter *p, const QColor &fg,
                 const HexBoard &b,
                 int hxs, int hys,
                 int colDx, int colDy, int rowDx, int rowDy,
                 bool storeHexagons);

  HexGame _game;
  HexBoard _deadVert;
  HexBoard _deadHori;
  HexBoard _uselessVert;
  HexBoard _uselessHori;
  DisplayMode _displayMode;
  bool _numbering;

  bool _bufferOld;
  QPixmap _buffer;

  vector<QRegion> _hexagonRegions;
  vector<QPointArray> _hexagonPolygons;

  QTimer _timer;
  int _flashCounter;
  vector<HexField> _flashingHexagons;

  QColor _lastBackground;
  QColor _lastForeground;
};

#endif
