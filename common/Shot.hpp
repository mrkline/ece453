#pragma once

#include <memory>

#include <jsoncpp/json/json.h>

#include "Vector3.hpp"

typedef std::vector<Vector3> Movement;

/// Represents a shot with an idicator about whether or not it was hit
class Shot {
public:
	Shot(int8_t p, int8_t tar, int time);

	virtual Json::Value toJSON() const;

	static Shot fromJSON(const Json::Value& value);

	virtual bool operator==(const Shot& o) const;

	const int8_t player; ///< The ID of the player that took the shot
	const int8_t target; ///< The ID of the target that was hit. -1 if it was a miss
	const int time; ///< The timestamp of the shot, in milliseconds since the game started
};

class ShotWithMovement : public Shot {
public:
	ShotWithMovement(char p, char tar, int time, Movement&& m);

	Json::Value toJSON() const override;

	static ShotWithMovement fromJSON(const Json::Value& value);

	bool operator==(const Shot& o) const override;

	// A vector of vectors. Surely this will cause no confusion.
	const Movement movement; ///< Provides accelerometer data leading up to the shot
};

Movement movementFromJSON(const Json::Value& moves);

Json::Value movementToJSON(const Movement moves);
