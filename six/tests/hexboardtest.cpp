#undef NDEBUG

#include "misc.h"
#include "hexboard.h"

#include <cassert>
#include <sstream>
#include <string>
#include <algorithm>

using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::ostringstream;

class HexBoardTest
{
public:
  void testTopEdgeNeighbours()
  {
    HexBoard b(5, 7);
    HexField f = HexBoard::TOP_EDGE;
    HexBoard::Iterator cur = b.neighbourBegin(f);
    HexBoard::Iterator end = b.neighbourEnd();
    int i = 0;
    while(cur != end) {
      b.printField(cout, *cur);
      cout << endl;
      assert((*cur) == b.coords2Field(b.xs() - 1 - i, 0));
      ++cur;
      ++i;
    }
    cout << endl;
    assert(i == b.xs());
  }

  void testBottomEdgeNeighbours()
  {
    HexBoard b(5, 7);
    HexField f = HexBoard::BOTTOM_EDGE;
    HexBoard::Iterator cur = b.neighbourBegin(f);
    HexBoard::Iterator end = b.neighbourEnd();
    int i = 0;
    while(cur != end) {
      b.printField(cout, *cur);
      cout << endl;
      assert((*cur) == b.coords2Field(b.xs() - 1 - i, b.ys() - 1));
      ++cur;
      ++i;
    }
    cout << endl;
    assert(i == b.xs());
  }

  void testLeftEdgeNeighbours()
  {
    HexBoard b(5, 7);
    HexField f = HexBoard::LEFT_EDGE;
    HexBoard::Iterator cur = b.neighbourBegin(f);
    HexBoard::Iterator end = b.neighbourEnd();
    int i = 0;
    while(cur != end) {
      b.printField(cout, *cur);
      cout << endl;
      assert((*cur) == b.coords2Field(0, b.ys() - 1 - i));
      ++cur;
      ++i;
    }
    cout << endl;
    assert(i == b.ys());
  }

  void testRightEdgeNeighbours()
  {
    HexBoard b(5, 7);
    HexField f = HexBoard::RIGHT_EDGE;
    HexBoard::Iterator cur = b.neighbourBegin(f);
    HexBoard::Iterator end = b.neighbourEnd();
    int i = 0;
    while(cur != end) {
      b.printField(cout, *cur);
      cout << endl;
      assert((*cur) == b.coords2Field(b.xs() - 1, b.ys() - 1 - i));
      ++cur;
      ++i;
    }
    cout << endl;
    assert(i == b.ys());
  }

  void testFieldInTheMiddleNeighbours()
  {
    HexBoard b(5, 7);
    HexField f = b.coords2Field(2, 3);
    HexBoard::Iterator cur = b.neighbourBegin(f);
    HexBoard::Iterator end = b.neighbourEnd();
    int neighbours[6][2] = { {2, 2}, {3, 2}, {3, 3}, {2, 4}, {1, 4}, {1, 3} };
    int i = 0;
    while(cur != end) {
      b.printField(cout, *cur);
      cout << endl;
      assert((*cur) == b.coords2Field(neighbours[i][0], neighbours[i][1]));
      ++cur;
      ++i;
    }
    cout << endl;
    assert(i == 6);
  }

  void testFieldOnTopNeighbours()
  {
    HexBoard b(5, 7);
    HexField f = b.coords2Field(2, 0);
    HexBoard::Iterator cur = b.neighbourBegin(f);
    HexBoard::Iterator end = b.neighbourEnd();
    int neighbours[6][2] = { {2, -1}, {3, -1}, {3, 0},
                             {2, 1}, {1, 1}, {1, 0} };
    int i = 0;
    while(cur != end) {
      b.printField(cout, *cur);
      cout << endl;
      assert((*cur) == b.coords2Field(neighbours[i][0], neighbours[i][1]));
      ++cur;
      ++i;
    }
    cout << endl;
    assert(i == 6);
  }

  void testFieldOnTopLeftNeighbours()
  {
    HexBoard b(5, 7);
    HexField f = b.coords2Field(0, 0);
    HexBoard::Iterator cur = b.neighbourBegin(f);
    HexBoard::Iterator end = b.neighbourEnd();
    int neighbours[6][2] = { {-1, -1}, {0, -1}, {1, 0},
                             {0, 1}, {-1, 1}, {-1, 0} };
    int i = 0;
    while(cur != end) {
      b.printField(cout, *cur);
      cout << endl;
      assert((*cur) == b.coords2Field(neighbours[i][0], neighbours[i][1]));
      ++cur;
      ++i;
    }
    cout << endl;
    assert(i == 6);
  }

  void testWinner()
  {
    {
      HexBoard b(1, 1);
      assert(b.winner() == HEX_MARK_EMPTY);
      b.set(b.coords2Field(0, 0), HEX_MARK_VERT);
      assert(b.winner() == HEX_MARK_VERT);
      b.set(b.coords2Field(0, 0), HEX_MARK_HORI);
      assert(b.winner() == HEX_MARK_HORI);
    }

    {
      HexBoard b(3, 3);
      assert(b.winner() == HEX_MARK_EMPTY);
      b.set(b.coords2Field(0, 0), HEX_MARK_VERT);
      assert(b.winner() == HEX_MARK_EMPTY);
      b.set(b.coords2Field(2, 1), HEX_MARK_VERT);
      assert(b.winner() == HEX_MARK_EMPTY);
      b.set(b.coords2Field(0, 1), HEX_MARK_VERT);
      assert(b.winner() == HEX_MARK_EMPTY);
      b.set(b.coords2Field(1, 1), HEX_MARK_VERT);
      assert(b.winner() == HEX_MARK_EMPTY);
      b.set(b.coords2Field(2, 2), HEX_MARK_VERT);
      assert(b.winner() == HEX_MARK_VERT);
    }
  }

  void testWinningPath()
  {
    {
      HexBoard b(1, 1);
      b.set(b.coords2Field(0, 0), HEX_MARK_VERT);
      vector<HexField> expectedPath;
      expectedPath.push_back(HexBoard::TOP_EDGE);
      expectedPath.push_back(HexBoard::BOTTOM_EDGE);
      expectedPath.push_back(HexBoard::FIRST_NORMAL_FIELD);
      pair<bool, vector<HexField> > wp = b.winningPath();
      assert(wp.first);
      assert(wp.second == expectedPath);
    }
  }

  void testSaveLoad()
  {
    {
      HexBoard b(5, 7);
      b.set(b.coords2Field(0, 1), HEX_MARK_VERT);
      b.set(b.coords2Field(2, 3), HEX_MARK_HORI);
      ostringstream os;
      os << b;
      os << '\0';
      assert(!os.fail());

      HexBoard b1;
      istringstream is(os.str());
      is >> b1;
      assert(!is.fail());
      assert(b == b1);
    }

    {
      // Error in: A C C
      string s("A C C D E 1 . . . . . 1 2 V . . . . 2 3 . . . . . 3"
               " 4 . . H . . 4 5 . . . . . 5 6 . . . . . 6 7 . . . . . 7"
               " A B C D E");
      istringstream is(s);
      HexBoard b;
      is >> b;
      assert(is.fail());
    }

    {
      // only four cells in 2nd row
      string s("A B C D E 1 . . . . . 1 2 V . . . 2 3 . . . . . 3"
               " 4 . . H . . 4 5 . . . . . 5 6 . . . . . 6 7 . . . . . 7"
               " A B C D E");
      istringstream is(s);
      HexBoard b;
      is >> b;
      assert(is.fail());
    }

    {
      // 7 at the end of 2nd row
      string s("A B C D E 1 . . . . . 1 2 V . . . . 7 3 . . . . . 3"
               " 4 . . H . . 4 5 . . . . . 5 6 . . . . . 6 7 . . . . . 7"
               " A B C D E");
      istringstream is(s);
      HexBoard b;
      is >> b;
      assert(is.fail());
    }

    {
      // Bad mark in 4th row
      string s("A B C D E 1 . . . . . 1 2 V . . . . 2 3 . . . . . 3"
               " 4 . . I . . 4 5 . . . . . 5 6 . . . . . 6 7 . . . . . 7"
               " A B C D E");
      istringstream is(s);
      HexBoard b;
      is >> b;
      assert(is.fail());
    }

  }
};

int main()
{
  HexBoardTest t;
  t.testTopEdgeNeighbours();
  t.testBottomEdgeNeighbours();
  t.testLeftEdgeNeighbours();
  t.testRightEdgeNeighbours();
  t.testFieldInTheMiddleNeighbours();
  t.testFieldOnTopNeighbours();
  t.testFieldOnTopLeftNeighbours();

  t.testWinner();
  t.testWinningPath();

  t.testSaveLoad();
  return 0;
}
