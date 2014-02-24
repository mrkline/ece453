#pragma once

#include <cstdint>

// The type of game being played
enum GameType {
	/// A target flashes and players shoot it. Score is based on how quickly they shoot.
	GT_POP_UP,
	/// Two targets pop up in succession and players shoot them in the order they popped up.
	/// Score is based on how quickly players hit both targets.
	GT_FOLLOW_UP,
	/// A target flashes and players must shoot the target a given number of times.
	GT_DUMP
};

enum WinCondition : uint8_t {
	WC_TIME = 0x01, ///< End the game after a given amount of time.
	WC_POINTS = 0x03 ///< End the game after a user has scored a given number of points.
};
