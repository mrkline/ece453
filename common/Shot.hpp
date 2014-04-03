#pragma once

#include <memory>

#include <jsoncpp/json/json.h>

#include "Vector3.hpp"

/// Represents a shot with an idicator about whether or not it was hit
class Shot {
public:
	Shot(char p, char tar, int time);

	virtual Json::Value toJSON() const;

	static Shot fromJSON(const Json::Value& value);

	virtual bool operator==(const Shot& o) const;

	const char player; ///< The ID of the player that took the shot
	const char target; ///< The ID of the target that was hit. -1 if it was a miss
	const int time; ///< The timestamp of the shot, in milliseconds since the game started
};

class ShotWithMovement : public Shot {
public:
	ShotWithMovement(char p, char tar, int time, std::vector<Vector3>&& m);

	Json::Value toJSON() const override;

	static ShotWithMovement fromJSON(const Json::Value& value);

	bool operator==(const Shot& o) const override;

	// A vector of vectors. Surely this will cause no confusion.
	const std::vector<Vector3> movement; ///< Provides accelerometer data leading up to the shot
};
