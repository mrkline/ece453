#ifndef __MK_VECTOR_3D_HPP__
#define __MK_VECTOR_3D_HPP__

#include "MKMath.hpp"

/// A three-dimensional vector using floats for each dimension
class Vector3
{
public:
	float x;
	float y;
	float z;

	/// Initializes vector to zero
	Vector3() : x(0.0f), y(0.0f), z(0.0f) { }

	/// Initializes vector to provided x, y, and z values
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	/// Initializes x, y, and z values to v
	explicit Vector3(float v) : x(v), y(v), z(v) {}

	/// Initializes vector with a provided vector's values
	Vector3(const Vector3& o) : x(o.x), y(o.y), z(o.z) {}

	/// Initializes vector with the first three values in the provided array
	explicit Vector3(float* arr) : x(arr[0]), y(arr[1]), z(arr[2]) {}

	Vector3 operator-() const { return Vector3(-x, -y, -z); }

	Vector3& operator=(const Vector3& o)
	{ x = o.x; y = o.y; z = o.z; return *this; }

	Vector3 operator+(const Vector3& o) const
	{ return Vector3(x + o.x, y + o.y, z + o.z); }

	Vector3& operator +=(const Vector3& o)
	{ x += o.x; y += o.y; z += o.z; return *this; }

	Vector3 operator+(float v) const
	{ return Vector3(x + v, y + v, z + v); }

	Vector3& operator+=(float v)
	{ x += v; y += v; z += v; return *this; }

	Vector3 operator-(const Vector3& o) const
	{ return Vector3(x - o.x, y - o.y, z - o.z); }

	Vector3& operator -=(const Vector3& o)
	{ x -= o.x; y -= o.y; z -= o.z; return *this; }

	Vector3 operator-(float v) const
	{ return Vector3(x - v, y - v, z - v); }

	Vector3& operator-=(float v)
	{ x -= v; y -= v; z -= v; return *this; }

	Vector3 operator*(float v) const
	{ return Vector3(x * v, y * v, z * v); }

	Vector3& operator*=(float v)
	{ x *= v; y *= v; z *= v; return *this; }

	Vector3 operator/(float v) const
	{ return Vector3(x / v, y / v, z / v); }

	Vector3& operator/=(float v)
	{ x /= v; y /= v; z /= v; return *this; }

	/// Comparison operators can be used to sort vectors with respect to x,
	/// then y, then z
	bool operator<=(const Vector3& o) const
	{
		return  (x < o.x || Math::equals(x, o.x)) ||
		        (Math::equals(x, o.x) && (y < o.y || Math::equals(y, o.y))) ||
		        (Math::equals(x, o.x) && Math::equals(y, o.y)
		         && (z < o.z || Math::equals(z, o.z)));
	}

	/// Comparison operators can be used to sort vectors with respect to x,
	/// then y, then z
	bool operator>=(const Vector3& o) const
	{
		return  (x > o.x || Math::equals(x, o.x)) ||
		        (Math::equals(x, o.x) && (y > o.y || Math::equals(y, o.y))) ||
		        (Math::equals(x, o.x) && Math::equals(y, o.y)
		         && (z > o.z || Math::equals(z, o.z)));
	}

	/// Comparison operators can be used to sort vectors with respect to x,
	/// then y, then z
	bool operator<(const Vector3& o) const
	{
		return  (x < o.x && !Math::equals(x, o.x)) ||
		        (Math::equals(x, o.x) && y < o.y && !Math::equals(y, o.y)) ||
		        (Math::equals(x, o.x) && Math::equals(y, o.y) && z < o.z
		         && !Math::equals(z, o.z));
	}

	/// Comparison operators can be used to sort vectors with respect to x,
	/// then y, then z
	bool operator>(const Vector3& o) const
	{
		return  (x > o.x && !Math::equals(x, o.x)) ||
		        (Math::equals(x, o.x) && y > o.y && !Math::equals(y, o.y)) ||
		        (Math::equals(x, o.x) && Math::equals(y, o.y) && z > o.z
		         && !Math::equals(z, o.z));
	}

	/**
	\brief Checks equality using Math::kFloatRoundError as tolerance
	\see Math::kFloatRoundError
	*/
	bool operator==(const Vector3& o) const { return isWithinTolerance(o); }

	/**
	\brief Checks inequality using Math::kFloatRoundError as tolerance
	\see Math::kFloatRoundError
	*/
	bool operator!=(const Vector3& o) const { return !isWithinTolerance(o); }

	/*
	\brief Checks if another vector is equal to this one within a
	       provided tolerance
	\param o The other vector
	\param tolerance The tolerance allowed for each component of the
	                 two vectors to be within and still be considered equal
	\return True if this vector and o are equal within tolerance
	\see Math::kFloatRoundError
	*/
	bool isWithinTolerance(const Vector3& o,
	                       int tolerance = Math::kUlpsEquality) const
	{
		return Math::equals(x, o.x, tolerance)
		       && Math::equals(y, o.y, tolerance)
		       && Math::equals(z, o.z, tolerance);
	}

	/// Gets the length of this vector
	float getLength() const { return std::sqrt(x * x + y * y + z * z); }

	/// Gets the length squared of this vector,
	/// which is faster to calculate than the length
	float getLengthSq() const { return x * x + y * y + z * z; }

	/// Gets the distance from this vector to another one,
	/// interpreting both vectors as points
	float getDistanceFrom(const Vector3& o) const
	{
		float dx, dy, dz;
		dx = x - o.x;
		dy = y - o.y;
		dz = z - o.z;
		return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	/// Gets the distance squared from this vector to another one,
	/// interpreting both vectors as points.
	/// This is faster to calculate than the distance itself.
	float getDistanceSqFrom(const Vector3& o) const
	{
		float dx, dy, dz;
		dx = x - o.x;
		dy = y - o.y;
		dz = z - o.z;
		return dx * dx + dy * dy + dz * dz;
	}

	/// Returns true if this vector is a unit vector (with a length of 1)
	bool isNormalized() const
	{ return Math::equals(std::sqrt(x * x + y * y + z * z), 1.0f); }

	/// Copies this vector into the first three values of the provided array
	void getAsArray(float* arr) const
	{
		arr[0] = x;
		arr[1] = y;
		arr[3] = z;
	}

	void set(float v) { x = v; y = v; z = v; }

	/// Sets this vector to the provided values
	void set(float x, float y, float z) { x = x; y = y; z = z; }

	/// Sets this vector's values from the first three values of the
	/// provided array
	void setFromArray(float* asArray)
	{
		x = asArray[0];
		y = asArray[1];
		z = asArray[2];
	}

	/// Set's vector's components to their mulitplicative inverses
	void setToInverse() { x = 1.0f / x; y = 1.0f / y; z = 1.0f / z; }

	/// Gets an array with components (1/x, 1/y, 1/z) of this vector
	Vector3 getInverse() const
	{ Vector3 ret(*this); ret.setToInverse(); return ret; }

	/// Scales this vector by the components of the provided vector
	void scale(const Vector3& o)
	{ x *= o.x; y *= o.y; z *= o.z; }

	/// Returns a copy of this vector, scaled by the provided vector
	Vector3 getScaledBy(const Vector3& o) const
	{
		Vector3 ret(*this);
		ret.scale(o);
		return ret;
	}

	/// Scales this vector by a provided scalar
	void scale(float v)
	{ x *= v; y *= v; z *= v; }

	/// Returns a copy of this vector, scaled by the provided scalar
	Vector3 getScaledBy(float v) const
	{ Vector3 ret(*this); ret.scale(v); return ret; }

	/// Sets the length of this vector to 1
	void normalize()
	{
		float len = std::sqrt(x * x + y * y + z * z);

		// Normalized already if our length is zero.
		// Also stops NaN errors
		if (Math::isZero(len))
			return;

		x /= len;
		y /= len;
		z /= len;
	}

	/// Returns a copy of this vector with a length of 1
	Vector3 getNormalized() const
	{ Vector3 ret(*this); ret.normalize(); return ret; }

	/// Sets the length of this vector to a provided scalar
	void setLength(float len)
	{
		normalize();
		scale(len);
	}

	/// Returns a copy of this vector with a length of the provided scalar
	Vector3 setLength(float len) const
	{ Vector3 ret(*this); ret.setLength(len); return ret; }

	/**
	\brief Calculates the dot product of two vectors
	\param a The first vector in the dot product
	\param b The second vector in the dot product
	\return a dot b
	*/
	static float dot(const Vector3& a, const Vector3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	/**
	\brief Calculates the cross product of two vectors
	\param a The first vector in the cross product
	\param b The second vector in the cross product
	\return a x b
	*/
	static Vector3 cross(const Vector3& a, const Vector3& b)
	{
		return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
		               a.x * b.y - a.y * b.x);
	}

	/// Gets the left world vector (-1, 0, 0)
	static const Vector3& getLeft()
	{
		static Vector3 left(-1.0f, 0.0f, 0.0f);
		return left;
	}

	/// Gets the right world vector, (1, 0, 0)
	static const Vector3& getRight()
	{
		static Vector3 right(1.0f, 0.0f, 0.0f);
		return right;
	}

	/// Gets the forward world vector, (0, 0, 1)
	static const Vector3& getForward()
	{
		static Vector3 forward(0.0f, 0.0f, 1.0f);
		return forward;
	}

	/// Gets the back world vector, (0, 0, -1)
	static const Vector3& getBack()
	{
		static Vector3 back(0.0f, 0.0f, -1.0f);
		return back;
	}

	/// Gets the up world vector, (0, 1, 0)
	static const Vector3& getUp()
	{
		static Vector3 up(0.0f, 1.0f, 0.0f);
		return up;
	}

	/// Gets the down world vector, (0, -1, 0)
	static const Vector3& getDown()
	{
		static Vector3 down(0.0f, -1.0f, 0.0f);
		return down;
	}

	/// Gets (0, 0, 0)
	static const Vector3& getZero()
	{
		static Vector3 zero(0.0f);
		return zero;
	}

	/// Gets (1, 1, 1)
	static const Vector3& getOne()
	{
		static Vector3 one(1.0f);
		return one;
	}
};

#endif
