#include "asyncplayer.h"

AsyncPlayer::AsyncPlayer()
{
  _hasMove = false;
}

void AsyncPlayer::init(const HexGame *, HexMark)
{
}

void AsyncPlayer::played(const HexMove &)
{
}

pair<bool, HexMove> AsyncPlayer::play()
{
  pair<bool, HexMove> m;
  m.first = _hasMove;
  if(_hasMove) {
    m.second = _move;
    _hasMove = false;
  }
  return m;
}

void AsyncPlayer::play(const HexMove &move)
{
  if(!_hasMove) {
    _move = move;
    _hasMove = true;
  }
}
