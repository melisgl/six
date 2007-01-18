#include "circuit.h"
#include "misc.h"

#include "testutil.h"

#include <math.h>

#include <algorithm>

using std::cout;
using std::sort;
using std::endl;

bool operator <(const pair<HexMove, pair<double, double> > &m1,
                const pair<HexMove, pair<double, double> > &m2)
{
  // m1 < m2 iff m1.second > m2.second
  return m1.second.first + m1.second.second >
    m2.second.first + m2.second.second;
}

class CircuitTest
{
public:

  void showCond(const HexBoard &b)
  {
    cout << b;
    Connector vert;
    Connector hori;
    vert.init(b, HEX_MARK_VERT);
    hori.init(b, HEX_MARK_HORI);
    Circuit vertCond(vert);
    Circuit horiCond(hori);
    cout << "Vert resistance: " << vertCond.resistance() << endl;
    cout << "Hori resistance: " << horiCond.resistance() << endl;
    cout << "Vb=" << log(horiCond.resistance() / vertCond.resistance())
         << endl;

    vector<pair<HexMove, pair<double, double> > > moves;
    {
      const Grouping &bg = vert.grouping();
      const Grouping &wg = hori.grouping();
      for(HexField f = HexBoard::FIRST_NORMAL_FIELD; f < b.size(); f++) {
        if(b.get(f) == HEX_MARK_EMPTY) {
            HexMove m(HEX_MARK_VERT, f);
            double be = vertCond.energy(bg.groupIndex(bg(f)));
            double we = horiCond.energy(wg.groupIndex(wg(f)));
            moves.push_back(pair<HexMove, pair<double, double> >
                            (m, pair<double, double>(be, we)));
        }
      }
    }

    sort(moves.begin(), moves.end());
    for(unsigned i = 0; i < moves.size(); i++) {
      b.printField(cout, moves[i].first.field());
      cout << " V=" << moves[i].second.first + moves[i].second.second
           << " Eb=" << moves[i].second.first
           << " Ew=" << moves[i].second.second
           << endl;
    }
  }

  void testCircuit()
  {
    {
      HexBoard b(1, 1);
      showCond(b);
    }

    {
      HexBoard b(2, 2);
      showCond(b);
    }

    {
      HexBoard b(3, 3);
      showCond(b);
    }

    {
      HexBoard b(4, 4);
      showCond(b);
    }

    {
      HexBoard b(7, 7);
      showCond(b);
    }

    {
      HexBoard b(7, 7);
      b.set(b.coords2Field(3, 3), HEX_MARK_VERT);
      showCond(b);
    }
    {
      HexBoard b(7, 7);
      b.set(b.coords2Field(2, 3), HEX_MARK_VERT);
      showCond(b);
    }

    {
      HexBoard b(11, 11);
      showCond(b);
    }

    {
      HexBoard b(11, 11);
      b.set(b.coords2Field(0, 2), HEX_MARK_VERT);
      b.set(b.coords2Field(10, 0), HEX_MARK_HORI);
      showCond(b);
    }
  }
};

int main()
{
  Carrier::init();
  {
    CircuitTest t;
    t.testCircuit();
  }
  return 0;
}
