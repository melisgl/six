// Yo Emacs, this -*- C++ -*-
#ifndef MISC_H
#define MISC_H

#define SWAP(a1, a2, tmp)  { (tmp) = (a1); (a1) = (a2); (a2) = (tmp); }
#define MIN(a, b)          (((a) < (b)) ? (a) : (b))
#define MAX(a, b)          (((a) < (b)) ? (b) : (a))

#define RANDOM(num)        ((int)(((rand() * double(num)) / (RAND_MAX+1.))))
#define DRANDOM(d)         (((rand() * double(d)) / (RAND_MAX + 1.)))
#define RANDOMIZE()        srand((unsigned)time(0))
#define RANDOMSEED(s)      srand((unsigned)s)

#ifndef NDEBUG
#include <iostream>
#define DBG                std::cout
#else
#include <sstream>
extern std::ostringstream cnull;
#define DBG                cnull
#endif

#endif
