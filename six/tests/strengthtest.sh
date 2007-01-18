#!/bin/sh

BASE=`dirname $0`
STARTPOSITIONS=$BASE/start-positions

test_game() {
    GAME=p$2-p$3-`basename $1`
    if [ ! -f $GAME ]; then
        cp $1 $GAME
        rm -f $GAME.log
    fi
    $BASE/auto-player $2 $3 $GAME 2>&1 | tee -a $GAME.log
}

for i in $STARTPOSITIONS/*.six; do
    test_game $i 3 2
    test_game $i 2 3
done
