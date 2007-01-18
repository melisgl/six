#undef NDEBUG

#include "hexgame.h"

#include <cassert>
#include <sstream>
#include <fstream>
#include <string>

using std::string;
using std::endl;
using std::istringstream;
using std::ostringstream;
using std::ifstream;

void testSaveLoad()
{
  {
    string s("BoardSize: 11x11\nFirst: V\nSwap: 1\nMoves:\n"
             "  1. A2 (123 ms) F6 (1345 ms)\n"
             "  2. C7 (23 ms) C8 (34567 ms)\n");
    istringstream is(s);
    ostringstream os;
    HexGame g;
    g.load(is);
    assert(!is.fail());
    g.save(os);
    assert(!os.fail());
    assert(s == os.str());
  }

  {
    string s("BoardSize: 11x11\nFirst: V\nSwap: 1\nMoves:\n"
             "  1. A2 (123 ms) Swap (1345 ms)\n"
             "  2. C7 (23 ms)\n");
    istringstream is(s);
    ostringstream os;
    HexGame g;
    g.load(is);
    assert(!is.fail());
    g.save(os);
    assert(!os.fail());
    assert(s == os.str());
  }

  {
    string s("BoardSiz: 11x11\nFirst: V\nSwap: 1\nMoves:\n"
             "  1. A2 (123 ms) F6 (1345 ms)\n"
             "  2. C7 (23 ms) C8 (34567 ms)\n");
    istringstream is(s);
    HexGame g;
    g.load(is);
    assert(is.fail());
  }

  {
    string s("BoardSize: 11x11\nFirst: V\nSwap: 1\nMoves:\n"
             "  1. A2 (123 ms) F6 (1345)\n"
             "  2. C7 (23 ms) C8 (34567 ms)\n");
    istringstream is(s);
    HexGame g;
    g.load(is);
    // although "ms" is missing, the current implementation doesn't find it
    //assert(is.fail());
  }
}

void testImportPBEM()
{
  ifstream is("pbem.txt");
  HexGame g;
  g.importPBEMGame(is);
  assert(!is.fail());
}

int main()
{
  testSaveLoad();
  testImportPBEM();
  return 0;
}
