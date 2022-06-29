//
// Created by Robin on 29.06.2022.
//

#include "board.h"

#ifndef CHESSENGINE_CHECKLOGICHANDLER_H
#define CHESSENGINE_CHECKLOGICHANDLER_H

class CheckLogicHandler {
    Board board;
    int kingSquare{};

    BB addPins(bool diagonal);

    [[nodiscard]] constexpr bool isAttacked(int index) const {
        return CHECK_BIT(attacked, index);
    }
public:
    bool isDoubleCheck{}, blockEP{};
    BB attacked{}, checkMask{}, pinsStraight{}, pinsDiagonal{};

    template<State state>
    void reload(Board& brd);

    [[nodiscard]] constexpr BB pruneEpPin(BB epPawns) const {
        return blockEP ? 0 : epPawns;
    }

    [[nodiscard]] constexpr bool isCheck() const {
        return isAttacked(kingSquare);
    }

    [[nodiscard]] constexpr BB getCheckMask() const {
        return isCheck() ? checkMask : FULL_BB;
    }
};

#endif //CHESSENGINE_CHECKLOGICHANDLER_H
