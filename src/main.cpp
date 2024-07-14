#include <iostream>

#include "engine/search.h"
#include "utils/fenreader.h"
#include "core/movecollectors.h"

using Dory::Board, Dory::Search::Searcher;

//void monteCarlo(const Board& board, const State state, int depth) {
//    auto fen = MonteCarlo::simulateGame(board, state, depth);
//    std::cout << "FEN: \n" << fen << std::endl;
//}
//
//void monteCarloTreeSearch(const Board& board, const State state, [[maybe_unused]] int depth) {
//    GameTree gt{board, state};
//    ChildrenData best{};
//    for(int i = 0; i < 12000; i++) {
//        gt.run();
////        ChildrenData candidate = *std::max_element(gt.root->children.begin(), gt.root->children.end(), [](auto& a, auto& b) {return a.score > b.score;});
////        if (candidate.move != best.move) {
////            best = candidate;
////            std::cout << Utils::moveNameNotation(best.move) << ":   " << best.node->wins << " / " << best.node->total << "  (" << best.score << ")" << std::endl;
////        }
//    }
//
////    for(auto& cd: gt.root->children) {
////        std::cout << Utils::moveNameNotation(cd.move) << ":   " << cd.node->wins << " / " << cd.node->total << "  (" << cd.score << ")" << std::endl;
////    }
////    std::cout << "\n" << std::endl;
//
//    best = *std::max_element(gt.root->children.begin(), gt.root->children.end(), [](auto& a, auto& b) {return a.score < b.score;});
//
//    std::cout << "\n\n" << Utils::moveNameNotation(best.move) << ":   " << best.node->wins << " / " << best.node->total << "  (" << best.score << ")" << std::endl;
//
////    std::sort(gt.root->children.begin(), gt.root->children.end(), [](auto& a, auto& b) {return a.score > b.score;});
////
////    for(auto& cd: gt.root->children) {
////        std::cout << Utils::moveNameNotation(cd.move) << ":   " << cd.node->wins << " / " << cd.node->total << "  (" << cd.score << ")" << std::endl;
////    }
//
//}

template<bool whiteToMove>
void timeEvaluation(const Board& board, int depth) {
    Searcher::reset();
    auto t1 = std::chrono::high_resolution_clock::now();
    /// -----------------------------------------------------------
//    auto [eval, line] = Searcher::searchDepth(board, state, depth);
    auto [eval, line] = Searcher::template iterativeDeepening<whiteToMove>(board, depth);
//    monteCarlo(board, state, depth);
//    MonteCarlo::runSimulations(board, state, depth, 10);

//    monteCarloTreeSearch(board, state, depth);


//    MoveListGenerator::countLegalMoves(board, state);
//    std::cout << static_cast<int>(MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Pawn)) << " Pawn Moves" << std::endl;
//    std::cout << static_cast<int>(MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Knight)) << " Knight Moves" << std::endl;
//    std::cout << static_cast<int>(MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Bishop)) << " Bishop Moves" << std::endl;
//    std::cout << static_cast<int>(MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Rook)) << " Rook Moves" << std::endl;
//    std::cout << static_cast<int>(MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::Queen)) << " Queen Moves" << std::endl;
//    std::cout << static_cast<int>(MoveGenerator<MoveListGenerator, false, true>::numberOfMovesByPiece.at(Piece::King)) << " King Moves" << std::endl;
//    engine_params::EngineParams params;
//    std::cout << features::mobility(board, state, params);
    /// -----------------------------------------------------------
    auto t2 = std::chrono::high_resolution_clock::now();

    auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);

    std::cout << "\n" << std::endl;
    Dory::Utils::printLine(line, eval);

//    std::cout << "Evaluation: " << eval << std::endl;

    std::chrono::duration<double> seconds = t2 - t1;

    std::cout << "Searched " << Searcher::nodesSearched << " nodes in " << ms_int.count() << "ms";

    double knps = (static_cast<double>(Searcher::nodesSearched) / 1000) / seconds.count();
    if (knps < 1000) {
        std::cout << "\t\t(" << knps << " k nps)\n\n";
    } else {
        std::cout << "\t\t(" << (knps / 1000) << " M nps)\n\n";
    }

//    return {eval, line};
}

template<bool whiteToMove, int depth>
void enumerateMoves(const Board& board) {
    Dory::MoveCollectors::nodes.resize(depth + 1);

    auto t1 = std::chrono::high_resolution_clock::now();
    Dory::MoveCollectors::LimitedDFS<depth>::template generateGameTree<whiteToMove>(board);
    auto t2 = std::chrono::high_resolution_clock::now();

    auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);

    std::chrono::duration<double> seconds = t2 - t1;

    unsigned long long nodes = Dory::MoveCollectors::LimitedDFS<1>::totalNodes;

    std::cout << "Generated " <<  nodes << " nodes in " << ms_int.count() << "ms";

    double knps = (static_cast<double>(nodes) / 1000) / seconds.count();
    if (knps < 1000) {
        std::cout << "\t\t(" << knps << " k nps)\n\n";
    } else {
        std::cout << "\t\t(" << (knps / 1000) << " M nps)\n\n";
    }
}

int main() {
    std::string command, fen, depth_str, num_lines_str;
    std::getline(std::cin, command);
    std::getline(std::cin, fen);
    std::getline(std::cin, depth_str);
    int depth = static_cast<int>(std::strtol(depth_str.c_str(), nullptr, 10));

    const auto [board, whiteToMove] = Dory::Utils::parseFEN(fen);
    Dory::PieceSteps::load();
    Dory::Zobrist::init(23984729);

    if (command == "perft") {
        if(whiteToMove) enumerateMoves<true, 5>(board);
        else enumerateMoves<false, 5>(board);
        return 0;
    } else if (command == "eval") {
        int eval;
        if(whiteToMove) eval = Dory::Evaluation::evaluatePosition<true>(board);
        else eval = Dory::Evaluation::evaluatePosition<false>(board);
        std::cout << "Static Eval: " << eval << std::endl;
        return 0;
    } else if (command == "zobrist") {
        size_t zobrist;
        if(whiteToMove) zobrist = Dory::Zobrist::hash<true>(board);
        else zobrist = Dory::Zobrist::hash<false>(board);
        std::cout << "Zobrist hash: " << zobrist << std::endl;
        return 0;
    }

    std::getline(std::cin, num_lines_str);
    auto num_lines = static_cast<unsigned int>(std::strtol(num_lines_str.c_str(), nullptr, 10));

    Dory::Search::NUM_LINES = num_lines;

    if(whiteToMove) timeEvaluation<true>(board, depth);
    else timeEvaluation<false>(board, depth);



//    auto [eval, line] = Searcher::searchDepth(board, state, depth);
//    Utils::printLine(line, eval);


//    auto [eval, line] = Searcher::quiescenceSearch(board, state, 0, -INF, INF);
//    Utils::printLine(line, eval);

//    monteCarlo(board, state, 2);
//    engine_params::EngineParams params;
//
//    Utils::print_board(board);
//    constexpr bool whiteToMove = true;
//
//    std::cout << bitCount(board.pawns<whiteToMove>()) << " Pawns. " << bitCount(board.knights<whiteToMove>()) << " Knights. " <<
//        bitCount(board.bishops<whiteToMove>())  << " Bishops. " << bitCount(board.rooks<whiteToMove>())  << " Rooks. " <<
//        bitCount(board.queens<whiteToMove>())  << " Queens" << std::endl;
//
//                                               int res = Evaluation::evaluatePosition(board, state);
//    std::cout << res << std::endl;
//    std::cout << features::material<true>(board, params) << std::endl;
//    std::cout << features::material<false>(board, params) << std::endl;


    std::cout << "\nTable lookups:\t" << Searcher::trTable.lookups << std::endl;
    std::cout << "Table size:\t" << Searcher::trTable.size() << " kB" << std::endl;
    std::cout << "Searched " << Searcher::nodesSearched << " nodes";

    return 0;
}