// Yo Emacs, this -*- C++ -*-
#ifndef ASYNCPLAYER_H
#define ASYNCPLAYER_H

#include "hexplayer.h"

/**
 * AsyncPlayer can be told which move to play next time it is asked to.
 */
class AsyncPlayer : public HexPlayer
{
public:
  /**
   * Constructs an AsyncPlayer that doesn't have a move.
   */
  AsyncPlayer();

  /**
   * Does nothing.
   */
  void init(const HexGame *, HexMark yourMark);

  /**
   * Does nothing.
   */
  void played(const HexMove &m);

  /**
   * Returns and forgets the stored move if it has one.
   */
  pair<bool, HexMove> play();

  /**
   * Stores the move.
   */
  void play(const HexMove &);
private:
  bool _hasMove;
  HexMove _move;
};

#endif
