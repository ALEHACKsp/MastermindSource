#include <Psapi.h>

typedef struct LDR_DATA_TABLE_ENTRY_FIX
{
	PVOID Reserved1[2];
	LIST_ENTRY InMemoryOrderLinks;
	PVOID Reserved2[2];
	PVOID DllBase;
	PVOID Reserved3[1];
	ULONG64 SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	PVOID Reserved5[2];
#pragma warning(push)
#pragma warning(disable: 4201)
	union
	{
		ULONG CheckSum;
		PVOID Reserved6;
	} DUMMYUNIONNAME;
#pragma warning(pop)
	ULONG TimeDateStamp;
};

std::string TextFormat(const char* format, ...)
{
	va_list argptr;
	va_start(argptr, format);

	char buffer[2048];
	vsprintf(buffer, format, argptr);

	va_end(argptr);

	return buffer;
}

//FACE Utils v4
namespace FORTNITE_UTILS
{
	//Static Macro
	#define RVA(Instr, InstrSize) ((DWORD64)Instr + InstrSize + *(LONG*)((DWORD64)Instr + (InstrSize - sizeof(LONG))))
	#define ConstStrLen(Str) ((sizeof(Str) - sizeof(Str[0])) / sizeof(Str[0]))
	#define ToLower(Char) ((Char >= 'A' && Char <= 'Z') ? (Char + 32) : Char)

	#define ReadPointer(base, offset) (*(PVOID *)(((PBYTE)base + offset)))
	#define ReadDWORD(base, offset) (*(PDWORD)(((PBYTE)base + offset)))
	#define ReadBYTE(base, offset) (*(((PBYTE)base + offset)))

	//StrCompare (with StrInStrI(Two = false))
	template <typename StrType, typename StrType2>
	bool StrCmp(StrType Str, StrType2 InStr, bool Two)
	{
		if (!Str || !InStr) return false;
		wchar_t c1, c2; do
		{
			c1 = *Str++; c2 = *InStr++;
			c1 = ToLower(c1); c2 = ToLower(c2);
			if (!c1 && (Two ? !c2 : 1)) return true;
		} while (c1 == c2); return false;
	}

	//CRC16 StrHash	
	template <typename StrType> __declspec(noinline) constexpr unsigned short HashStr(StrType Data, int Len)
	{
		unsigned short CRC = 0xFFFF; while (Len--)
		{
			auto CurChar = *Data++; if (!CurChar) break;
			CRC ^= ToLower(CurChar) << 8; for (int i = 0; i < 8; i++)
				CRC = CRC & 0x8000 ? (CRC << 1) ^ 0x8408 : CRC << 1;
		} return CRC;
	}
	#define ConstHashStr(Str) [](){ constexpr unsigned short CRC = FORTNITE_UTILS::HashStr(Str, ConstStrLen(Str)); return CRC; }()

	//EncryptDecryptPointer
	template <typename PtrType>
	__forceinline PtrType EPtr(PtrType Ptr)
	{
		typedef union { struct { USHORT Key1; USHORT Key2; USHORT Key3; USHORT Key4; }; ULONG64 Key; } CryptData;
		CryptData Key{ ConstHashStr(__TIME__), ConstHashStr(__DATE__), ConstHashStr(__TIMESTAMP__), ConstHashStr(__TIMESTAMP__) };
		return (PtrType)((DWORD64)Ptr ^ Key.Key);
	}
	#define EPtr(Ptr) FORTNITE_UTILS::EPtr(Ptr)

	//GetModuleBase
	template <typename StrType>
	PBYTE GetModuleBase_Wrapper(StrType ModuleName)
	{
		PPEB_LDR_DATA Ldr = ((PTEB)__readgsqword(FIELD_OFFSET(NT_TIB, Self)))->ProcessEnvironmentBlock->Ldr; void* ModBase = nullptr;
		for (PLIST_ENTRY CurEnt = Ldr->InMemoryOrderModuleList.Flink; CurEnt != &Ldr->InMemoryOrderModuleList; CurEnt = CurEnt->Flink)
		{
			LDR_DATA_TABLE_ENTRY_FIX* pEntry = CONTAINING_RECORD(CurEnt, LDR_DATA_TABLE_ENTRY_FIX, InMemoryOrderLinks);
			if (!ModuleName || StrCmp(ModuleName, pEntry->BaseDllName.Buffer, false)) return (PBYTE)pEntry->DllBase;
		} return nullptr;
	}
	#define GetModuleBase FORTNITE_UTILS::GetModuleBase_Wrapper

	//Signature Scan
	BOOLEAN MaskCompare(PVOID buffer, LPCSTR pattern, LPCSTR mask)
	{
		for (auto b = reinterpret_cast<PBYTE>(buffer); *mask; ++pattern, ++mask, ++b) 
		{
			if (*mask == 'x' && *reinterpret_cast<LPCBYTE>(pattern) != *b)
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	PBYTE FindPattern_Wrapper(const char* Pattern, const char* Module = nullptr)
	{
		//find pattern utils
		#define InRange(x, a, b) (x >= a && x <= b) 
		#define GetBits(x) (InRange(x, '0', '9') ? (x - '0') : ((x - 'A') + 0xA))
		#define GetByte(x) ((BYTE)(GetBits(x[0]) << 4 | GetBits(x[1])))

		//get module range
		PBYTE ModuleStart = (PBYTE)GetModuleBase_Wrapper(Module); if (!ModuleStart) return nullptr;
		PIMAGE_NT_HEADERS NtHeader = ((PIMAGE_NT_HEADERS)(ModuleStart + ((PIMAGE_DOS_HEADER)ModuleStart)->e_lfanew));
		PBYTE ModuleEnd = (PBYTE)(ModuleStart + NtHeader->OptionalHeader.SizeOfImage - 0x1000); ModuleStart += 0x1000;

		//scan pattern main
		PBYTE FirstMatch = nullptr;
		const char* CurPatt = Pattern;
		for (; ModuleStart < ModuleEnd; ++ModuleStart)
		{
			bool SkipByte = (*CurPatt == '\?');
			if (SkipByte || *ModuleStart == GetByte(CurPatt))
			{
				if (!FirstMatch) FirstMatch = ModuleStart;
				SkipByte ? CurPatt += 2 : CurPatt += 3;
				if (CurPatt[-1] == 0) return FirstMatch;
			}

			else if (FirstMatch)
			{
				ModuleStart = FirstMatch;
				FirstMatch = nullptr; 
				CurPatt = Pattern;
			}
		}

		//MessageBoxA(0, "didnt find", Module, 0);
		return nullptr;
	}
	#define FindPattern FORTNITE_UTILS::FindPattern_Wrapper
	#define FindPattern2 FORTNITE_UTILS::FindPatterns

	//Spoof Return Address
	PVOID ReturnAddressStubEnc = 0;
	extern "C" void* _spoofer_stub();
	template<typename Ret = void, typename First = void*, typename Second = void*, typename Third = void*, typename Fourth = void*, typename... Stack>
	__forceinline Ret SpoofCall(void* Func, First a1 = First{}, Second a2 = Second{}, Third a3 = Third{}, Fourth a4 = Fourth{}, Stack... args)
	{
		struct shell_params { const void* a1; void* a2; void* a3; };
		shell_params CallCtx = { EPtr(ReturnAddressStubEnc), Func, nullptr };
		typedef Ret(*ShellFn)(First, Second, Third, Fourth, PVOID, PVOID, Stack...);
		return ((ShellFn)&FORTNITE_UTILS::_spoofer_stub)(a1, a2, a3, a4, &CallCtx, nullptr, args...);
	}
	__forceinline void SetSpoofStub(PVOID R15_Stub) { ReturnAddressStubEnc = EPtr(R15_Stub); }
	#define SpoofCall FORTNITE_UTILS::SpoofCall

	//Spoof VFTable Return Address
	template<typename Ret = void, typename... Args>
	__forceinline Ret VCall(PVOID Class, int Index, Args... AddArgs) {
		return SpoofCall<Ret>((*(PVOID**)Class)[Index], Class, AddArgs...);
	}
	#define VCall FORTNITE_UTILS::VCall

	//Spoof Call Export Return Address
	PVOID GetExportAddress(PBYTE hDll, const char* Name)
	{
		//process image data
		PIMAGE_NT_HEADERS NT_Head = (PIMAGE_NT_HEADERS)(hDll + ((PIMAGE_DOS_HEADER)hDll)->e_lfanew);
		PIMAGE_EXPORT_DIRECTORY ExportDir = (PIMAGE_EXPORT_DIRECTORY)(hDll + NT_Head->OptionalHeader.DataDirectory[0].VirtualAddress);

		//process list
		for (DWORD i = 0; i < ExportDir->NumberOfNames; i++)
		{
			//get ordinal & name
			USHORT Ordinal = ((USHORT*)(hDll + ExportDir->AddressOfNameOrdinals))[i];
			const char* ExpName = (const char*)hDll + ((DWORD*)(hDll + ExportDir->AddressOfNames))[i];
			if (StrCmp(Name, ExpName, true))
				return (PVOID)(hDll + ((DWORD*)(hDll + ExportDir->AddressOfFunctions))[Ordinal]);
		} return nullptr;
	}

	#define FC(Mod, Name, ...) [&](){ static PVOID FAddr = nullptr; \
		if (!FAddr) FAddr = EPtr(FORTNITE_UTILS::GetExportAddress(FORTNITE_UTILS::GetModuleBase_Wrapper(XorString(#Mod)), XorString(#Name))); \
		return SpoofCall<decltype(Name(__VA_ARGS__))>(EPtr(FAddr), __VA_ARGS__); \
	}()
}