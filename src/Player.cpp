#include "Gomoku/Player.h"

#include <utility>

namespace gomoku {

Player::Player()
    : name_("Player"), stone_(Stone::Empty) {
}

Player::Player(std::string name, Stone stone)
    : name_(std::move(name)), stone_(stone) {
    if (name_.empty()) {
        name_ = "Player";
    }
}

const std::string& Player::name() const {
    return name_;
}

Stone Player::stone() const {
    return stone_;
}

} // namespace gomoku
