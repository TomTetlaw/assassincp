#ifndef __MATHLIB_H__
#define __MATHLIB_H__

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_PI_F
#define M_PI_F ((float)(M_PI))
#endif

inline float sign(float x) {
	if (x < 0) {
		return -1;
	}
	else {
		return 1;
	}
}

inline float deg2rad(float x) {
	return x * (M_PI_F / 180.0f);
}

inline float rad2deg(float x) {
	return x * (180.0f / M_PI_F);
}

inline float approach(float current, float goal, float dt) {
	float dist = goal - current;
	if (dist > dt) {
		return current + dt;
	}
	else if (dist < -dt) {
		return current - dt;
	}
	else {
		return goal;
	}
}

inline bool within_tolerance(float x, float y, float tolerance) {
	if (x > y + tolerance) {
		return false;
	}
	else if (x < y - tolerance) {
		return false;
	}
	else {
		return true;
	}
}

template<typename T>
inline T clamp(T x, T min_value, T max_value) {
	if (x < min_value) {
		return min_value;
	}
	else if (x > max_value) {
		return max_value;
	}
	else {
		return x;
	}
}

inline float map_range(float x, float min1, float max1, float min2, float max2) {
	return (max2 - min2) * (x / (max1 - min1));
}

inline float random_float(float min_value, float max_value) {
	return min_value + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max_value - min_value)));
}

#define V2PARMS(v) v.x, v.y
struct Vec2 {
	float x = 0, y = 0;
	float *data = nullptr;

	Vec2() {
		data = &x;
		x = 0;
		y = 0;
	}

	Vec2(float x, float y) {
		this->x = x;
		this->y = y;
	}

	inline float distance_to(Vec2 &other) {
		Vec2 delta = *this - other;
		return delta.length();
	}

	inline float length() {
		return sqrtf(x*x + y*y);
	}

	inline float dot(Vec2 &other) {
		return x*other.x + y*other.y;
	}

	inline float determinant(Vec2 &other) {
		return x*other.y - y*other.x;
	}

	inline Vec2 normalized() {
		float len = length();
		return Vec2(x / len, y / len);
	}

	static Vec2 from_angle(float angle) {
		return Vec2(cos(angle), -sin(angle));
	}

	inline float angle_to(Vec2 other) {
		Vec2 delta = (other - *this).normalized();
		float a = atan2f(-delta.y, delta.x);
		//if (a < 0) {
		//	a += 2 * M_PI_F;
		//}
		return a;
	}

	inline void scale(float x, float y) {
		this->x *= x;
		this->y *= y;
	}

	inline Vec2 operator +(Vec2 other) {
		return Vec2(x + other.x, y + other.y);
	}
	inline Vec2 operator -(Vec2 other) {
		return Vec2(x - other.x, y - other.y);
	}
	inline Vec2 operator *(Vec2 other) {
		return Vec2(x * other.x, y * other.y);
	}
	inline Vec2 operator +(float other) {
		return Vec2(x + other, y + other);
	}
	inline Vec2 operator -(float other) {
		return Vec2(x - other, y - other);
	}
	inline Vec2 operator *(float other) {
		return Vec2(x * other, y * other);
	}
};

inline Vec2 approach(Vec2 current, Vec2 goal, float dt) {
	return Vec2(
		approach(current.x, goal.x, dt),
		approach(current.y, goal.y, dt)
	);
}

#define V4PARMS(v) v.x,v.y,v.z,v.w
struct Vec4 {
	float x = 0, y = 0, z = 0, w = 0;
	float *data = nullptr;

	Vec4() {
		data = &x;
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	Vec4(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	inline float distance_to(Vec4 &other) {
		Vec4 delta = *this - other;
		return delta.length();
	}

	inline float length() {
		return sqrtf(x*x + y*y + z*z + w*w);
	}

	inline Vec4 operator +(Vec4 other) {
		return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
	}
	inline Vec4 operator -(Vec4 other) {
		return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
	}
	inline Vec4 operator *(Vec4 other) {
		return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
	}
	inline Vec4 operator +(float other) {
		return Vec4(x + other, y + other, z + other, w + other);
	}
	inline Vec4 operator -(float other) {
		return Vec4(x - other, y - other, z - other, w - other);
	}
	inline Vec4 operator *(float other) {
		return Vec4(x * other, y * other, z * other, w * other);
	}
};

inline Vec4 approach(Vec4 current, Vec4 goal, float dt) {
	return Vec4(
		approach(current.x, goal.x, dt),
		approach(current.y, goal.y, dt),
		approach(current.z, goal.z, dt),
		approach(current.w, goal.w, dt)
	);
}

#define V3PARMS(v) v.x,v.y,v.z
struct Vec3 {
	float x = 0, y = 0, z = 0;
	float *data = nullptr;

	Vec3() {
		data = &x;
		x = 0;
		y = 0;
		z = 0;
	}

	Vec3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	inline float distance_to(Vec3 &other) {
		Vec3 delta = *this - other;
		return delta.length();
	}

	inline float length() {
		return sqrtf(x*x + y*y + z*z);
	}

	inline Vec3 normalize() {
		float len = length();
		return Vec3(x / len, y / len, z / len);
	}

	inline float dot(Vec3 &other) {
		return x*other.x + y*other.y + z*other.z;
	}

	inline Vec3 cross(Vec3 &other) {
		Vec3 out;
		out.x = y * other.z - z * other.y;
      	out.y = z * other.x - x * other.z;
    	out.z = x * other.y - y * other.x;
		return out;
	}

	inline Vec3 negate() {
		return Vec3(-x, -y, -z);
	}

	inline Vec3 operator +(Vec3 other) {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}
	inline Vec3 operator -(Vec3 other) {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}
	inline Vec3 operator *(Vec3 other) {
		return Vec3(x * other.x, y * other.y, z * other.z);
	}
	inline Vec3 operator +(float other) {
		return Vec3(x + other, y + other, z + other);
	}
	inline Vec3 operator -(float other) {
		return Vec3(x - other, y - other, z - other);
	}
	inline Vec3 operator *(float other) {
		return Vec3(x * other, y * other, z * other);
	}
};

inline Vec3 approach(Vec3 current, Vec3 goal, float dt) {
	return Vec3(
		approach(current.x, goal.x, dt),
		approach(current.y, goal.y, dt),
		approach(current.z, goal.z, dt)
	);
}

#endif
