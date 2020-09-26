class Vector3
{
public:
	float x, y, z;

	inline Vector3()
	{
		x = y = z = 0.0f;
	}

	inline Vector3(float X, float Y, float Z)
	{
		x = X; y = Y; z = Z;
	}

	inline float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	inline Vector3& operator+=(float v)
	{
		x += v; y += v; z += v; return *this;
	}

	inline Vector3& operator-=(float v)
	{
		x -= v; y -= v; z -= v; return *this;
	}

	inline Vector3& operator-=(const Vector3& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	inline Vector3 operator*(float v) const
	{
		return Vector3(x * v, y * v, z * v);
	}

	inline Vector3 operator/(float v) const
	{
		return Vector3(x / v, y / v, z / v);
	}

	inline Vector3& operator+=(const Vector3& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	inline Vector3 operator-(const Vector3& v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	inline Vector3 operator+(const Vector3& v) const
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	inline Vector3& operator/=(float v)
	{
		x /= v; y /= v; z /= v; return *this;
	}

	inline bool Zero() const
	{
		return (x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f && z > -0.1f && z < 0.1f);
	}
};

class Vector2
{
public:
	float x, y;

	inline Vector2()
	{
		x = y = 0.0f;
	}

	inline Vector2(float X, float Y)
	{
		x = X; y = Y;
	}

	inline Vector2 operator+(float v) const
	{
		return Vector2(x + v, y + v);
	}

	inline Vector2 operator-(float v) const
	{
		return Vector2(x - v, y - v);
	}

	inline Vector2& operator+=(float v)
	{
		x += v; y += v; return *this;
	}

	inline Vector2& operator*=(float v)
	{
		x *= v; y *= v; return *this;
	}

	inline Vector2& operator/=(float v)
	{
		x /= v; y /= v; return *this;
	}

	inline Vector2 operator-(const Vector2& v) const
	{
		return Vector2(x - v.x, y - v.y);
	}

	inline Vector2 operator+(const Vector2 & v) const
	{
		return Vector2(x + v.x, y + v.y);
	}

	inline Vector2& operator+=(const Vector2& v)
	{
		x += v.x; y += v.y; return *this;
	}

	inline Vector2& operator-=(const Vector2& v)
	{
		x -= v.x; y -= v.y; return *this;
	}

	inline Vector2 operator/(float v) const
	{
		return Vector2(x / v, y / v);
	}

	inline Vector2 operator*(float v) const
	{
		return Vector2(x * v, y * v);
	}

	inline Vector2 operator/(const Vector2& v) const
	{
		return Vector2(x / v.x, y / v.y);
	}

	inline bool Zero() const
	{
		return (x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f);
	}
};

class Matrix3x4
{
public:
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
		};

		float m[3][4];
		float mm[12];
	};
};

class Matrix4x4
{
public:
	float m[4][4];
};

namespace Math
{
	#define M_PI 3.14159265358979323846f
	#define M_RADPI	57.295779513082f
	#define M_PI_F ((float)(M_PI))
	#define RAD2DEG(x) ((float)(x) * (float)(180.f / M_PI_F))
	#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.f))

	#define sinf(a) ((float)FC(ntdll, sin, (double)(a)))
	#define cosf(a) ((float)FC(ntdll, cos, (double)(a)))
	#define tanf(a) ((float)FC(ntdll, tan, (double)(a)))
	#define atanf(a) ((float)FC(ntdll, atan, (double)(a)))
	#define powf(a, b) ((a) * (a))

	__forceinline float FastSQRT(float x)
	{
		union { int i; float x; } u;
		u.x = x; u.i = (u.i >> 1) + 0x1FC00000;
		u.x = u.x + x / u.x;
		return .25f * u.x + x / u.x;
	}

	__forceinline float FastDist2D(const Vector2& Src, const Vector2& Dst)
	{
		return FastSQRT(powf(Src.x - Dst.x, 2.f) + powf(Src.y - Dst.y, 2.f));
	}
	__forceinline float FastDist2DVec3(const Vector2& Src, const Vector3& Dst)
	{
		return FastSQRT(powf(Src.x - Dst.x, 2.f) + powf(Src.y - Dst.y, 2.f));
	}

	__forceinline float FastDist3D(const Vector3& Src, const Vector3& Dst)
	{
		return FastSQRT(powf(Src.x - Dst.x, 2.f) + powf(Src.y - Dst.y, 2.f) + powf(Src.z - Dst.z, 2.f));
	}

	__forceinline float GameDist(const Vector3& Src, const Vector3& Dst)
	{
		return FastDist3D(Src, Dst) / 100.f;
	}

	__forceinline float Vec3Length(const Vector3& Src)
	{
		return FastSQRT(powf(Src.x, 2.f) + powf(Src.y, 2.f) + powf(Src.z, 2.f));
	}

	__forceinline float Vec2Length(const Vector2& Src)
	{
		return FastSQRT(powf(Src.x, 2.f) + powf(Src.y, 2.f));
	}

	__forceinline void ClampAngle(Vector3& Ang)
	{
		if (Ang.x < -89.f) Ang.x = -89.f;
		if (Ang.x > 89.f) Ang.x = 89.f;
		while (Ang.y < -180.f) Ang.y += 360.f;
		while (Ang.y > 180.f) Ang.y -= 360.f;
		Ang.z = 0.f;
	}

	float Normalize(float angle)
	{
		float a = (float)fmod(fmod(angle, 360.0) + 360.0, 360.0);
		if (a > 180.0f)
		{
			a -= 360.0f;
		}
		return a;
	}

	__forceinline Vector3 CalcAngle(const Vector3& Src, const Vector3& Dst)
	{
		Vector3 Delta = Src - Dst, AimAngles;
		float Hyp = FastSQRT(powf(Delta.x, 2.f) + powf(Delta.y, 2.f));
		AimAngles.y = atanf(Delta.y / Delta.x) * M_RADPI;
		AimAngles.x = (atanf(Delta.z / Hyp) * M_RADPI) * -1.f;
		if (Delta.x >= 0.f) AimAngles.y += 180.f;
		AimAngles.z = 0.f; return AimAngles;
	}

	__forceinline Vector3 CalcAngleInput(const Vector3& Src, const Vector3& Dst, Vector3 rot)
	{
		Vector3 Delta = Src - Dst, AimAngles;
		float Hyp = FastSQRT(powf(Delta.x, 2.f) + powf(Delta.y, 2.f));
		AimAngles.y = atanf(Delta.y / Delta.x) * M_RADPI;
		AimAngles.x = (atanf(Delta.z / Hyp) * M_RADPI) * -1.f;
		if (Delta.x >= 0.f) AimAngles.y += 180.f;
		AimAngles.z = 0.f; return AimAngles - rot;
	}
	
	__forceinline void VectorNormalize(Vector3& Src)
	{
		float l = Vec3Length(Src);
		if (l != 0.0f)
		{
			Src /= l;
		}
		else
		{
			Src.x = Src.y = 0.0f; Src.z = 1.0f;
		}
	}

	__forceinline float Dot(const Vector3& v1, const Vector3& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}
}