// Yo Emacs, this -*- C++ -*-
#ifndef TESTUTIL_H
#define TESTUTIL_H

#include "hexmove.h"
#include "hexboard.h"

#include <vector>

vector<HexMove> parseMoves(const HexBoard &b, const char *s);
vector<HexMove> parseMoves2(const HexBoard &b, const char *s);

#endif
