#pragma once

#include <memory>

#ifdef WITH_JSON
#include <jsoncpp/json/json.h>
#endif

#include "GameTypes.hpp"
#include "Vector3.hpp"

typedef std::vector<Vector3> Movement;

/// Represents a shot with an idicator about whether or not it was hit
class Shot {
public:
	Shot(board_id_t p, board_id_t tar, timestamp_t time);

	Shot(const Shot&) = default;

#ifdef WITH_JSON
	virtual Json::Value toJSON() const;

	static Shot fromJSON(const Json::Value& value);
#endif

	virtual bool operator==(const Shot& o) const;

	// Allow us to sort via time
	bool operator<(const Shot& o) const { return time < o.time; }

	board_id_t player; ///< The ID of the player that took the shot
	board_id_t target; ///< The ID of the target that was hit. -1 if it was a miss
	timestamp_t time; ///< The timestamp of the shot, in milliseconds since the game started
};

namespace std {

/// Hash function for a Shot
template<>
struct hash<Shot> {
	size_t operator()(const Shot& s) const
	{
		return (size_t)s.time
			^ ((size_t)s.target << ((sizeof(size_t) - 1) * 8))
			^ ((size_t)s.player << ((sizeof(size_t) - 2) * 8));
	}
};

} // end namespace std

class ShotWithMovement : public Shot {
public:
	ShotWithMovement(board_id_t p, board_id_t tar, timestamp_t time, Movement&& m);

	ShotWithMovement(const Shot&, Movement&& m);

	ShotWithMovement(const ShotWithMovement&) = default;

	ShotWithMovement(ShotWithMovement&& o);

#ifdef WITH_JSON
	Json::Value toJSON() const override;

	static ShotWithMovement fromJSON(const Json::Value& value);
#endif

	bool operator==(const Shot& o) const override;

	ShotWithMovement& operator=(ShotWithMovement&&) = default;

	ShotWithMovement& operator=(const ShotWithMovement&) = default;

	// A vector of vectors. Surely this will cause no confusion.
	Movement movement; ///< Provides accelerometer data leading up to the shot
};

#ifdef WITH_JSON
Movement movementFromJSON(const Json::Value& moves);

Json::Value movementToJSON(const Movement moves);
#endif
