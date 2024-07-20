//
// Created by Robin on 13.07.2024.
//

#include <string>
#include <iostream>

#include "dory.h"

class UciManager {
    enum UciStatus{ IDLE = 0, NEW_GAME, READY, RUNNING };
    UciStatus status{IDLE};

    Dory::Board board;
    bool whiteToMove{true};

    void respond(std::string_view resp) {
        std::cout << resp << std::endl;
    }

    void processCommand(std::string_view cmd) {
        if(cmd == "uci") respond("uciok");
        else if(cmd == "ucinewgame") { Dory::initialize(); status = NEW_GAME; }
        else if(cmd == "isready") respond("readyok");

        std::stringstream stream(cmd.data());
        std::string segment;
        std::vector<std::string> seglist;
        while(std::getline(stream, segment, ' ')) seglist.push_back(segment);

        if(seglist.at(0) == "position") {
            if(seglist.at(1) == "startpos") { board = Dory::STARTBOARD; whiteToMove = true; }
            else {
                auto [b, w] = Dory::Utils::parseFEN(seglist, 2);
                board = b;
                whiteToMove = w;
            }

            unsigned int ix = 2;
            while(ix < seglist.size() && seglist.at(ix) != "moves") ix++;
            ix++;

            if(seglist.size() > ix) {
                while(ix < seglist.size()) {
                    Dory::Move move = Dory::Utils::parseMove(board, whiteToMove, seglist.at(ix));
                    board.makeMove(move, whiteToMove);
                    whiteToMove = !whiteToMove;
                    ++ix;
                }
            }
            status = READY;
        }
        else if (seglist.at(0) == "go") {
            status = RUNNING;
            auto [eval, line] = Dory::searchDepth(board, 6, whiteToMove);
            std::cout << "bestmove " << Dory::Utils::moveFullNotation(line.back()) << std::endl;
            status = READY;
        }
    }

public:
    void run() {
        std::string cmd;
        while(cmd != "quit") {
            std::getline(std::cin, cmd, '\n');
            if(!cmd.empty()) processCommand(cmd);
        }
    }
};

int main() {
    UciManager uci;
    uci.run();
}