//
// Created by robin on 29.07.2022.
//

#include <gtest/gtest.h>

#include "../src/movecollectors.h"
#include "../src/fenreader.h"

using Collector = MoveCollectors::LimitedDFS<false, false>;

template<int depth>
struct Runner {
    template<State state>
    static void main(Board& board) {
        Collector::template generateGameTree<state, depth>(board);
    }
};

TEST(StartingPosition, Depth1) {
    PieceSteps::load();
    Board board = STARTBOARD;
    Runner<1>::template main<STARTSTATE>(board);
    ASSERT_EQ(Collector::totalNodes, 20);
}

TEST(StartingPosition, Depth2) {
    PieceSteps::load();
    Board board = STARTBOARD;
    Runner<2>::template main<STARTSTATE>(board);
    ASSERT_EQ(Collector::totalNodes, 400);
}

TEST(StartingPosition, Depth3) {
    PieceSteps::load();
    Board board = STARTBOARD;
    Runner<3>::template main<STARTSTATE>(board);
    ASSERT_EQ(Collector::totalNodes, 8902);
}