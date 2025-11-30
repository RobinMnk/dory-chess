//
// Created by Robin on 13.07.2024.
//

#include <string>
#include <iostream>

#include "dory.h"

class UciManager {
    enum UciStatus{ IDLE = 0, NEW_GAME, READY, RUNNING };
    UciStatus status{IDLE};

    Dory::Board board{};
    std::unique_ptr<Dory::Engine> dory{std::make_unique<Dory::Engine>()};
    bool whiteToMove{true};

    void respond(std::string_view resp) {
        std::cout << resp << std::endl;
    }

    void processCommand(std::string_view cmd) {
        if(cmd == "uci") {
            respond("id name Dory Engine");
            respond("id author Robin");
            respond("uciok");
            return;
        }
        else if(cmd == "ucinewgame") {
            status = NEW_GAME;
            return;
        }
        else if(cmd == "isready") {
            respond("readyok");
            return;
        }

        std::stringstream stream(cmd.data());
        std::string segment;
        std::vector<std::string> seglist;
        while(std::getline(stream, segment, ' ')) seglist.push_back(segment);


        if(seglist.at(0) == "ping") {
            std::cout << "pong " << seglist.at(1) << std::endl;
            return;
        }

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

            while(ix < seglist.size()) {
                Dory::Move move = Dory::Utils::parseMove(board, whiteToMove, seglist.at(ix));
                board.makeMove(move, whiteToMove);
                whiteToMove = !whiteToMove;
                ++ix;
            }
            status = READY;
        }
        else if (seglist.at(0) == "go") {
            status = RUNNING;
            auto [eval, line] = dory->searchTime(board, 1000, whiteToMove);
            std::cout << "bestmove " << Dory::Utils::moveFullNotation(line.back()) << std::endl;
            status = READY;
        }
    }

public:
    void run() {
        std::ios::sync_with_stdio(false);
        std::cin.tie(nullptr);

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