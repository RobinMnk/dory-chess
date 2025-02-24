//
// Created by Robin on 22.12.2023.
//

#include <gtest/gtest.h>
#include <fstream>
#include "../src/dory.h"

namespace Dory::Testing {

    typedef std::pair<std::string, std::string> TestParam;
    typedef std::vector<TestParam> TestCaseInput;

    const std::array<std::string, 2> puzzleFiles{
            "../resources/puzzles2000.txt",
            "../resources/puzzlesSacrifices.txt"
    };

    const size_t LIMIT = 50;
    const int MAX_SEARCH_DEPTH = 6;

    class EngineTest : public ::testing::TestWithParam<TestParam> {
    protected:
        static Dory* dory;
        static TestCaseInput testCaseInput;

        // Set up the object once for the whole suite
        static void SetUpTestSuite() {
           dory = new Dory();
        }

        // Clean up after all tests are done
        static void TearDownTestSuite() {
            delete dory;
            dory = nullptr;
        }
    };
    Dory* EngineTest::dory = nullptr;

    TestCaseInput loadTestCases(int index, int limit) {
        std::vector<std::pair<std::string, std::string>> testCases;
        std::string fen, solution;
        std::ifstream file(puzzleFiles.at(index));
        int count = 0;
        if (file.is_open()) {
            while (count++ <= limit && std::getline(file, fen)) {
                std::getline(file, solution);
                testCases.emplace_back(fen, solution);
            }
            file.close();
        }
        return testCases;
    }


    TEST_P(EngineTest, NegamaxEngine) {
        auto [fen, solution] = GetParam();
        auto [board, whiteToMove] = Utils::parseFEN(fen);

        std::string output;
        if (whiteToMove) {
            auto [_, line] = dory->searchDepth<true>(board, MAX_SEARCH_DEPTH);
            output = Utils::moveNameShortNotation(line.back());
        } else {
            auto [_, line] = dory->searchDepth<false>(board, MAX_SEARCH_DEPTH);
            output = Utils::moveNameShortNotation(line.back());
        }

        ASSERT_EQ(output, solution);
    }

//TEST_P(EngineTest, MonteCarlo) {
//    auto [fen, solution] = GetParam();
//    auto [board, state] = Utils::loadFEN(fen);
//
//    Move move = MCTS(board, state, 1000000);
//    std::string output = Utils::moveNameShortNotation(move);
//
//    ASSERT_EQ(output, solution);
//}

    INSTANTIATE_TEST_SUITE_P(
            Puzzles2000,
            EngineTest,
            testing::ValuesIn(loadTestCases(0, LIMIT))
    );

    INSTANTIATE_TEST_SUITE_P(
            PuzzlesSacrifices,
            EngineTest,
            testing::ValuesIn(loadTestCases(1, LIMIT))
    );

} // namespace Dory