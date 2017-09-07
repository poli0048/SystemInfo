#pragma once

#include <string>
#include <vector>
#include <list>
#include <tuple>
#include <sstream>
#include <ctime>
#include <bitset>
#include <array>
#include <cstdint>
#include <iomanip>

#if defined(_WIN32)
	#include <Windows.h>
	#include <intrin.h>
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/param.h>
	#if defined(BSD)
		#include <sys/sysctl.h>
	#endif
#else
	#error "Unable to define GetInstalledMemorySizeBytes( ) for an unknown OS."
#endif

#ifndef SYSTEMINFO_NS
	#define SYSTEMINFO_NS SystemInfo
#endif

namespace SYSTEMINFO_NS {
	
	struct CPUCapabilities;

	static uint64_t        GetInstalledMemorySizeBytes(); // Get the size of installed RAM available to the OS, in bytes.
	static CPUCapabilities GetCPUCapabilities();          // Get a structure describing the capabilities/intrinsics of the CPU.
	static std::string     GetSystemInfoString(); //Get a string with formatted, human-readable text showing info on the running system.

	struct CPUCapabilities 
	{
		std::string Vendor;
		std::string Brand;
		
		// Old SIMD Extentions
		bool MMX;    //(Deployed around 1997)
		bool MMXEXT; //Extended MMX (Deployed around 1999)
		
		// SSE SIMD extensions common to both Intel and AMD
		bool SSE;    
		bool SSE2;   
		bool SSE3;
		bool SSSE3;  
		bool SSE41;  
		bool SSE42;
		
		// Non-Standard Extentions Introduced around the same time as later-revision SSE4
		bool POPCNT;  // Population count
		bool LZCNT;   // Leading Zero Count
		
		// Mainstream AVX - Advanced Vector Extentions (Follow-on to SSE)
		bool AVX;
		bool AVX2;
		
		// Specialty AVX - Generally supported only on modern high-end and specialized CPUs
		bool AVX512F;  // AVX-512 Foundation
		bool AVX512PF; // AVX-512 Pre-Fetch Instructions
		bool AVX512ER; // AVX-512 Exponential and Reciprocal Instructions
		bool AVX512CD; // AVX-512 Conflict Detection Instructions
		
		// AMD-only SSE extensions (Introduced in AMD Barcelona Architecture)
		bool SSE4a;
		
		// AMD-only 3DNow Extentions (Competed with original SSE - never supported by Intel and dropped by AMD in new processors)
		bool _3DNOW;    // (Deployed around 1998)
		bool _3DNOWEXT;
		
		// Additional Extentions:
		bool PCLMULQDQ;  // Carry-less Multiplication (Deployed around 2010)
		bool FMA;        // Fused Multiply-Add (Probably refers to FMA3) (Deployed around 2013)
		bool CMPXCHG16B; // Compare and Exchange - 16 Bytes (Deployed around 2003)
		bool AES;        // Advanced Encryption Standard Instruction Set (Deployed around 2011)
		bool SHA;        // Secure Hash Algorithm Extentions (Deployed around 2016)
		
		bool MONITOR;    bool MOVBE;       bool XSAVE;    bool OSXSAVE;
		bool F16C;       bool RDRAND;      bool MSR;      bool CX8;      
		bool SEP;        bool CMOV;        bool CLFSH;    bool FXSR;      
		bool FSGSBASE;   bool BMI1;        bool HLE;      bool BMI2;
		bool ERMS;       bool INVPCID;     bool RTM;      bool RDSEED;
		bool ADX;        bool PREFETCHWT1; bool LAHF;     bool ABM;
		bool XOP;        bool TBM;         bool SYSCALL;  bool RDTSCP;
		
	};
}


//************************************************************************************************************************************************************************
//*************************************************************                Implementation                *************************************************************
//************************************************************************************************************************************************************************

/// Get the size of installed RAM available to the OS, in bytes.
/// This function is very platform and environment-specific
inline uint64_t SYSTEMINFO_NS::GetInstalledMemorySizeBytes() 
{
	#if defined(_WIN32) && (defined(__CYGWIN__) || defined(__CYGWIN32__))
		// Cygwin under Windows. ------------------------------------
		// New 64-bit MEMORYSTATUSEX isn't available.  Use old 32.bit
		MEMORYSTATUS status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatus( &status );
		return (uint64_t)status.dwTotalPhys;

	#elif defined(_WIN32)
		// Windows. -------------------------------------------------
		// Use new 64-bit MEMORYSTATUSEX, not old 32-bit MEMORYSTATUS
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(status);
		GlobalMemoryStatusEx( &status );
		return (uint64_t)status.ullTotalPhys;

	#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
		// UNIX variants. -------------------------------------------
		// Prefer sysctl() over sysconf() except sysctl() HW_REALMEM and HW_PHYSMEM

	#if defined(CTL_HW) && (defined(HW_MEMSIZE) || defined(HW_PHYSMEM64))
		int mib[2];
		mib[0] = CTL_HW;
	#if defined(HW_MEMSIZE)
		mib[1] = HW_MEMSIZE;    // OSX. ---------------------
	#elif defined(HW_PHYSMEM64)
		mib[1] = HW_PHYSMEM64;  // NetBSD, OpenBSD. ---------
	#endif
		int64_t size = 0;       // 64-bit
		size_t len = sizeof( size );
		if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 )
			return (uint64_t)size;
		return (uint64_t)0U;    // Failed?

	#elif defined(_SC_AIX_REALMEM)
		// AIX. -----------------------------------------------------
		return (uint64_t)sysconf( _SC_AIX_REALMEM ) * (uint64_t)1024U;

	#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
		// FreeBSD, Linux, OpenBSD, and Solaris. --------------------
		return (uint64_t)sysconf( _SC_PHYS_PAGES ) * (uint64_t)sysconf( _SC_PAGESIZE );

	#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGE_SIZE)
		// Legacy. --------------------------------------------------
		return (uint64_t)sysconf( _SC_PHYS_PAGES ) * (uint64_t)sysconf( _SC_PAGE_SIZE );

	#elif defined(CTL_HW) && (defined(HW_PHYSMEM) || defined(HW_REALMEM))
		// DragonFly BSD, FreeBSD, NetBSD, OpenBSD, and OSX. --------
		int mib[2];
		mib[0] = CTL_HW;
	#if defined(HW_REALMEM)
		mib[1] = HW_REALMEM;    // FreeBSD. -----------------
	#elif defined(HW_PYSMEM)
		mib[1] = HW_PHYSMEM;    // Others. ------------------
	#endif
		unsigned int size = 0;  // 32-bit
		size_t len = sizeof( size );
		if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 )
			return (uint64_t)size;
		return (uint64_t)0U;    // Failed?
	#endif // sysctl and sysconf variants

	#else
		return (uint64_t)0U;    // Unknown OS.
	#endif
}

static inline void cpuid(int output[4], int functionnumber) 
{  
	#if defined (_MSC_VER) || defined (__INTEL_COMPILER)    // Microsoft or Intel compiler, intrin.h included
		__cpuidex(output, functionnumber, 0);               // intrinsic function for CPUID

	#elif defined(__GNUC__) || defined(__clang__)           // use inline assembly, Gnu/AT&T syntax
		int a, b, c, d;
		__asm("cpuid" : "=a"(a),"=b"(b),"=c"(c),"=d"(d) : "a"(functionnumber),"c"(0) : );
		output[0] = a;
		output[1] = b;
		output[2] = c;
		output[3] = d;

	#else                                                   // unknown platform. try inline assembly with masm/intel syntax
		__asm {
			mov eax, functionnumber
			xor ecx, ecx
			cpuid;
			mov esi, output
			mov [esi],    eax
			mov [esi+4],  ebx
			mov [esi+8],  ecx
			mov [esi+12], edx
		}
	#endif
}

static inline SYSTEMINFO_NS::CPUCapabilities SYSTEMINFO_NS::GetCPUCapabilities() 
{
	int nIds;
	int nExIds;
	std::string vendorString;
	std::string brandString;
	bool isIntel = false;
	bool isAMD = false;
	std::bitset<32> f_1_ECX_;
	std::bitset<32> f_1_EDX_;
	std::bitset<32> f_7_EBX_;
	std::bitset<32> f_7_ECX_;
	std::bitset<32> f_81_ECX_;
	std::bitset<32> f_81_EDX_;
	std::vector<std::array<int, 4>> data_;
	std::vector<std::array<int, 4>> extdata_;
	
	//Use CPUID to populate the register bitsets
	{
		std::array<int, 4> cpui;  

		// Calling cpuid with 0x0 as the function_id argument gets the number of the highest valid function ID.  
		cpuid(cpui.data(), 0);  
		nIds = cpui[0];  

		for (int i = 0; i <= nIds; ++i) 
		{  
			cpuid(cpui.data(), i);
			data_.push_back(cpui);
		}  

		// Capture vendor string  
		char vendor[0x20];
		memset(vendor, 0, sizeof(vendor));
		*reinterpret_cast<int*>(vendor)     = data_[0][1];
		*reinterpret_cast<int*>(vendor + 4) = data_[0][3];
		*reinterpret_cast<int*>(vendor + 8) = data_[0][2];
		vendorString = vendor;
		if (vendorString == "GenuineIntel")
			isIntel = true;
		else if (vendorString == "AuthenticAMD")
			isAMD = true;

		// load bitset with flags for function 0x00000001  
		if (nIds >= 1) 
		{
			f_1_ECX_ = data_[1][2];
			f_1_EDX_ = data_[1][3];
		}

		// load bitset with flags for function 0x00000007  
		if (nIds >= 7) 
		{  
			f_7_EBX_ = data_[7][1];  
			f_7_ECX_ = data_[7][2];  
		}  

		// Calling cpuid with 0x80000000 as the function_id argument gets the number of the highest valid extended ID.  
		cpuid(cpui.data(), 0x80000000);  
		nExIds = cpui[0];  

		char brand[0x40];  
		memset(brand, 0, sizeof(brand));  

		for (int i = 0x80000000; i <= nExIds; ++i) 
		{
			cpuid(cpui.data(), i);
			extdata_.push_back(cpui);
		}

		// load bitset with flags for function 0x80000001  
		if ((unsigned int) nExIds >= 0x80000001)
		{  
			f_81_ECX_ = extdata_[1][2];
			f_81_EDX_ = extdata_[1][3];
		}  

		// Interpret CPU brand string if reported  
		if ((unsigned int) nExIds >= 0x80000004) 
		{  
			memcpy(brand, extdata_[2].data(), sizeof(cpui));
			memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
			memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
			brandString = brand;
		}
	}
	
	// Interperet the register bitsets and populate a CPUCapabilities struct
	CPUCapabilities CPU;
	{
		CPU.Vendor      = vendorString;
		CPU.Brand       = brandString;
		
		CPU.SSE3        = f_1_ECX_[0];
		CPU.PCLMULQDQ   = f_1_ECX_[1];
		CPU.MONITOR     = f_1_ECX_[3];
		CPU.SSSE3       = f_1_ECX_[9];
		CPU.FMA         = f_1_ECX_[12];
		CPU.CMPXCHG16B  = f_1_ECX_[13];
		CPU.SSE41       = f_1_ECX_[19];
		CPU.SSE42       = f_1_ECX_[20];
		CPU.MOVBE       = f_1_ECX_[22];
		CPU.POPCNT      = f_1_ECX_[23];
		CPU.AES         = f_1_ECX_[25];
		CPU.XSAVE       = f_1_ECX_[26];
		CPU.OSXSAVE     = f_1_ECX_[27];
		CPU.AVX         = f_1_ECX_[28];
		CPU.F16C        = f_1_ECX_[29];
		CPU.RDRAND      = f_1_ECX_[30];
		
		CPU.MSR         = f_1_EDX_[5];
		CPU.CX8         = f_1_EDX_[8];
		CPU.SEP         = f_1_EDX_[11];
		CPU.CMOV        = f_1_EDX_[15];
		CPU.CLFSH       = f_1_EDX_[19];
		CPU.MMX         = f_1_EDX_[23];
		CPU.FXSR        = f_1_EDX_[24];
		CPU.SSE         = f_1_EDX_[25];
		CPU.SSE2        = f_1_EDX_[26];
		
		CPU.FSGSBASE    = f_7_EBX_[0];
		CPU.BMI1        = f_7_EBX_[3];
		CPU.HLE         = isIntel && f_7_EBX_[4];
		CPU.AVX2        = f_7_EBX_[5];
		CPU.BMI2        = f_7_EBX_[8];
		CPU.ERMS        = f_7_EBX_[9];
		CPU.INVPCID     = f_7_EBX_[10];
		CPU.RTM         = isIntel && f_7_EBX_[11];
		CPU.AVX512F     = f_7_EBX_[16];
		CPU.RDSEED      = f_7_EBX_[18];
		CPU.ADX         = f_7_EBX_[19];
		CPU.AVX512PF    = f_7_EBX_[26];
		CPU.AVX512ER    = f_7_EBX_[27];
		CPU.AVX512CD    = f_7_EBX_[28];
		CPU.SHA         = f_7_EBX_[29];
		
		CPU.PREFETCHWT1 = f_7_ECX_[0];
		
		CPU.LAHF        = f_81_ECX_[0];
		CPU.LZCNT       = isIntel && f_81_ECX_[5];
		CPU.ABM         = isAMD   && f_81_ECX_[5];
		CPU.SSE4a       = isAMD   && f_81_ECX_[6];
		CPU.XOP         = isAMD   && f_81_ECX_[11];
		CPU.TBM         = isAMD   && f_81_ECX_[21];
		
		CPU.SYSCALL     = isAMD   && f_81_EDX_[11];
		CPU.MMXEXT      = isAMD   && f_81_EDX_[22];
		CPU.RDTSCP      = isIntel && f_81_EDX_[27];
		CPU._3DNOWEXT   = isAMD   && f_81_EDX_[30];
		CPU._3DNOW      = isAMD   && f_81_EDX_[31];
	}
	
	return CPU;
}

// Take a vector of strings and return a camma-separated list of the items (as a string)
static inline std::string vectorOfStringsToCSL(std::vector<std::string> VecOfStrings) 
{
	std::string CSL;
	if (! VecOfStrings.empty()) 
	{
		for (size_t n = 0U; n + 1U < VecOfStrings.size(); n++)
			CSL += VecOfStrings[n] + ", ";
		CSL += VecOfStrings.back();
	}
	return CSL;
}

// Get a string with formatted, human-readable text showing info on the running system
static inline std::string SYSTEMINFO_NS::GetSystemInfoString() 
{
	uint64_t InstalledRAM_Bytes = SYSTEMINFO_NS::GetInstalledMemorySizeBytes();
	uint64_t InstalledRAM_MiB = InstalledRAM_Bytes / (uint64_t) 1024U / (uint64_t) 1024U;
	double InstalledRAM_GiB = (double) InstalledRAM_MiB / 1024.0;
	SYSTEMINFO_NS::CPUCapabilities CPU = SYSTEMINFO_NS::GetCPUCapabilities();
	
	std::ostringstream strStream;
	strStream << std::fixed << std::setprecision(1) << InstalledRAM_GiB;
	std::string RAMStr = strStream.str();
	
	std::string InfoStr("**************************   General Info:   ***************************\r\n");
	InfoStr += "CPU Vendor:       " + CPU.Vendor + "\r\n";
	InfoStr += "CPU Brand/Model:  " + CPU.Brand + "\r\n";
	InfoStr += "Installed Memory: " + RAMStr + " GiB\r\n\r\n";
	
	InfoStr +=          "************************   CPU SIMD Extensions:   ***********************\r\n";
	InfoStr += "MMX: " + std::string(CPU.MMX ? "Yes" : "No") + "\r\n";
	
	// Check SSE Support
	InfoStr += "SSE: ";
	if (CPU.SSE || CPU.SSE2 || CPU.SSE3 || CPU.SSSE3 || CPU.SSE41 || CPU.SSE42) 
	{
		if (CPU.SSE && CPU.SSE2 && CPU.SSE3 && CPU.SSSE3 && CPU.SSE41 && CPU.SSE42)
			InfoStr += "Yes (All versions through SSE 4.2)\r\n";
		else if (CPU.SSE && CPU.SSE2 && CPU.SSE3 && CPU.SSSE3 && CPU.SSE41 && (!CPU.SSE42))
			InfoStr += "Yes (Only through SSE 4.1; SSE 4.2 Not supported)\r\n";
		else if (CPU.SSE && CPU.SSE2 && CPU.SSE3 && CPU.SSSE3 && (!CPU.SSE41) && (!CPU.SSE42))
			InfoStr += "Yes (Only through SSSE3; SSE 4.1/4.2 Not supported)\r\n";
		else if (CPU.SSE && CPU.SSE2 && CPU.SSE3 && (!CPU.SSSE3) && (!CPU.SSE41) && (!CPU.SSE42))
			InfoStr += "Yes (Only through SSE3; SSSE3 and SSE 4.1/4.2 Not supported)\r\n";
		else if (CPU.SSE && CPU.SSE2 && (!CPU.SSE3) && (!CPU.SSSE3) && (!CPU.SSE41) && (!CPU.SSE42))
			InfoStr += "Yes (Only through SSE2; SSE3, SSSE3, and SSE 4.1/4.2 Not supported)\r\n";
		else if (CPU.SSE && (!CPU.SSE2) && (!CPU.SSE3) && (!CPU.SSSE3) && (!CPU.SSE41) && (!CPU.SSE42))
			InfoStr += "Yes (Only original SSE; SSE2, SSE3, SSSE3, and SSE 4.1/4.2 Not supported)\r\n";
		else {
			std::vector<std::string> supported;
			std::vector<std::string> unsupported;
			
			if (CPU.SSE)   supported  .push_back(std::string("SSE"));
			else           unsupported.push_back(std::string("SSE"));
			if (CPU.SSE2)  supported  .push_back(std::string("SSE2"));
			else           unsupported.push_back(std::string("SSE2"));
			if (CPU.SSE3)  supported  .push_back(std::string("SSE3"));
			else           unsupported.push_back(std::string("SSE3"));
			if (CPU.SSSE3) supported  .push_back(std::string("SSSE3"));
			else           unsupported.push_back(std::string("SSSE3"));
			if (CPU.SSE41) supported  .push_back(std::string("SSE4.1"));
			else           unsupported.push_back(std::string("SSE4.1"));
			if (CPU.SSE42) supported  .push_back(std::string("SSE4.2"));
			else           unsupported.push_back(std::string("SSE4.2"));
			
			InfoStr += "Yes (Unusual configuration: " + vectorOfStringsToCSL(supported) + " supported; " + vectorOfStringsToCSL(unsupported) + " not supported)\r\n";
		}
	}
	else
		InfoStr += "No\r\n";
	
	// Check AVX Support
	InfoStr += "AVX: ";
	if (CPU.AVX || CPU.AVX2 || CPU.AVX512F) 
	{
		if (CPU.AVX && CPU.AVX2 && CPU.AVX512F)
			InfoStr += "Yes (AVX, AVX2, and AVX-512 Foundation)\r\n";
		else if (CPU.AVX && CPU.AVX2 && (!CPU.AVX512F))
			InfoStr += "Yes (AVX and AVX2 only; AVX-512 not supported)\r\n";
		else if (CPU.AVX && (!CPU.AVX2) && (!CPU.AVX512F))
			InfoStr += "Yes (Original AVX only; AVX2 and AVX-512 not supported)\r\n";
		else if ((!CPU.AVX) && CPU.AVX2 && (!CPU.AVX512F))
			InfoStr += "Yes (Unusual configuration: AVX2 supported, but not AVX or AVX-512)\r\n";
		else if ((!CPU.AVX) && (!CPU.AVX2) && CPU.AVX512F)
			InfoStr += "Yes (Unusual configuration: AVX-512 Foundation supported, but not AVX or AVX2)\r\n";
		else if ((!CPU.AVX) && CPU.AVX2 && CPU.AVX512F)
			InfoStr += "Yes (Unusual configuration: AVX2 and AVX-512 Foundation supported, but not AVX)\r\n";
		else if (CPU.AVX && (!CPU.AVX2) && CPU.AVX512F)
			InfoStr += "Yes (Unusual configuration: AVX and AVX-512 Foundation supported, but not AVX2)\r\n";
		else
			InfoStr += "Yes (Unhandled Configuration)\r\n";
	}
	else
		InfoStr += "No\r\n";
	if (CPU.AVX512F) 
	{
		InfoStr += "AVX-512 Pre-Fetch Instructions -------------------: " + std::string(CPU.AVX512PF ? "Yes" : "No") + "\r\n";
		InfoStr += "AVX-512 Exponential and Reciprocal Instructions --: " + std::string(CPU.AVX512ER ? "Yes" : "No") + "\r\n";
		InfoStr += "AVX-512 Conflict Detection Instructions ----------: " + std::string(CPU.AVX512CD ? "Yes" : "No") + "\r\n";
	}
	
	InfoStr += "\r\n";
	InfoStr +=          "*****   CPU Special Binary Instructions (introduced around SSE4):   *****\r\n";
	InfoStr += "POPCNT (Population Count):  " + std::string(CPU.POPCNT ? "Yes" : "No") + "\r\n";
	InfoStr += "LZCNT (Leading Zero Count): " + std::string(CPU.LZCNT ? "Yes" : "No")  + "\r\n";
	
	InfoStr += "\r\n";
	InfoStr +=          "********************   Vendor-Specific Extentions:   ********************\r\n";
	std::vector<std::string> supported;
	std::vector<std::string> unsupported;
	
	// Check AMD-only extensions first
	if (CPU.MMXEXT)    supported.push_back(std::string("Extended MMX"));
	else             unsupported.push_back(std::string("Extended MMX"));
	if (CPU.SSE4a)     supported.push_back(std::string("SSE4a"));
	else             unsupported.push_back(std::string("SSE4a"));
	if (CPU._3DNOW)    supported.push_back(std::string("3DNow!"));
	else             unsupported.push_back(std::string("3DNow!"));
	if (CPU._3DNOWEXT) supported.push_back(std::string("Extended 3DNow!"));
	else             unsupported.push_back(std::string("Extended 3DNow!"));
	if (CPU.ABM)       supported.push_back(std::string("ABM"));
	else             unsupported.push_back(std::string("ABM"));
	if (CPU.XOP)       supported.push_back(std::string("XOP"));
	else             unsupported.push_back(std::string("XOP"));
	if (CPU.TBM)       supported.push_back(std::string("TBM"));
	else             unsupported.push_back(std::string("TBM"));
	if (CPU.SYSCALL)   supported.push_back(std::string("SYSCALL"));
	else             unsupported.push_back(std::string("SYSCALL"));
	
	InfoStr += "Supported AMD-Only:   " + vectorOfStringsToCSL(supported) + "\r\n";
	InfoStr += "Unsupported AMD-Only: " + vectorOfStringsToCSL(unsupported) + "\r\n\r\n";
	
	supported.clear();
	unsupported.clear();
	
	// Check Intel-only extensions next
	if (CPU.SEP)     supported.push_back(std::string("SEP (SYSENTER/SYSEXIT)"));
	else           unsupported.push_back(std::string("SEP (SYSENTER/SYSEXIT)"));
	if (CPU.RDTSCP)  supported.push_back(std::string("RDTSCP"));
	else           unsupported.push_back(std::string("RDTSCP"));
	if (CPU.RTM)     supported.push_back(std::string("RTM"));
	else           unsupported.push_back(std::string("RTM"));
	if (CPU.HLE)     supported.push_back(std::string("HLE"));
	else           unsupported.push_back(std::string("HLE"));
	
	InfoStr += "Supported Intel-Only:   " + vectorOfStringsToCSL(supported) + "\r\n";
	InfoStr += "Unsupported Intel-Only: " + vectorOfStringsToCSL(unsupported) + "\r\n\r\n";
	
	
	InfoStr +=          "***********************   Additional Extentions:   **********************\r\n";
	InfoStr += "PCLMULQDQ:  "  + std::string(CPU.PCLMULQDQ   ? "Yes" : "No ") + "                 ";
	InfoStr += "PREFETCHWT1: " + std::string(CPU.PREFETCHWT1 ? "Yes" : "No ") + "\r\n";
	
	InfoStr += "CMPXCHG16B: "  + std::string(CPU.CMPXCHG16B  ? "Yes" : "No ") + "                 ";
	InfoStr += "MONITOR:     " + std::string(CPU.MONITOR     ? "Yes" : "No ") + "\r\n";
	
	InfoStr += "XSAVE:      "  + std::string(CPU.XSAVE       ? "Yes" : "No ") + "                 ";
	InfoStr += "OSXSAVE:     " + std::string(CPU.OSXSAVE     ? "Yes" : "No ") + "\r\n";
	
	InfoStr += "FSGSBASE:   "  + std::string(CPU.FSGSBASE    ? "Yes" : "No ") + "                 ";
	InfoStr += "INVPCID:     " + std::string(CPU.INVPCID     ? "Yes" : "No ") + "\r\n";
	
	InfoStr += "RDRAND:     "  + std::string(CPU.RDRAND      ? "Yes" : "No ") + "                 ";
	InfoStr += "RDSEED:      " + std::string(CPU.RDSEED      ? "Yes" : "No ") + "\r\n";
	InfoStr += "\r\n";
	
	InfoStr += "AES:   " + std::string(CPU.AES   ? "Yes" : "No ") + "         ";
	InfoStr += "SHA:   " + std::string(CPU.SHA   ? "Yes" : "No ") + "         ";
	InfoStr += "FMA:   " + std::string(CPU.FMA   ? "Yes" : "No ") + "\r\n";
	
	InfoStr += "MOVBE: " + std::string(CPU.MOVBE ? "Yes" : "No ") + "         ";
	InfoStr += "F16C:  " + std::string(CPU.F16C  ? "Yes" : "No ") + "         ";
	InfoStr += "MSR:   " + std::string(CPU.MSR   ? "Yes" : "No ") + "\r\n";
	
	InfoStr += "CX8:   " + std::string(CPU.CX8   ? "Yes" : "No ") + "         ";
	InfoStr += "LAHF:  " + std::string(CPU.LAHF  ? "Yes" : "No ") + "         ";
	InfoStr += "CMOV:  " + std::string(CPU.CMOV  ? "Yes" : "No ") + "\r\n";
	
	InfoStr += "CLFSH: " + std::string(CPU.CLFSH ? "Yes" : "No ") + "         ";
	InfoStr += "FXSR:  " + std::string(CPU.FXSR  ? "Yes" : "No ") + "         ";
	InfoStr += "BMI1:  " + std::string(CPU.BMI1  ? "Yes" : "No ") + "\r\n";
	
	InfoStr += "BMI2:  " + std::string(CPU.BMI2  ? "Yes" : "No ") + "         ";
	InfoStr += "ADX:   " + std::string(CPU.ADX   ? "Yes" : "No ") + "         ";
	InfoStr += "ERMS:  " + std::string(CPU.ERMS  ? "Yes" : "No ") + "\r\n";
	
	return InfoStr;
}

