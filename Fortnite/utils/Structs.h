#pragma once

class UClass
{
public:
	BYTE _padding_0[0x40];
	UClass* SuperClass;
};

class UObject
{
public:
	PVOID VTableObject;
	DWORD ObjectFlags;
	DWORD InternalIndex;
	UClass* Class;
	BYTE _padding_0[0x8];
	UObject* Outer;

	inline BOOLEAN IsA(PVOID parentClass)
	{
		for (auto super = this->Class; super; super = super->SuperClass)
		{
			if (super == parentClass)
			{
				return TRUE;
			}
		}

		return FALSE;
	}
};

class FUObjectItem
{
public:
	UObject* Object;
	DWORD Flags;
	DWORD ClusterIndex;
	DWORD SerialNumber;
	DWORD SerialNumber2;
};

class TUObjectArray
{
public:
	FUObjectItem* Objects[9];
};

class GObjects
{
public:
	TUObjectArray* ObjectArray;
	BYTE _padding_0[0xC];
	DWORD ObjectCount;
};

template<class T>
struct TArray {
	friend struct FString;

public:
	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline INT Num() const
	{
		return Count;
	};

	inline T& operator[](INT i)
	{
		return Data[i];
	};

	inline BOOLEAN IsValidIndex(INT i)
	{
		return i < Num();
	}

private:
	T* Data;
	INT Count;
	INT Max;
};

struct FString : private TArray<WCHAR>
{
	FString()
	{
		Data = nullptr;
		Max = Count = 0;
	}

	FString(LPCWSTR other)
	{
		Max = Count = static_cast<INT>(wcslen(other));

		if (Count)
		{
			Data = const_cast<PWCHAR>(other);
		}
	};

	inline BOOLEAN IsValid()
	{
		return Data != nullptr;
	}

	inline PWCHAR c_str()
	{
		return Data;
	}

};

struct FName
{
	int32_t ComparisonIndex;
	int32_t Number;
};

class FTextData
{
public:
	char pad_0x0000[0x0028]; //0x0000
	wchar_t* Name; //0x0028 
	__int32 Length; //0x0030 
};

struct FText
{
	FTextData* Data;
	char UnknownData[0x10];

	wchar_t* Get() const
	{
		if (Data)
			return Data->Name;

		return (wchar_t*)XorString("???");
	}
};

typedef struct
{
	Vector3 Location;
	Vector3 Rotation;
	float FOV;
	float OrthoWidth;
	float OrthoNearClipPlane;
	float OrthoFarClipPlane;
	float AspectRatio;
} FMinimalViewInfo;

typedef struct
{
	float M[4][4];
} FMatrix;
typedef struct
{
	Vector3 ViewOrigin;
	char _padding_0[4];
	FMatrix ViewRotationMatrix;
	FMatrix ProjectionMatrix;
} FSceneViewProjectionData;

typedef struct _D3DMATRIX
{
	union
	{
		struct
		{
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		};
		float m[4][4];
	};
} D3DMATRIX;

struct FQuat
{
	float x;
	float y;
	float z;
	float w;
};

struct FTransform
{
	FQuat rot;
	Vector3 translation;
	char pad[4];
	Vector3 scale;
	char pad1[4];
	D3DMATRIX ToMatrixWithScale()
	{
		D3DMATRIX m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;

		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};


class AActor
{
public:
	char _pad1[0x0018];
	int ActorID;
	char _pad2[0x007C];
	float CustomTimeDilation;
	char _pad3[0x0094];
	class USceneComponent* RootComponent;

	int GetActorID()
	{
		if (!this) return 0;
		else return ActorID;
	}
	void Timer(float mult)
	{
		CustomTimeDilation = mult;
	}
	USceneComponent* GetRootComp()
	{
		if (!this) return nullptr;
		else return RootComponent;
	}
};

class player : public AActor
{
public:
	char _pad3[0x0108];
	class APlayerState* PlayerState;
	char _pad4[0x001C];
	Vector3 ControlInputVector;
	Vector3 LastControlInputVector;
	char _pad5[0x0004];
	class USkeletalMeshComponent* Mesh;

	APlayerState* GetPlayerState()
	{
		if (!this) return nullptr;
		else return PlayerState;
	}
	USkeletalMeshComponent* GetMesh()
	{
		if (!this) return nullptr;
		else return Mesh;
	}
	Vector3 GetMovement()
	{
		if (!this) return { 0, 0, 0 };
		else return ControlInputVector;
	}
	void SetMovement(Vector3 move)
	{
		if (!this) return;
		else ControlInputVector = move;
	}
};

class APlayerState;

class USkeletalMeshComponent
{
public:
	char _pad1[0x01C0];
	FTransform ComponentToWorld;

	FTransform GetComponentToWorld()
	{
		if (!this) return { NULL };
		else return ComponentToWorld;
	}
};

class UFortItemDefinition
{
public:
	char _pad1[0x0050];
	unsigned char Rarity;
	char _pad2[0x001F];
	FText DisplayName;

	char GetTier()
	{
		if (!this) return 0;
		else return Rarity;
	}
};

class USceneComponent
{
public:
	char _pad1[0x011C];
	Vector3 RelativeLocation;
	Vector3 RelativeRotation;
	char _pad2[0x000C];
	Vector3 ComponentVelocity;

	Vector3 GetPos()
	{
		if (!this) return Vector3(0, 0, 0);
		else return RelativeLocation;
	}
	Vector3 GetVelocity()
	{
		if (!this) return Vector3(0, 0, 0);
		else return ComponentVelocity;
	}
	void SetPos(Vector3 arg)
	{
		RelativeLocation = arg;
	}
	void SetVelocity(Vector3 arg)
	{
		ComponentVelocity = arg;
	}
};

class UWorld
{
public:
	char _pad1[0x0030];
	class ULevel* PersistentLevel;
	char _pad2[0x0110];
	TArray < class ULevel* > Levels;
	char _pad3[0x0038];
	class UGameInstance* OwningGameInstance;

	ULevel* GetMainLevel()
	{
		if (!this) return nullptr;
		else return PersistentLevel;
	}
	UGameInstance* GetGameInstance()
	{
		if (!this) return nullptr;
		else return OwningGameInstance;
	}
};

class ULevel
{
public:
	char _pad1[0x0098];
	TArray < AActor* > entityList;
};

class UGameInstance
{
public:
	char _pad1[0x0038];
	TArray < class ULocalPlayer* > LocalPlayers;

	ULocalPlayer* GetLocalPlayer()
	{
		if (!this) return nullptr;
		else return LocalPlayers[0];
	}
};

class ULocalPlayer
{
public:
	char _pad1[0x0030];
	class APlayerController* PlayerController;

	APlayerController* GetController()
	{
		if (!this) return nullptr;
		else return PlayerController;
	}
};

class APlayerController
{
public:
	char _pad1[0x02A0];
	class AActor* AcknowledgedPawn;

	AActor* GetPawn()
	{
		if (!this) return nullptr;
		else return AcknowledgedPawn;
	}
};