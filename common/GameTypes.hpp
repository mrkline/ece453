#pragma once

#include <cstdint>

// The type of game being played
enum class GameType {
	/// A target flashes and players shoot it. Score is based on how quickly they shoot.
	POP_UP,
	/// Two targets pop up in succession and players shoot them in the order they popped up.
	/// Score is based on how quickly players hit both targets.
	FOLLOW_UP,
	/// A target flashes and players must shoot the target a given number of times.
	DUMP
};
