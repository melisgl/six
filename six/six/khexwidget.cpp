#include "khexwidget.h"

#include "misc.h"

#include <iostream>
#include <cassert>

#include <qpainter.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qpointarray.h>

#include <klocale.h>

static QColor grey = QColor(184, 184, 184);
static QColor lightGrey = QColor("lightgrey");
static QColor darkGrey = QColor(128, 128, 128);
static QPen blackPen = QPen(QColor("black"));
static QPen whitePen = QPen(QColor("white"));
static QPen lightGreyPen = QPen(lightGrey);
static QPen darkGreyPen = QPen(darkGrey);
static QBrush blackBrush = QBrush(QColor("black"));
static QBrush whiteBrush = QBrush(QColor("white"));
static QBrush greyBrush = QBrush(grey);
static QBrush lightGreyBrush = QBrush(lightGrey);
static QBrush darkGreyBrush = QBrush(darkGrey);

KHexWidget::KHexWidget(QWidget *parent) : QWidget(parent)
{
  _bufferOld = true;
  _numbering = false;
  _displayMode = DISPLAY_DIAGONAL;
  setBackgroundMode(NoBackground);
  connect(&_timer, SIGNAL(timeout()), this, SLOT(flash()));
}

KHexWidget::~KHexWidget()
{
}

QSizePolicy KHexWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void KHexWidget::flashHexagons(const vector<HexField> &hexagons,
                               int interval, int nFlashes)
{
  stopFlash();
  _flashingHexagons = hexagons;
  _flashCounter = nFlashes * 2;
  flash();
  _timer.start(interval);
}

void KHexWidget::drawCenteredText(QPainter *p, int x, int y, const QString &s)
{
  QRect re = p->fontMetrics().boundingRect(s);
  int w = p->fontMetrics().width(s);
  int h = re.height();
  p->drawText(x - w / 2 , y + h / 2 , s);
}

void KHexWidget::setUpEdge(QPointArray &edge, int n,
                           int x0, int y0, int x1, int y1, int x2, int y2,
                           int dx, int dy)
{
  int qx = x1 / 3;
  int qy = y1 / 3;
  for(int i = 0; i < n; i++) {
    edge.setPoint(edge.size() / 2 - 2 * i - 1,
                  x0 + i * dx, y0 + i * dy);
    if(i == 0) {
      edge.setPoint(edge.size() / 2 + 2 * i,
                    x0 / 3 + x0 + i * dx, y0 / 3 + y0 + i * dy);
    } else {
      edge.setPoint(edge.size() / 2 + 2 * i,
                    qx + x0 + i * dx, qy + y0 + i * dy);
    }
    edge.setPoint(edge.size() / 2 - 2 * i - 2,
                  x1 + i * dx, y1 + i * dy);
    edge.setPoint(edge.size() / 2 + 2 * i + 1,
                  qx + x1 + i * dx, qy + y1 + i * dy);
  }
  edge.setPoint(0,
                x2 + (n - 1) * dx,  y2 + (n - 1) * dy);
  edge.setPoint(edge.size() - 1,
                x2 / 3 + x2 + (n - 1) * dx,  y2 / 3 + y2 + (n - 1) * dy);
}

void KHexWidget::drawCoords(QPainter *p, const QColor &fg,
                            int bxs, int bys,
                            int hxs, int,
                            int colDx, int colDy, int rowDx, int rowDy)
{
  p->save();
  QFont font("times", hxs / 2, QFont::Bold);
  p->setFont(font);
  p->setPen(fg);
  for(int y = 0; y < bys; y++) {
    drawCenteredText(p, -colDx + y * rowDx, -colDy + y * rowDy,
                     QString::number(y + 1));
    drawCenteredText(p, bxs * colDx + y * rowDx, bxs * colDy + y * rowDy,
                     QString::number(y + 1));
  }
  for(int x = 0; x < bxs; x++) {
    drawCenteredText(p, x * colDx - rowDx, x * colDy - rowDy,
                     QString(QChar('A' + x)));
    drawCenteredText(p, x * colDx + bys * rowDx, x * colDy + bys * rowDy,
                     QString(QChar('A' + x)));
  }
  p->restore();
}

void KHexWidget::drawBoard(QPainter *p, const QColor &,
                           const HexBoard &b,
                           int hxs, int hys,
                           int colDx, int colDy, int rowDx, int rowDy,
                           bool storeHexagons)
{
  p->save();
  int bxs = b.xs();
  int bys = b.ys();
  int hms = MAX(hxs, hys);
  QRect prect(-hms * 5 / 8, -hms * 5 / 8, hms * 10 / 8, hms * 10 / 8);
  QRect smallPrect(-hms * 5 / 14, -hms * 5 / 14, hms * 10 / 14, hms * 10 / 14);
  QRect smallerPrect(-hms * 2 / 14, -hms * 2 / 14, hms * 4 / 14, hms * 4 / 14);
  QFont font("times", hxs / 2, QFont::Bold);
  QFontMetrics fm(font);
  QPointArray pa(6);
  if(_displayMode == DISPLAY_DIAGONAL) {
    pa.setPoints(6, hxs, 0, hxs / 2, hys, -hxs / 2, hys, -hxs, 0,
                 -hxs / 2, -hys, hxs / 2, -hys);
  } else {
    pa.setPoints(6, 0, hys, -hxs, hys / 2, -hxs, -hys / 2,
                 0, -hys, hxs, -hys / 2, hxs, hys / 2);
  }
  QPointArray horiLine;
  QPointArray vertLine;
  vertLine.setPoints(2, -hxs / 2, -hys, hxs / 2, hys);
  horiLine.setPoints(2, hxs / 2, -hys, -hxs / 2, hys);
  p->setFont(font);
  p->setPen(blackPen);
  p->setBrush(greyBrush);
  for(int y = 0; y < bys; y++) {
    p->save();
    for(int x = 0; x < bxs; x++) {
      p->drawPolygon(pa);
      HexField f = b.coords2Field(x, y);
      p->setPen(blackPen);
      p->setBrush(greyBrush);
      p->drawPolygon(pa);
      if(b.get(f) == HEX_MARK_VERT) {
        p->save();
        if(_deadVert.get(f) != HEX_MARK_EMPTY) {
          p->setPen(blackPen);
          p->setBrush(darkGreyBrush);
        } else {
          p->setPen(blackPen);
          p->setBrush(blackBrush);
        }
        p->drawEllipse(prect);
        p->restore();
      } else if(b.get(f) == HEX_MARK_HORI) {
        p->save();
        if(_deadHori.get(f) != HEX_MARK_EMPTY) {
          p->setPen(whitePen);
          p->setBrush(lightGreyBrush);
        } else {
          p->setPen(whitePen);
          p->setBrush(whiteBrush);
        }
        p->drawEllipse(prect);
        p->restore();
      } else {
        if((_deadVert.get(f) != HEX_MARK_EMPTY) &&
           (_deadHori.get(f) != HEX_MARK_EMPTY)) {
          p->setPen(blackPen);
          p->setBrush(greyBrush);
          p->drawEllipse(smallPrect);
        } else if(_deadVert.get(f) != HEX_MARK_EMPTY) {
          p->setPen(whitePen);
          p->setBrush(whiteBrush);
          p->drawEllipse(smallPrect);
        } else if(_deadHori.get(f) != HEX_MARK_EMPTY) {
          p->setPen(blackPen);
          p->setBrush(blackBrush);
          p->drawEllipse(smallPrect);
        } else if((_uselessVert.get(f) != HEX_MARK_EMPTY) &&
           (_uselessHori.get(f) != HEX_MARK_EMPTY)) {
          p->setPen(blackPen);
          p->setBrush(greyBrush);
          p->drawEllipse(smallerPrect);
        } else if(_uselessVert.get(f) != HEX_MARK_EMPTY) {
          p->setPen(whitePen);
          p->setBrush(whiteBrush);
          p->drawEllipse(smallerPrect);
        } else if(_uselessHori.get(f) != HEX_MARK_EMPTY) {
          p->setPen(blackPen);
          p->setBrush(blackBrush);
          p->drawEllipse(smallerPrect);
        }
      }
      if(storeHexagons) {
        _hexagonPolygons[y * bxs + x] = p->worldMatrix().map(pa);
        _hexagonRegions[y * bxs + x] = QRegion(_hexagonPolygons[y * bxs + x]);
      }
      p->translate(colDx, colDy);
    }
    p->restore();
    p->translate(rowDx, rowDy);
  }
  p->restore();
}

void KHexWidget::drawNumbering(QPainter *p, const HexGame &g,
                               int hxs, int hys,
                               int colDx, int colDy, int rowDx, int rowDy)
{
  p->save();
  p->setFont(QFont("helvetica", 2 * MAX(hxs, hys) / 5));
  int n = g.initialState().board().nMark() + 1;
  HexGame::Iterator cur = g.begin();
  HexGame::Iterator next;
  HexGame::Iterator end = g.end();
  while(cur != end) {
    next = cur;
    ++next;
    const HexMove &m = (*cur).move();
    if(m.isNormal()) {
      int x, y;
      g.board().field2Coords(m.field(), &x, &y);
      if(next != end && (*next).move().isSwap()) {
        if(m.mark() == HEX_MARK_VERT)
          p->setPen(blackPen);
        else
          p->setPen(whitePen);
//         {
//           QRect smallPrect(-hms * 5 / 14, -hms * 5 / 14, hms * 10 / 14, hms * 10 / 14);
//           drawEllipse(smallPrect);
//         }
        drawCenteredText(p, y * colDx + x * rowDx, y * colDy + x * rowDy,
                         i18n("S"));
      } else {
        if(m.mark() == HEX_MARK_VERT)
          p->setPen(whitePen);
        else
          p->setPen(blackPen);
        drawCenteredText(p, x * colDx + y * rowDx, x * colDy + y * rowDy,
                         QString::number(n));
      }
    }
    n++;
    cur = next;
  }
  p->restore();
}

void KHexWidget::drawEdges(QPainter *p, const QColor &fg,
                           const QPointArray &e1, const QPointArray &e2)
{
  p->save();
  p->setPen(fg);
  if(_game.winner() == HEX_MARK_HORI)
    p->setBrush(darkGreyBrush);
  else
    p->setBrush(blackBrush);
  p->drawPolygon(e1);
  p->scale(-1., -1.);
  p->drawPolygon(e1);
  p->restore();

  p->save();
  p->setPen(fg);
  if(_game.winner() == HEX_MARK_VERT)
    p->setBrush(lightGreyBrush);
  else
    p->setBrush(whiteBrush);
  p->drawPolygon(e2);
  p->scale(-1., -1.);
  p->drawPolygon(e2);
  p->restore();
}

void KHexWidget::drawDiagonalEdges(QPainter *p, const QColor &fg,
                                   int bxs, int bys,
                                   int hxs, int hys,
                                   int colDx, int colDy, int rowDx, int rowDy,
                                   int xoff, int yoff)
{
  QPointArray topEdge(bxs * 4 + 2);
  QPointArray leftEdge(bys * 4 + 2);
  setUpEdge(topEdge, bxs,
            -hxs, 0, -hxs / 2, -hys, 0, -hys,
            colDx, colDy);
  setUpEdge(leftEdge, bys,
            -hxs, 0, -hxs / 2, hys, 0, hys,
            rowDx, rowDy);
  topEdge.translate(xoff, yoff);
  leftEdge.translate(xoff, yoff);

  drawEdges(p, fg, topEdge, leftEdge);
}

void KHexWidget::drawFlatEdges(QPainter *p, const QColor &fg,
                               int bxs, int bys,
                               int hxs, int hys,
                               int colDx, int colDy, int rowDx, int rowDy,
                               int xoff, int yoff)
{
  QPointArray topEdge(bxs * 4 + 2);
  QPointArray leftEdge(bys * 4 + 2);
  setUpEdge(topEdge, bxs,
            -hxs, -hys / 2, 0, -hys, hxs / 2, -hys * 3 / 4,
            colDx, colDy);
  setUpEdge(leftEdge, bys,
            -hxs, -hys / 2, -hxs, hys / 2, -hxs / 2, hys * 3 / 4,
            rowDx, rowDy);
  topEdge.translate(xoff, yoff);
  leftEdge.translate(xoff, yoff);

  drawEdges(p, fg, topEdge, leftEdge);
}

void KHexWidget::print(QPainter *p, const QColor &fg,
                       int width, int height, bool storeHexagons)
{
  int bxs = _game.board().xs();
  int bys = _game.board().ys();
  if(storeHexagons) {
    _hexagonRegions.resize(bys * bxs);
    _hexagonPolygons.resize(bys * bxs);
  }

  double zoom;
  {
    int boardXs, boardYs;
    if(_displayMode == DISPLAY_DIAGONAL) {
      boardXs = 2 * 100 + (bxs - 1 + bys - 1) * 150;
      boardYs = (bxs + bys) * 86;
    } else {
      boardXs = (2 * bxs + bys - 1) * 86;
      boardYs = 2 * 100 + (bys - 1) * 150;
    }
    double xa = ((double)boardXs + 300) / ((double)width);
    double ya = ((double)boardYs + 300) / ((double)height);
    zoom = MAX(xa, ya);
  }

  int hxs, hys;
  int colDx, colDy;
  int rowDx, rowDy;
  if(_displayMode == DISPLAY_DIAGONAL) {
    hxs = (int)(100. / zoom);
    hys = (int)(86. / zoom);
    colDx = hxs * 3 / 2;
    colDy = -hys;
    rowDx = hxs * 3 / 2;
    rowDy = hys;
    p->translate(width / 2, height / 2);
    int boardXsa = 2 * hxs + (bxs - 1 + bys - 1) * (hxs * 3 / 2);
    drawDiagonalEdges(p, fg,
                      bxs, bys, hxs, hys,
                      colDx, colDy, rowDx, rowDy,
                      -boardXsa / 2 + hxs, (bxs - bys) * hys / 2);
    p->translate(-boardXsa / 2 + hxs, (bxs - bys) * hys / 2);
  } else {
    assert(_displayMode == DISPLAY_FLAT);
    hxs = (int)(86. / zoom);
    hys = (int)(100. / zoom);
    colDx = 2 * hxs;
    colDy = 0;
    rowDx = hxs;
    rowDy = hys * 3 / 2;
    int boardXsa = (2 * bxs + bys - 1) * hxs;
    int boardYsa = 2 * hys + (bys - 1) * (hys * 3 / 2);
    p->translate(width / 2, height / 2);
    drawFlatEdges(p, fg,
                  bxs, bys, hxs, hys,
                  colDx, colDy, rowDx, rowDy,
                  -boardXsa / 2 + hxs, -boardYsa / 2 + hys);
    p->translate(-boardXsa / 2 + hxs, -boardYsa / 2 + hys);
  }

  drawCoords(p, fg, _game.board().xs(), _game.board().ys(),
             hxs, hys, colDx, colDy, rowDx, rowDy);
  drawBoard(p, fg, _game.board(), hxs, hys, colDx, colDy, rowDx, rowDy,
            storeHexagons);
  if(_numbering)
    drawNumbering(p, _game, hxs, hys, colDx, colDy, rowDx, rowDy);
}

void KHexWidget::paintEvent(QPaintEvent *)
{
  QRect rect = (*this).rect();
  if(_buffer.size() != rect.size() || _bufferOld ||
     _lastBackground != parentWidget()->backgroundColor() ||
     _lastForeground != foregroundColor()) {
    _lastBackground = parentWidget()->backgroundColor();
    _lastForeground = foregroundColor();
    _buffer.resize(rect.size());
    // probably because NoBackground is set, we need to fetch the bg color
    // from parent
    _buffer.fill(parentWidget()->backgroundColor());
    QPainter p(&_buffer);
    print(&p, _lastForeground, _buffer.width(), _buffer.height(), true);
    p.end();
    _bufferOld = false;
  }
  bitBlt(this, 0, 0, &_buffer);
}

void KHexWidget::mouseReleaseEvent(QMouseEvent *e)
{
  int xs = _game.board().xs();
  for(unsigned i = 0; i < _hexagonRegions.size(); i++) {
    if(_hexagonRegions[i].contains(e->pos())) {
      if(e->button() == LeftButton) {
        emit signalClickLeft(_game.board().coords2Field(i % xs, i / xs));
      }
      break;
    }
  }
}

void KHexWidget::setGame(const HexGame &g,
                         const HexBoard &deadVert,
                         const HexBoard &deadHori,
                         const HexBoard &uselessVert,
                         const HexBoard &uselessHori)
{
  _game = g;
  _deadVert = deadVert;
  _deadHori = deadHori;
  _uselessVert = uselessVert;
  _uselessHori = uselessHori;
  _bufferOld = true;
  stopFlash();
  repaint();
  emit signalChangeGame(_game);
}

void KHexWidget::setDisplayMode(DisplayMode dm)
{
  if(_displayMode != dm) {
    _displayMode = dm;
    _bufferOld = true;
    repaint();
  }
}

void KHexWidget::setNumbering(bool on)
{
  if(_numbering != on) {
    _numbering = on;
    _bufferOld = true;
    repaint();
  }
}

void KHexWidget::flash()
{
  _flashCounter--;
  if(_flashCounter <= 0 || ((_flashCounter % 2) == 0)) {
    if(!_bufferOld)
      bitBlt(this, 0, 0, &_buffer);
    if(_flashCounter <= 0) {
      _timer.stop();
    }
  } else {
    QPainter p(this);
    p.setPen(foregroundColor());
    p.setBrush(lightGreyBrush);
    for(unsigned i = 0; i < _flashingHexagons.size(); i++) {
      // The board might not be drawn yet, in which case
      // _hexagonPolygons are not there.
      if(_flashingHexagons[i] < (signed)_hexagonPolygons.size()) {
        p.drawPolygon(_hexagonPolygons[_flashingHexagons[i]]);
      }
    }
  }
}

bool KHexWidget::isFlashing() const
{
  return _timer.isActive();
}

void KHexWidget::stopFlash()
{
  if(_timer.isActive()) {
    _timer.stop();
    if(!_bufferOld)
      bitBlt(this, 0, 0, &_buffer);
  }
}

#include "khexwidget.moc"
