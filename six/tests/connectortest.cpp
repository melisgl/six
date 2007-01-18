#undef NDEBUG

#include "connector.h"
#include "hexgame.h"
#include "misc.h"

#include "testutil.h"

#include <cassert>
#include <cmath>

#include <sstream>
#include <string>

using std::cout;
using std::endl;
using std::string;

static Poi<Connector::DualBatchLimiter>
createSoftLimiter(unsigned smc, unsigned hmc, unsigned sms, unsigned hms)
{
  return Poi<Connector::DualBatchLimiter>
    (new Connector::SoftLimiter(smc, hmc, sms, hms));
}


class ConnectorTest
{
public:
  static void print(const Connector::GroupPairQueue &q)
  {
//     cout << "(GroupPairs to b processed:" << endl;
//     for(unsigned int i = 0; i < q.size(); i++) {
//       cout << q[i] << endl;
//     }
//     cout << ")" << endl;
  }

  static void print(const Batch &b)
  {
    cout << "(Batch:";
    vector<Carrier> v;
    Batch::Iterator cur = b.begin();
    Batch::Iterator end = b.end();
    while(cur != end) {
      v.push_back(*cur);
      ++cur;
    }
    sort(v.begin(), v.end());
    for(unsigned i = 0; i < v.size(); ++i) {
      cout << v[i];
      cout << " ";
    }
    cout << ")" << endl;
  }

  static void print(const DualBatch &db)
  {
    cout << "(DualBatch :x " << *db.minGroup()
         << " :y " << *db.maxGroup() << endl;
    cout << " :conns ";
    print(db.connBatch());
    cout << " :semis ";
    print(db.semiBatch());
    cout << ")" << endl;
  }

  static int countSemis(const Connector &c)
  {
    int r = 0;
    const Connector::DualBatchMap &m = c._map;
    Connector::DualBatchMap::const_iterator cur = m.begin();
    Connector::DualBatchMap::const_iterator end = m.end();
    while(cur != end) {
      r += (*cur).second.semiBatch().size();
      ++cur;
    }
    return r;
  }

  static int countConns(const Connector &c)
  {
    int r = 0;
    const Connector::DualBatchMap &m = c._map;
    Connector::DualBatchMap::const_iterator cur = m.begin();
    Connector::DualBatchMap::const_iterator end = m.end();
    while(cur != end) {
      r += (*cur).second.connBatch().size();
      ++cur;
    }
    return r;
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

  static void print(const Connector &c, bool dump = false)
  {
    cout << "connWinner=" << c.connWinner() << endl;
    cout << "nNew=" << c._queue.size()
         << ", nConn=" << countConns(c)
         << ", nSemi=" << countSemis(c)
         << endl;
    if(dump) {
      cout << c.grouping();
      print(c._queue);
      const Connector::DualBatchMap &m = c._map;
      Connector::DualBatchMap::const_iterator cur = m.begin();
      Connector::DualBatchMap::const_iterator end = m.end();
      while(cur != end) {
        print((*cur).second);
        ++cur;
      }
    }
  }

//   static void checkNoEmptySemiBatch(const Connector::SemiBatchMap &semis)
//   {
//     Connector::SemiBatchMap::const_iterator cur = semis.begin();
//     Connector::SemiBatchMap::const_iterator end = semis.end();
//     while(cur != end) {
//       if((*cur).second.empty()) {
//         cout << *(*cur).first.start() << " " << *(*cur).first.end() << endl;
//       }
//       assert(!(*cur).second.empty());
//       ++cur;
//     }
//   }

//   static void checkNoEmptyConnBatch(const Connector::ConnBatchMap &conns)
//   {
//     Connector::ConnBatchMap::const_iterator cur = conns.begin();
//     Connector::ConnBatchMap::const_iterator end = conns.end();
//     while(cur != end) {
//       if((*cur).second.empty()) {
//         cout << *(*cur).first.start() << " " << *(*cur).first.end() << endl;
//       }
//       assert(!(*cur).second.empty());
//       ++cur;
//     }
//   }

  static void checkNoUnprocessed(const Connector &c)
  {
    if(c.connWinner() == HEX_MARK_EMPTY) {
      const Connector::DualBatchMap &m = c._map;
      Connector::DualBatchMap::const_iterator cur = m.begin();
      Connector::DualBatchMap::const_iterator end = m.end();
      while(cur != end) {
        if(std::find(c._queue.begin(), c._queue.end(), (*cur).second) ==
           c._queue.end() &&
           ((*cur).second.connBatch().hasUnprocessed() ||
            (*cur).second.semiBatch().hasUnprocessed())) {
          print((*cur).second);
          cout << "HAS" << (*cur).second.semiBatch().hasUnprocessed() << endl;
          cout << "SIZE" << (*cur).second.semiBatch().size() << endl;
        }
        if(std::find(c._queue.begin(), c._queue.end(), (*cur).second) ==
           c._queue.end()) {
           assert(!(*cur).second.connBatch().hasUnprocessed());
           assert(!(*cur).second.semiBatch().hasUnprocessed());
        }
        ++cur;
      }
    }
  }

  static void checkBatchOrdering(const Connector &c)
  {
    const Connector::DualBatchMap &m = c._map;
    Connector::DualBatchMap::const_iterator cur = m.begin();
    Connector::DualBatchMap::const_iterator end = m.end();
    while(cur != end) {
      //FIXME
      ++cur;
    }
  }

  static void checkQueue(const Connector &c)
  {
    assert(c._queue.empty() || c.connWinner() != HEX_MARK_EMPTY);
  }

  void check(const Connector &c)
  {
//     checkNoEmptyConnBatch(c.conns());
//     checkNoEmptySemiBatch(c.semis());
    checkNoUnprocessed(c);
    checkBatchOrdering(c);
    checkQueue(c);
  }

  void testSsss()
  {
    {
      HexBoard b(1, 1);
      cout << b;
      Connector vert;
      Connector hori;
      vert.init(b, HEX_MARK_VERT);
      hori.init(b, HEX_MARK_HORI);
      print(vert, true);
      assert(countConns(vert) == 2);
      assert(countSemis(vert) == 1);
      assert(countConns(hori) == 2);
      assert(countSemis(hori) == 1);
    }

    {
      HexBoard b(1, 2);
      cout << b;
      Connector vert;
      Connector hori;
      vert.init(b, HEX_MARK_VERT);
      hori.init(b, HEX_MARK_HORI);
//       print(vert, true);
      print(hori, true);
      assert(vert.winner() == b.winner() &&
             hori.winner() == b.winner());
      assert(vert.connWinner() == HEX_MARK_EMPTY);
      assert(countConns(vert) == 3);
      assert(countSemis(vert) == 2);
      assert(hori.connWinner() == HEX_MARK_HORI);
    }

    {
      HexBoard b(2, 1);
      cout << b;
      Connector vert;
      Connector hori;
      vert.init(b, HEX_MARK_VERT);
      hori.init(b, HEX_MARK_HORI);
      assert(vert.winner() == b.winner() &&
             hori.winner() == b.winner());
      assert(vert.connWinner() == HEX_MARK_VERT);
      assert(hori.connWinner() == HEX_MARK_EMPTY);
      assert(countConns(hori) == 3);
      assert(countSemis(hori) == 2);
    }

    {
      HexBoard b(2, 2);
      cout << b;
      Connector vert;
      Connector hori;
      vert.init(b, HEX_MARK_VERT);
      hori.init(b, HEX_MARK_HORI);
      assert(vert.winner() == b.winner() &&
             hori.winner() == b.winner());
      assert(vert.connWinner() == HEX_MARK_EMPTY);
      assert(countConns(vert) == 12);
      assert(countSemis(vert) == 10);
      assert(hori.connWinner() == HEX_MARK_EMPTY);
      assert(countConns(hori) == 12);
      assert(countSemis(hori) == 10);
    }

    {
      HexBoard b(2, 3);
      cout << b;
      Connector vert;
      Connector hori;
      vert.init(b, HEX_MARK_VERT);
      hori.init(b, HEX_MARK_HORI);
      assert(vert.winner() == b.winner() &&
             hori.winner() == b.winner());
      assert(vert.connWinner() == HEX_MARK_EMPTY);
      assert(countConns(vert) == 18);
      assert(hori.connWinner() == HEX_MARK_HORI);
    }
  }

  void testCccc()
  {
    HexBoard b(4, 4);
    cout << b;
    Connector vert(createSoftLimiter(MAXINT, MAXINT, MAXINT, MAXINT),
                   MAXINT, true);
    Connector vertCov(createSoftLimiter(MAXINT, MAXINT, MAXINT, MAXINT),
                       MAXINT, true, true);
    cout << "NO COV" << endl;
    vert.init(b, HEX_MARK_VERT);
    print(vert, 1);
    cout << "WITH COV" << endl;
    vertCov.init(vert.grouping());
    print(vertCov, 1);
    
//     vert.move(HexMove(HEX_MARK_VERT, b.coords2Field(0, 1)));
//     print(vert, 1);
//     hori.init(b, HEX_MARK_HORI);
//     print(hori, 1);
  }

  void testWins()
  {
    {
      HexBoard b0(3, 3);
      for(HexField f = HexBoard::FIRST_NORMAL_FIELD; f < b0.size(); f++) {
        HexBoard b(b0);
        b.set(f, HEX_MARK_VERT);
        cout << b;
        Connector vert;
        Connector hori;
        vert.init(b, HEX_MARK_VERT);
        int x, y;
        b.field2Coords(f, &x, &y);
        assert((y + x == 2 || (x == 0 && y == 1) || (x == 2 && y == 1)) ==
               (vert.connWinner() == HEX_MARK_VERT));
        hori.init(b, HEX_MARK_HORI);
        assert(hori.connWinner() == HEX_MARK_EMPTY);
      }
    }
  }

  HexMove randomMove(const HexGame &g)
  {
    vector<HexMove> moves;
    for(HexField f = HexBoard::FIRST_NORMAL_FIELD; f < g.board().size(); f++) {
      if(g.board().get(f) == HEX_MARK_EMPTY) {
        moves.push_back(HexMove(g.next(), f));
      }
    }
    return moves[RANDOM(moves.size())];
  }

  void moveMover(Connector &mover, const HexMove &m,
                 int softMaxConn, int hardMaxConn,
                 int softMaxSemi, int hardMaxSemi,
                 int maxInOrRule, bool useEdge)
  {
    HexMark mark = mover.grouping().mark();
    string side = ((mark == HEX_MARK_VERT) ? "VERT" : "HORI");
    cout << "Moving " << side << endl;
    mover.move(m, true);
    cout << "Initializing " << side << endl;
    Connector inited(createSoftLimiter(softMaxConn, hardMaxConn,
                                       softMaxSemi, hardMaxSemi),
                     maxInOrRule, useEdge);
//     inited.init(mover.board(), mark);
    inited.init(mover.grouping());
    cout << side << " mover" << endl;
    print(mover);
    cout << side << " inited" << endl;
    print(inited);
    if(mover.winner() == HEX_MARK_EMPTY &&
       mover.connWinner() == mark) {
      cout << "Mover WC=" << mover.winningConnCarrier() << endl;
    }
    if(inited.winner() == HEX_MARK_EMPTY &&
       inited.connWinner() == mark) {
      cout << "Inited WC=" << inited.winningConnCarrier() << endl;
    }
    cout << "Checking mover" << endl;
    check(mover);
    cout << "Checking inited" << endl;
    check(inited);
    if(mover.winner() != inited.winner() ||
       mover.connWinner() != inited.connWinner() ||
       (mover.connWinner() == HEX_MARK_EMPTY &&
        (softMaxConn == MAXINT && hardMaxConn == MAXINT &&
         softMaxSemi == MAXINT && hardMaxSemi == MAXINT &&
         maxInOrRule == MAXINT) &&
        (mover._map != inited._map))) {
      cout << "MOVER" << endl;
      print(mover, true);
      cout << "INITED" << endl;
      print(inited, true);
    }
    assert(mover.winner() == inited.winner());
    assert(mover.connWinner() == inited.connWinner());
    assert(mover.connWinner() != HEX_MARK_EMPTY ||
           !(softMaxConn == MAXINT && hardMaxConn == MAXINT &&
             softMaxSemi == MAXINT && hardMaxSemi == MAXINT &&
             maxInOrRule == MAXINT) ||
           mover._map == inited._map);
  }

  void testMove(const HexBoard &b0, const vector<HexMove> &moves,
                int softMaxConn = MAXINT, int hardMaxConn = MAXINT,
                int softMaxSemi = MAXINT, int hardMaxSemi = MAXINT,
                int maxInOrRule = 5, bool useEdge = false)
  {
    HexGame g(b0);
    cout << g.board();
    Connector vert(createSoftLimiter(softMaxConn, hardMaxConn,
                                     softMaxSemi, hardMaxSemi),
                   maxInOrRule, useEdge);
    Connector hori(createSoftLimiter(softMaxConn, hardMaxConn,
                                     softMaxSemi, hardMaxSemi),
                   maxInOrRule, useEdge);
    check(vert);
    check(hori);
    cout << "Initializing VERT..." << endl;
    vert.init(g.board(), HEX_MARK_VERT);
    cout << "Initializing HORI..." << endl;
    hori.init(g.board(), HEX_MARK_HORI);
    cout << "Checking initial VERT" << endl;
    check(vert);
    cout << "Checking initial HORI" << endl;
    check(hori);
    for(unsigned i = 0; i < moves.size(); i++) {
      HexMove m = moves[i];
      cout << "Playing ";
      g.printMove(cout, m);
      cout << endl;
      g.play(m, 0);
      cout << g.board();
      moveMover(vert, m, softMaxConn, hardMaxConn, softMaxSemi, hardMaxSemi,
                maxInOrRule, useEdge);
      moveMover(hori, m, softMaxConn, hardMaxConn, softMaxSemi, hardMaxSemi,
                maxInOrRule, useEdge);
      cout << endl << endl;
    }
  }

  void testMove()
  {
    {
      HexBoard b(2, 2);
      testMove(b, parseMoves(b, "a1"), MAXINT, MAXINT, MAXINT, MAXINT, MAXINT);
    }

    {
      HexBoard b(11, 11);
      testMove(b, parseMoves(b, "f3 SWAP f6"), MAXINT, MAXINT, 7, MAXINT,
               4, true);
    }

    //for(unsigned i = 0; i < 10000; i++)
    {
      HexBoard b(4, 4);
      testMove(b, parseMoves2(b, "d1 b4 a2"), MAXINT, MAXINT, MAXINT, MAXINT,
               MAXINT, true);
    }

    {
      HexBoard b(4, 4);
      testMove(b, parseMoves2(b, "d1 c4 a2"), MAXINT, MAXINT, MAXINT, MAXINT,
               MAXINT, true);
    }

    {
      HexBoard b(11, 11);
      testMove(b, parseMoves(b, "b1 c9 f6 e6 d8 c8 d7 c7 d6 c6 d3 c3\
                           c4 b4 b5 a5 a6 d4 e3 i4 h7 g7 h5 i5\
                           h6 j6 i8 j9 k7 h8 i7 f4 e4 e8 e7 j5\
                           j7 g2 e5 e10 g6"),
               MAXINT, MAXINT, 7, MAXINT, 4);
    }

    {
      HexBoard b(11, 11);
      testMove(b, parseMoves(b, "B1 F6 H3 E4 F1 G1 F7 E7 E9 D9 C11 B10\
                           C9 C10 A11 B11 D8 E8 F3 E3 F2 D2 D1 E1\
                           E6 G4 G3 F4 F5 G5 D3 E2"),
               MAXINT, MAXINT, 7, MAXINT, 4);
    }

    {
      HexBoard b(11, 11);
      testMove(b, parseMoves(b, "B1 F6 F5 G5 H3 G3 G4 D6 E4 B5 J2 C3 C4 B4 C6 D5 D3 E3 D4 C5 D7 E6 D1 D2 F7 G8 H6 E7 D11 F10 E8 D8 E1 E2 F1 G2 H1 G1 F3 F2 C10 D9 E9 D10 B10 C11"),
               MAXINT, MAXINT, 7, MAXINT, 4);
    }

    {
      HexBoard b(11, 11);
      testMove(b, parseMoves(b, "I3 F6 F5 C6 C9 D7 D3 E4 F3 E3 D6 C7 D5 H4 H2 F4 G3 G4 G5 K3 J5 H6 H5 I5 H7 F8 D10 F9 E11 G10 G9 F10 G7 G6 I4 B5 C5 B6 C3 A3 A4 B3 B4 D4 C4 E5 E6 K4 K2 J3 J2"),
               MAXINT, MAXINT, 7, MAXINT, 4);
    }

    {
      HexBoard b(11, 11);
      testMove(b, parseMoves(b, "I3 F5 F6 G3 G4 F4 D7 G5 G7 J6 H7 K4 I8 K9 K7 J8 J7 I7 H8 G6 F7 A8 C7 B8 B6 C6 B7 A6 A7 E1 E7"),
               MAXINT, MAXINT, 7, MAXINT, 4);
    }

    {
      HexBoard b(11, 11);
      testMove(b, parseMoves(b, "C8 F6 F5 H4 H3 J2 J1 I2 I3 J3 I4 J4 I6 E8 H5 D6 E7 E6 F7 H6 G8 J7 H7 J6 J5 A8 B9 A10 A9 G7 F8 D8 D7"),
               MAXINT, MAXINT, 7, MAXINT, 4);
    }

    {
      HexBoard b(11, 11);
      testMove(b, parseMoves(b, "J11 C9 F5 B7 H4 I4 I3 K2 J3 K3 K1 J2 J1 I2 J4 K4 J5 K5 J6 K6 J7 E5 D7 C8 C6 C7 D6 B6 C4 C5 D4 A5 B3 A3 A4 C3 B4 F3 D5 E6 E7 F6 F7 G6 G7 H6 H7 I7 I6 K7 J8 I10 K8"),
               MAXINT, MAXINT, 7, MAXINT, 4);
    }
  }

  void testRandomMoves() {
    const int SIZE = 4;
    for(int i = 0; i < 10000; i++) {
      HexGame g(HexBoard(SIZE, SIZE));
      vector<HexMove> m;
      while(g.board().winner() == HEX_MARK_EMPTY) {
        HexMove move = randomMove(g);
        m.push_back(move);
        g.play(move, 0);
      }
      testMove(HexBoard(SIZE, SIZE), m, MAXINT, MAXINT, MAXINT, MAXINT,
               MAXINT, true);
    }
  }

  void testPerf()
  {
//     {
//       HexBoard b(11, 11);
// //       Connector c(MAXINT, MAXINT, 10, MAXINT, 4, false);
//       Connector c(3, MAXINT, 7, MAXINT, 4, true);
//       vector<HexMove> m;
//       m = parseMoves(b, "J11 C9 F5 B7 H4 I4 I3 K2 J3 K3 K1 J2 J1 I2 J4 K4 J5 K5 J6 K6 J7 E5 D7 C8 C6 C7 D6 B6 C4 C5 D4 A5 B3 A3 A4 C3 B4 F3 D5 E6 E7 F6 F7 G6 G7 H6 H7 I7 I6 K7 J8 I10 K8");
// //       m = parseMoves("B1 F6 H3 E4 F1 G1 F7 E7 E9 D9 C11 B10
// //                      C9 C10 A11 B11 D8 E8 F3 E3 F2 D2 D1 E1
// //                      E6 G4 G3 F4 F5 G5 D3 E2");

//       clock_t t0, t1;
//       clock_t ts0, ts1;
//       clock_t tm0, tm1;
//       vector<pair<clock_t, clock_t> > tmi(m.size());
//       t0 = ts0 = clock();
//       c.init(b, HEX_MARK_HORI);
//       ts1 = tm0 = clock();
//       for(unsigned i = 0; i < m.size(); i++) {
//         tmi[i].first = clock();
//         c.move(m[i]);
//         tmi[i].second = clock();
//         cout << "i=" << i << m[i].mark();
//         b.printField(cout, m[i].field());
//         cout << endl;
//         cout << c.grouping().board();
//       }
//       tm1 = t1 = clock();
//       cout << "Setup time: " << ((long)ts1 - (long)ts0) << endl;
//       for(unsigned i = 0; i < tmi.size(); i++) {
//         cout << "Move[" << i << "] time: "
//              << ((long)tmi[i].second - (long)tmi[i].first) << endl;
//       }
//       cout << "Move time: " << ((long)tm1 - (long)tm0) << endl;
//       cout << "Total time: " << ((long)t1 - (long)t0) << endl;
//     }
    {
      HexBoard b(11, 11);
      //     b.play(HexMove(HEX_MARK_VERT, 0, 0));
//       Connector c(createSoftLimiter(MAXINT, MAXINT, 100, MAXINT), 4, false);
//       Connector c(createSoftLimiter(MAXINT, MAXINT, 15, MAXINT), 5, true, false);
      Connector c(createSoftLimiter(MAXINT, MAXINT, 15, MAXINT), 5, true);
//       Connector c(createSoftLimiter(3, MAXINT, 7, MAXINT), 4, true);
//       Connector c(createSoftLimiter(MAXINT, MAXINT, 40, MAXINT), 4, true);
      vector<HexMove> m;
      m = parseMoves(b, "J11 C9 F5 B7 H4 I4 I3 K2 J3 K3 K1 J2 J1 I2 J4 K4 J5 K5 J6 K6 J7 E5 D7 C8 C6 C7 D6 B6 C4 C5 D4 A5 B3 A3 A4 C3 B4 F3 D5 E6 E7 F6 F7 G6 G7 H6 H7 I7 I6 K7 J8 I10 K8");
//       m = parseMoves("B1 F6 H3 E4 F1 G1 F7 E7 E9 D9 C11 B10
//                      C9 C10 A11 B11 D8 E8 F3 E3 F2 D2 D1 E1
//                      E6 G4 G3 F4 F5 G5 D3 E2");

      clock_t t0, t1;
      clock_t ts0, ts1;
      clock_t tm0, tm1;
      vector<pair<clock_t, clock_t> > tmi(m.size());
      t0 = ts0 = clock();
      c.init(b, HEX_MARK_VERT);
      ts1 = tm0 = clock();
      print(c);
      for(unsigned i = 0; i < m.size(); i++) {
//         system("ps axu | grep connectortest");
        tmi[i].first = clock();
//         {
//           system("ps axu | grep connectortest");
//           Connector copyc(c);
//           {
//             system("ps axu | grep connectortest");
//             Connector copyc2(c); 
//             {
//               system("ps axu | grep connectortest");
//               Connector copyc3(c);
//               system("ps axu | grep connectortest");
//             }}}

        c.move(m[i]);
        tmi[i].second = clock();
        cout << "i=" << i << m[i].mark();
        b.printField(cout, m[i].field());
        cout << endl;
        print(c);
        cout << c.grouping().board();
      }
      tm1 = t1 = clock();
      cout << "Setup time: " << ((long)ts1 - (long)ts0) << endl;
      for(unsigned i = 0; i < tmi.size(); i++) {
        cout << "Move[" << i << "] time: "
             << ((long)tmi[i].second - (long)tmi[i].first) << endl;
      }
      cout << "Move time: " << ((long)tm1 - (long)tm0) << endl;
      cout << "Total time: " << ((long)t1 - (long)t0) << endl;
    }
//     {
//       HexBoard b(11, 11);
//       Connector odd(3, MAXINT, 7, MAXINT, 4, true);
//       Connector even(3, MAXINT, 7, MAXINT, 4, true);
//       vector<HexMove> m;
//       m = parseMoves(b, "J11 C9 F5 B7 H4 I4 I3 K2 J3 K3 K1 J2 J1 I2 J4 K4 J5 K5 J6 K6 J7 E5 D7 C8 C6 C7 D6 B6 C4 C5 D4 A5 B3 A3 A4 C3 B4 F3 D5 E6 E7 F6 F7 G6 G7 H6 H7 I7 I6 K7 J8 I10 K8");
//       clock_t t0, t1;
//       clock_t ts0, ts1;
//       clock_t tm0, tm1;
//       vector<pair<clock_t, clock_t> > tmi(m.size());
//       t0 = ts0 = clock();
//       odd.init(b, HEX_MARK_VERT);
//       even.init(b, HEX_MARK_VERT);
//       ts1 = tm0 = clock();
//       //print(c);
//       for(unsigned i = 0; i < m.size(); i+= 2) {
//         tmi[i].first = clock();
//         even.moveWithoutCalc(m[i]);
//         if(i > 0)
//           even.moveWithoutCalc(m[i - 1]);
//         even.calc();
//         tmi[i].second = clock();
//         cout << "i=" << i << m[i].mark();
//         b.printField(cout, m[i].field());
//         cout << endl;
//         //print(c);
//         //cout << c.grouping().board();
//         if(i + 1 < m.size()) {
//           tmi[i + 1].first = clock();
//           odd.moveWithoutCalc(m[i]);
//           odd.moveWithoutCalc(m[i + 1]);
//           odd.calc();
//           tmi[i + 1].second = clock();
//           cout << "i=" << i + 1 << m[i + 1].mark();
//           b.printField(cout, m[i + 1].field());
//           cout << endl;
//         }
//       }
//       tm1 = t1 = clock();
//       cout << "Setup time: " << ((long)ts1 - (long)ts0) << endl;
//       for(unsigned i = 0; i < tmi.size(); i++) {
//         cout << "Move[" << i << "] time: "
//              << ((long)tmi[i].second - (long)tmi[i].first) << endl;
//       }
//       cout << "Move time: " << ((long)tm1 - (long)tm0) << endl;
//       cout << "Total time: " << ((long)t1 - (long)t0) << endl;
//     }
  }


  void testShufflingMove()
  {
    HexMark myMark = HEX_MARK_VERT;
    HexBoard b(11, 11);
    Connector c0(createSoftLimiter(MAXINT, MAXINT, 40, MAXINT), 5, true);
    Poi<Connector> c1;
    vector<HexMove> m;
    m = parseMoves(b, "J11 C9 F5 B7 H4 I4 I3 K2 J3 K3 K1 J2 J1 I2 J4 K4 J5 K5 J6 K6 J7 E5 D7 C8 C6 C7 D6 B6 C4 C5 D4 A5 B3 A3 A4 C3 B4 F3 D5 E6 E7 F6 F7 G6 G7 H6 H7 I7 I6 K7 J8 I10 K8");
    clock_t t0, t1;
    clock_t ts0, ts1;
    clock_t tm0, tm1;
    vector<pair<clock_t, clock_t> > tmi(m.size());
    t0 = ts0 = clock();
    c0.init(b, myMark);
    ts1 = tm0 = clock();
    print(c0);
    for(int i = 0; i < (signed)m.size(); i++) {
      tmi[i].first = clock();
      if(m[i].mark() == myMark) {
        cout << "Shuffled move" << endl;
        c0.move(m[i], false, false);
        if(i - 1 >= 0)
          c0.move(m[i - 1], false, false);
        c0.calc();
      } else {
        cout << "Normal move" << endl;
        c1 = new Connector(c0);
        (*c1).move(m[i]);
      }
      tmi[i].second = clock();
      cout << "i=" << i << m[i].mark();
      b.printField(cout, m[i].field());
      cout << endl;
      if(m[i].mark() == myMark) {
        print(c0);
        cout << c0.grouping().board();
      } else {
        print(*c1);
        cout << (*c1).grouping().board();
      }
    }
    tm1 = t1 = clock();
    cout << "Setup time: " << ((long)ts1 - (long)ts0) << endl;
    for(unsigned i = 0; i < tmi.size(); i++) {
      cout << "Move[" << i << "] time: "
           << ((long)tmi[i].second - (long)tmi[i].first) << endl;
    }
    cout << "Move time: " << ((long)tm1 - (long)tm0) << endl;
    cout << "Total time: " << ((long)t1 - (long)t0) << endl;
  }

};

int main()
{
  Carrier::init();
  ConnectorTest t;
//   t.testCccc();
//   t.testSsss();
//   t.testWins();
//   t.testMove();
//   t.testRandomMoves();
  t.testPerf();
//   t.testShufflingMove();
  return 0;
}
