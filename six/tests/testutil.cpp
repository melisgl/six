#include "testutil.h"

#include <ctype.h>

vector<HexMove> parseMoves(const HexBoard &b, const char *s)
{
  vector<HexMove> r;
  HexMark mark = HEX_MARK_VERT;
  while(*s) {
    while(*s && isspace(*s)) {
      s++;
    }
    if(*s) {
      if(strncmp(s, "SWAP", 4) == 0) {
        r.push_back(HexMove::createSwap(mark));
        s += 4;
      } else {
        int x, y;
        if(islower(*s)) {
          y = (*s) - 'a';
        } else {
          y = (*s) - 'A';
        }
        s++;
        x = atoi(s) - 1;
        r.push_back(HexMove(mark, b.coords2Field(x, y)));
      }
    }
    while(*s && !isspace(*s)) {
      s++;
    }
    mark = ((mark == HEX_MARK_VERT) ? HEX_MARK_HORI : HEX_MARK_VERT);
  }
  return r;
}

vector<HexMove> parseMoves2(const HexBoard &b, const char *s)
{
  vector<HexMove> r;
  HexMark mark = HEX_MARK_VERT;
  while(*s) {
    while(*s && isspace(*s)) {
      s++;
    }
    if(*s) {
      int x, y;
      if(islower(*s)) {
        x = (*s) - 'a';
      } else {
        x = (*s) - 'A';
      }
      s++;
      y = atoi(s) - 1;
      r.push_back(HexMove(mark, b.coords2Field(x, y)));
    }
    while(*s && !isspace(*s)) {
      s++;
    }
    mark = ((mark == HEX_MARK_VERT) ? HEX_MARK_HORI : HEX_MARK_VERT);
  }
  return r;
}
