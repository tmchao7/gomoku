#ifndef GOMOKU_PLAYER_H
#define GOMOKU_PLAYER_H

#include "Gomoku/Types.h"

#include <string>

namespace gomoku {

class Player {
public:
    Player();
    Player(std::string name, Stone stone);

    const std::string& name() const;
    Stone stone() const;

private:
    std::string name_;
    Stone stone_;
};

} // namespace gomoku

#endif // GOMOKU_PLAYER_H
