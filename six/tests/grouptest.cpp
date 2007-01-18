#undef NDEBUG

#include "group.h"
#include "hexmove.h"

#include <cassert>
#include <algorithm>

using std::cout;
using std::endl;

#define N_OFFSETS 6
static const int neighbourOffsets[N_OFFSETS][2] = {
  {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, 1}, {1, -1} };

class GroupTest
{
public:

  static bool equivalent(const Grouping &gi0, const Grouping &gi1)
  {
    // not quite enough:
    return gi0.size() == gi1.size();
  }

  static void checkEdge(const Group &g)
  {
    for(unsigned int i = 0; i < g.fields().size(); i++) {
      if(g.fields()[i] < HexBoard::FIRST_NORMAL_FIELD) {
        assert(g.edge());
        return;
      }
    }
    assert(!g.edge());
  }

  static void checkEdge(const Grouping &gi)
  {
    int i;
    for(i = 0; i < gi.size(); i++) {
      checkEdge(*gi[i]);
    }
  }
  
  static void checkMove(const Grouping &gi)
  {
    Grouping gi2;
    gi2.init(gi._board, gi._mark);
    if(!equivalent(gi, gi2)) {
      cout << "GI2" << endl;
      cout << gi2.board() << gi.size() << " " << gi2.size() << endl;
      print(gi2);
      cout << "END GI2" << endl;
    }
    assert(equivalent(gi, gi2));
  }

  // check if neighbouring cells belong to the same group
  //  *iff* they have the same mark that's not empty
  static void checkNeighbours(const Grouping &gi)
  {
    const HexBoard &b = gi.board();
    for(HexField f = 0; f < b.size(); f++) {
      Poi<Group> g0 = gi(f);
      HexBoard::Iterator neighbour = b.neighbourBegin(f);
      while(neighbour != b.neighbourEnd()) {
        Poi<Group> g1 = gi(*neighbour);
        if(!g0.null() && !g1.null()) {
          HexMark own = gi._board.get(f);
          HexMark n = gi._board.get(*neighbour);
          assert((g0 == g1) == (own == gi.mark() &&
                                own == n &&
                                own == (*g0).mark() &&
                                own == (*g1).mark()));
        }
        ++neighbour;
      }
    }
  }

  // check that cells with the opponents mark
  // don't belong to any group
  static void checkOpp(const Grouping &gi)
  {
    const HexBoard &b = gi.board();
    for(HexField f = 0; f < b.size(); f++) {
      Poi<Group> g0 = gi(f);
      assert(!(gi._board.get(f) != gi._mark &&
               gi._board.get(f) != HEX_MARK_EMPTY) || g0.null());
    }
  }

  static void checkEmpty(const Grouping &gi)
  {
    int i;
    for(i = 0; i < gi.size(); i++) {
      const Group &g = *gi[i];
      assert(!g.fields().empty());
    }
  }

  static void checkRmap1(const Grouping &gi)
  {
    int i;
    for(i = 0; i < gi.size(); i++) {
      const Poi<Group> g = gi[i];
      for(unsigned int j = 0; j < (*g).fields().size(); j++) {
        Poi<Group> g2 = gi((*g).fields()[j]);
        int idx = gi.groupIndex(g2);
        assert(idx >= 0 && idx < gi.size());
        assert(g == g2);
        assert(gi[idx] == g);
      }
    }
  }

  static void checkRmap2(const Grouping &gi)
  {
    const HexBoard &b = gi.board();
    for(HexField f = 0; f < b.size(); f++) {
      const Poi<Group> g0 = gi(f);
      if(!g0.null()) {
        const vector<int> &fields = (*g0).fields();
        assert(find(fields.begin(), fields.end(), f) != fields.end());
      }
    }
  }

  static void checkIndex(const Grouping &gi)
  {
    int i;
    for(i = 0; i < gi.size(); i++) {
      int idx = gi.groupIndex(gi[i]);
      assert(idx == i);
    }
  }

  static void checkEmptyFields(const Grouping &gi)
  {
    Carrier c;
    for(HexField i = 0; i < gi.board().size(); ++i) {
      if(gi.board().get(i) == HEX_MARK_EMPTY) {
        c.addField(i);
      }
    }
    assert(c.includes(gi.emptyFields()));
  }

  static void check(const Grouping &gi)
  {
    checkEmpty(gi);
    checkEdge(gi);
    checkOpp(gi);
    checkNeighbours(gi);
    checkRmap1(gi);
    checkRmap2(gi);
    checkIndex(gi);
    checkMove(gi);
    checkEmptyFields(gi);
  }

  static void print(const Grouping::Change &d)
  {
    if(!d.newGroup.null()) {
      cout << "New group: " << *d.newGroup << endl;
    }
    if(!d.emptyGroup.null()) {
      cout << "Empty group: " << *d.emptyGroup << endl;
    }
    cout << "United groups: ";
    for(unsigned i = 0; i < d.unitedGroups.size(); i++) {
      if(i) {
        cout << ",";
      }
      cout << *d.unitedGroups[i];
    }
    cout << endl;
    cout << "Deleted groups: ";
    for(unsigned i = 0; i < d.deletedGroups.size(); i++) {
      if(i) {
        cout << ",";
      }
      cout << *d.deletedGroups[i];
    }
    cout << endl;
  }

  static void print(const Grouping &gi)
  {
    int i;
    for(i = 0; i < gi.size(); i++) {
      const Group &g = *gi[i];
      cout << g;
      cout << endl;
    }
  }

  void testGame(HexBoard b, HexMark mark, const vector<HexMove> &m)
  {
    cout << "Test starts ..." << endl;
    cout << "Initial board position: " << endl << b;
    Grouping gi;
    gi.init(b, mark);
    print(gi);
    check(gi);
    for(unsigned int i = 0; i < m.size(); i++) {
      cout << "Playing: " << m[i].mark();
      b.printField(cout, m[i].field());
      cout << endl;
      print(gi.move(m[i].field(), m[i].mark()));
      cout << gi._board;
      print(gi);
      check(gi);
    }
    cout << "Test finished" << endl;
  }

  void testGame(HexBoard b, const vector<HexMove> &m)
  {
    testGame(b, HEX_MARK_VERT, m);
    testGame(b, HEX_MARK_HORI, m);
  }

  void testMove()
  {
    {
      HexBoard b(1, 1);
      vector<HexMove> m;
      testGame(b, m);
    }

    {
      HexBoard b(1, 1);
      b.set(b.coords2Field(0, 0), HEX_MARK_HORI);
      vector<HexMove> m;
      testGame(b, m);
    }

    {
      HexBoard b(1, 1);
      b.set(b.coords2Field(0, 0), HEX_MARK_VERT);
      vector<HexMove> m;
      testGame(b, m);
    }

    {
      HexBoard b(1, 1);
      vector<HexMove> m;
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(0, 0)));
      testGame(b, m);
    }

    {
      HexBoard b(1, 1);
      vector<HexMove> m;
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(0, 0)));
      testGame(b, m);
    }

    {
      HexBoard b(2, 2);
      vector<HexMove> m;
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(0, 0)));
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(1, 0)));
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(0, 1)));
      testGame(b, m);
    }

    {
      HexBoard b(3, 3);
      vector<HexMove> m;
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(2, 0)));
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(0, 0)));
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(2, 1)));
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(1, 0)));
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(0, 1)));
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(1, 2)));
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(0, 2)));
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(2, 2)));
      testGame(b, m);
    }

    {
      HexBoard b(3, 3);
      vector<HexMove> m;
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(0, 0)));
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(0, 1)));
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(0, 2)));
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(1, 0)));
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(2, 0)));
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(2, 1)));
      m.push_back(HexMove(HEX_MARK_VERT, b.coords2Field(2, 2)));
      m.push_back(HexMove(HEX_MARK_HORI, b.coords2Field(1, 2)));
      testGame(b, m);
    }

    {
      HexBoard b(11, 11);
      vector<HexMove> m;
      testGame(b, m);
    }

    {
      HexBoard b(11, 11);
      b.set(b.coords2Field(0, 0), HEX_MARK_VERT);
      vector<HexMove> m;
      testGame(b, m);
    }
  }

  void testSet()
  {
    {
      HexBoard b(3, 3);
      b.set(b.coords2Field(0, 0), HEX_MARK_VERT);
      Grouping g0, g1;
      g0.init(b, HEX_MARK_VERT);
      g1.init(b, HEX_MARK_VERT);
      g1.init(b, HEX_MARK_VERT);
      assert(equivalent(g0, g1));
    }
  }
};

int main()
{
  {
    GroupTest gt;
    gt.testMove();
    gt.testSet();
  }
  return 0;
}
