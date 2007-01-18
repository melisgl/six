#include "hexmatch.h"
#include "sixplayer.h"

#include <iostream>
#include <fstream>

void play(char *filename, Poi<HexPlayer> vert, Poi<HexPlayer> hori)
{
  HexGame g;
  {
    std::ifstream is(filename);
    g.load(is);
  }
  HexMatch m(g, vert, hori);
  while (m.status() != HexMatch::MATCH_FINISHED) {
    m.doSome();
    std::cout << m.game().board();
    {
      std::ofstream os(filename);
      m.game().save(os);
    }
  }
}

int main(int argc, char **argv)
{
  assert(argc == 4);
  Carrier::init();
  play(argv[3],
       Poi<HexPlayer>(new SixPlayer((SixPlayer::LevelT)atoi(argv[1]),
                                    true)),
       Poi<HexPlayer>(new SixPlayer((SixPlayer::LevelT)atoi(argv[2]),
                                    true)));
}
