#include <iostream>

#include "board.h"
#include "engine/engine.h"
#include "engine/monte_carlo.h"
#include "old/movecollectors.h"
#include "fenreader.h"

//using Collector = MoveCollectors::LimitedDFS<false, false>;

//NMR timeEvaluation(const BoardPtr& board, const State state, int depth) {
//    auto t1 = std::chrono::high_resolution_clock::now();
//    auto [eval, line] = EngineMC::beginEvaluation(board, state, depth);
//    auto t2 = std::chrono::high_resolution_clock::now();
//
//    auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);
//
//    std::cout << "Evaluation: " << eval << std::endl;
//
//    std::chrono::duration<double> seconds = t2 - t1;
//
//    std::cout << "Searched " << EngineMC::nodesSearched << " nodes in " << ms_int.count() << "ms";
//
//    double knps = (static_cast<double>(EngineMC::nodesSearched) / 1000) / seconds.count();
//    if (knps < 1000) {
//        std::cout << "\t\t(" << knps << " k nps)\n\n";
//    } else {
//        std::cout << "\t\t(" << (knps / 1000) << " M nps)\n\n";
//    }
//
//    return {eval, line};
//}

struct Runner {
    template<State state, int depth>
    static void main(const Board& board) {
        MoveCollectors::nodes.resize(7);

        auto t1 = std::chrono::high_resolution_clock::now();
        MoveCollectors::PerftCollector<6>::template generateGameTree<state>(board);
        auto t2 = std::chrono::high_resolution_clock::now();

        auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);

        std::chrono::duration<double> seconds = t2 - t1;

        unsigned long long nodes = MoveCollectors::nodes.at(1);

        std::cout << "Generated " <<  nodes << " nodes in " << ms_int.count() << "ms";

        double knps = (static_cast<double>(nodes) / 1000) / seconds.count();
        if (knps < 1000) {
            std::cout << "\t\t(" << knps << " k nps)\n\n";
        } else {
            std::cout << "\t\t(" << (knps / 1000) << " M nps)\n\n";
        }



//        bool monte = false;
//
//        if (monte) {
//            auto fen = MonteCarlo::simulateGame(board, state);
//            std::cout << "FEN: \n" << fen << std::endl;
//
//        } else {
//            auto [eval, line] = timeEvaluation(board, state, 5);
//
//            std::cout << "Best Move(s) " << std::endl;
//            for (auto& move: line) {
//                Utils::printMove(move);
//            }
//
//            std::cout << "Table lookups: " << EngineMC:: lookups << std::endl;
//        }
    }
};

int main() {
    std::string fen, depth_str;
    std::getline(std::cin, fen);
    std::getline(std::cin, depth_str);
    int depth = static_cast<int>(std::strtol(depth_str.c_str(), nullptr, 10));

    PieceSteps::load();
//    Zobrist::init();

    if (fen == "startpos" || fen == "start") {
        std::cout << "Starting from Startposition" << std::endl;
        Utils::startingPositionAtDepth<Runner>(depth);
    } else {
        std::cout << "Running from custom position" << std::endl;
        Utils::loadFEN<Runner>(fen, depth);
    }

    return 0;
}

int mainMoveEnumeration(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << R"(Usage: ./Dory "<FEN>" <Depth>)" << std::endl;
        return 1;
    }

    std::string_view fen{argv[1]};
    int depth = static_cast<int>(std::strtol(argv[2], nullptr, 10));

    PieceSteps::load();

    if (fen == "startpos" || fen == "start") {
        Utils::startingPositionAtDepth<Runner>(depth);
    } else {
        Utils::loadFEN<Runner>(fen, depth);
    }

    return 0;
}
