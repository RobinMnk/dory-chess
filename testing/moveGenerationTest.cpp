//
// Created by Robin on 29.07.2022.
//

#include <gtest/gtest.h>
#include "../src/dory.h"
#include "../src/utils/perft.h"

/**
 * Best Move found at depth 2
 * k7/p7/6b1/8/2K5/8/n3R1p1/8 w - - 0 1
 */

namespace Dory {

    using uLong = unsigned long long;

    TEST(NodeCounts, StartingPosition) {
        initialize();

        std::vector<uLong> ground_truth{
            1, 20, 400, 8'902, 197'281, 4'865'609, 119'060'324, 3'195'901'860, 84'998'978'956
        };

        const int maxDepth = 7;
        std::vector<uLong> result = DoryUtils::perft<true, maxDepth>(STARTBOARD);

        for (int i{1}; i <= maxDepth; i++) {
            uLong expected = ground_truth.at(i);
            uLong output = result.at(maxDepth + 1 - i);
            ASSERT_EQ(output, expected);
        }
    }

    template<int depth>
    void runNodeCountTest(std::string_view fen, std::vector<uLong> ground_truth) {
        initialize();
        const auto [board, whiteToMove] = Utils::parseFEN(fen);
        std::vector<uLong> result = DoryUtils::perft<depth>(board, whiteToMove);

        for (int i{1}; i <= depth; i++) {
            uLong expected = ground_truth.at(i);
            uLong output = result.at(depth + 1 - i);
            ASSERT_EQ(output, expected);
        }
    }

    TEST(NodeCounts, TestPos2) {
        runNodeCountTest<4>(
                "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
                std::vector<uLong>{
                        1, 48, 2'039, 97'862, 4'085'603, 193'690'690
                }
        );
    }

    TEST(NodeCounts, TestPos3) {
        runNodeCountTest<5>(
                "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
                std::vector<uLong>{
                        1, 14, 191, 2'812, 43'238, 674'624, 11'030'083, 178'633'661
                }
        );
    }

    TEST(NodeCounts, TestPos4w) {
        runNodeCountTest<5>(
                "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
                std::vector<uLong>{
                        1, 6, 264, 9'467, 422'333, 15'833'292, 706'045'033
                }
        );
    }

    TEST(NodeCounts, TestPos4b) {
        runNodeCountTest<5>(
                "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
                std::vector<uLong>{
                        1, 6, 264, 9'467, 422'333, 15'833'292, 706'045'033
                }
        );
    }

    TEST(NodeCounts, TestPos5) {
        runNodeCountTest<5>(
                "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
                std::vector<uLong>{
                        1, 44, 1486, 62'379, 2'103'487, 89'941'194
                }
        );
    }

    TEST(NodeCounts, TestPos6) {
        runNodeCountTest<5>(
                "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
                std::vector<uLong>{
                        1, 46, 2'079, 89'890, 3'894'594, 164'075'551, 6'923'051'137
                }
        );
    }

    template<int depth>
    void checkSingleDepth(std::string_view fen, uLong expected) {
        initialize();
        const auto [board, whiteToMove] = Utils::parseFEN(fen);
        uLong output = DoryUtils::perftSingleDepth<depth>(board, whiteToMove);
        ASSERT_EQ(output, expected);
    }

// - - - - - The following positions are taken from https://www.chessprogramming.net/perfect-perft/ - - - - -
    TEST(Scenarios, IllegalEpMove) {
        checkSingleDepth<6>("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 1134888);
    }

    TEST(Scenarios, IllegalEpMove2) {
        checkSingleDepth<6>("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 1015133);
    }

    TEST(Scenarios, EpCaptureCheck) {
        checkSingleDepth<6>("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 1440467);
    }

    TEST(Scenarios, ShortCastleCheck) {
        checkSingleDepth<6>("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 661072);
    }

    TEST(Scenarios, LongCastleCheck) {
        checkSingleDepth<6>("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 803711);
    }

    TEST(Scenarios, CastlingRights) {
        checkSingleDepth<4>("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 1274206);
    }

    TEST(Scenarios, CastlingPrevented) {
        checkSingleDepth<4>("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 1720476);
    }

    TEST(Scenarios, PromoteOutOfCheck) {
        checkSingleDepth<6>("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 3821001);
    }

    TEST(Scenarios, DiscoveredCheck) {
        checkSingleDepth<5>("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 1004658);
    }

    TEST(Scenarios, PromoteGiveCheck) {
        checkSingleDepth<6>("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 217342);
    }

    TEST(Scenarios, UnderPromoteGiveCheck) {
        checkSingleDepth<6>("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 92683);
    }

    TEST(Scenarios, SelfStalemate) {
        checkSingleDepth<6>("K1k5/8/P7/8/8/8/8/8 w - - 0 1", 2217);
    }

    TEST(Scenarios, StalemateAndCheckmate) {
        checkSingleDepth<7>("8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 567584);
    }

    TEST(Scenarios, StalemateAndCheckmate2) {
        checkSingleDepth<4>("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 23527);
    }

} // namespace Dory