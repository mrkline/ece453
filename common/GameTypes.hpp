#pragma once

#include <cstdint>

/// The type of game being played
enum class GameType {
	/// A target flashes and players shoot it. Score is based on how quickly they shoot.
	POP_UP,
	/// Two targets pop up in succession and players shoot them in the order they popped up.
	/// Score is based on how quickly players hit both targets.
	FOLLOW_UP,
	/// A target flashes and players must shoot the target a given number of times.
	DUMP
};

/// Message IDs are 16-bits wide and unsigned
typedef uint16_t message_id_t;
/// Player and target ID are 8-bits wide
typedef int8_t board_id_t;
/// Shots are 16 bits wide
typedef int16_t shot_t;
/// Scores are 16 bits wide
typedef int16_t score_t;
/// Timestamps are in milliseconds and are 32 bits wide
typedef int32_t timestamp_t;
/// Game durations are in seconds and are 16 bits wide
typedef int16_t duration_t;
