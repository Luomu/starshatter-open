/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2005. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         ExceptionHandler.cpp
	AUTHOR:       John DiCamillo

*/


#include <windows.h>
#include <imagehlp.h>

extern void Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

class ExceptionHandler
{
public:
	ExceptionHandler();
	~ExceptionHandler();

private:
	static LPTOP_LEVEL_EXCEPTION_FILTER old_filter;

	static LONG WINAPI   ExceptionFilter(EXCEPTION_POINTERS* info);

	static void          PrintReport(EXCEPTION_POINTERS* info);

	// Helper functions
	static const char*   GetExceptionString(DWORD dwCode);
	static BOOL          GetLogicalAddress(VOID* addr, char* module, int len,
	DWORD& section, DWORD& offset);

	static BOOL          InitImageHelp();
	static void          ImageStackTrace(CONTEXT* context);
	static void          IntelStackTrace(CONTEXT* context);


	// Make typedefs for some IMAGEHLP.DLL functions so that we can use them
	// with GetProcAddress
	typedef BOOL (__stdcall * SYMINITIALIZEPROC)(HANDLE, LPSTR, BOOL);
	typedef BOOL (__stdcall * SYMCLEANUPPROC)(HANDLE);

	typedef LPVOID (__stdcall *SYMFUNCTIONTABLEACCESSPROC)(HANDLE, DWORD);
	typedef DWORD (__stdcall *SYMGETMODULEBASEPROC)(HANDLE, DWORD);
	typedef BOOL (__stdcall *SYMGETSYMFROMADDRPROC)(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL);

	typedef BOOL (__stdcall * STACKWALKPROC)(DWORD,
	HANDLE,
	HANDLE, 
	LPSTACKFRAME,
	LPVOID,
	PREAD_PROCESS_MEMORY_ROUTINE,
	PFUNCTION_TABLE_ACCESS_ROUTINE,
	PGET_MODULE_BASE_ROUTINE,
	PTRANSLATE_ADDRESS_ROUTINE);

	static SYMINITIALIZEPROC            SymInitialize;
	static SYMCLEANUPPROC               SymCleanup;
	static STACKWALKPROC                StackTrace;
	static SYMFUNCTIONTABLEACCESSPROC   SymFunctionTableAccess;
	static SYMGETMODULEBASEPROC         SymGetModuleBase;
	static SYMGETSYMFROMADDRPROC        SymGetSymFromAddr;
};

// +--------------------------------------------------------------------+

LPTOP_LEVEL_EXCEPTION_FILTER ExceptionHandler::old_filter = 0;

ExceptionHandler::SYMINITIALIZEPROC ExceptionHandler::SymInitialize = 0;
ExceptionHandler::SYMCLEANUPPROC    ExceptionHandler::SymCleanup = 0;
ExceptionHandler::STACKWALKPROC     ExceptionHandler::StackTrace = 0;

ExceptionHandler::SYMFUNCTIONTABLEACCESSPROC
ExceptionHandler::SymFunctionTableAccess = 0;

ExceptionHandler::SYMGETMODULEBASEPROC
ExceptionHandler::SymGetModuleBase = 0;

ExceptionHandler::SYMGETSYMFROMADDRPROC
ExceptionHandler::SymGetSymFromAddr = 0;

ExceptionHandler global_exception_handler;


// +--------------------------------------------------------------------+

ExceptionHandler::ExceptionHandler()
{
	old_filter = SetUnhandledExceptionFilter(ExceptionFilter);
}

ExceptionHandler::~ExceptionHandler()
{
	SetUnhandledExceptionFilter(old_filter);
}

// +--------------------------------------------------------------------+

static bool in_filter = false;

LONG WINAPI
ExceptionHandler::ExceptionFilter(EXCEPTION_POINTERS* info)
{
	if (in_filter) {
		Print("\n\n*********************************************\n");
		Print("SECOND EXCEPTION CAUGHT: TERMINATING.\n");
		Print("*********************************************\n");
	}

	else {
		in_filter = true;
		PrintReport(info);
		in_filter = false;
	}

	if (old_filter)
	return old_filter(info);
	else
	return EXCEPTION_CONTINUE_SEARCH;
}

// +--------------------------------------------------------------------+

void
ExceptionHandler::PrintReport(EXCEPTION_POINTERS* info)
{
	EXCEPTION_RECORD* record  = info->ExceptionRecord;
	CONTEXT*          context = info->ContextRecord;
	DWORD             code    = record->ExceptionCode;

	Print("\n*********************************************\n");
	Print("FATAL EXCEPTION:\n");

	Print("\nRegisters:\n");
	Print("EAX:    %08x\n", context->Eax);
	Print("EBX:    %08x\n", context->Ebx);
	Print("ECX:    %08x\n", context->Ecx);
	Print("EDX:    %08x\n", context->Edx);
	Print("EDI:    %08x\n", context->Edi);
	Print("ESI:    %08x\n", context->Esi);
	Print("EBP:    %08x\n", context->Ebp);
	Print("\n");
	Print("CS:EIP: %04x:%08x\n", context->SegCs, context->Eip);
	Print("SS:ESP: %04x:%08x\n", context->SegSs, context->Esp);
	Print("DS:     %04x\n", context->SegDs);
	Print("ES:     %04x\n", context->SegEs);
	Print("FS:     %04x\n", context->SegFs);
	Print("GS:     %04x\n", context->SegGs);
	Print("Flags:  %08x\n", context->EFlags );
	Print("\n");

	Print("Exception Code:  %08x %s\n",code, GetExceptionString(code));
	Print("Exception Addr:  %08x \n",  record->ExceptionAddress);

	if (code == EXCEPTION_ACCESS_VIOLATION && record->NumberParameters >= 2) {
		if (record->ExceptionInformation[0])
		Print("                 Program attempted to WRITE to address 0x%08x\n", record->ExceptionInformation[1]);
		else
		Print("                 Program attempted to READ from address 0x%08x\n", record->ExceptionInformation[1]);
	}

	if (InitImageHelp()) {
		ImageStackTrace(context);
		SymCleanup(GetCurrentProcess());
	}
	else {
		IntelStackTrace(context);
	}

	Print("\n*********************************************\nPROGRAM TERMINATED.\n");
}

// +--------------------------------------------------------------------+

const char*
ExceptionHandler::GetExceptionString(DWORD code)
{
#define EXCEPTION( x ) case EXCEPTION_##x: return #x;

	switch (code) {
		EXCEPTION( ACCESS_VIOLATION )
		EXCEPTION( DATATYPE_MISALIGNMENT )
		EXCEPTION( BREAKPOINT )
		EXCEPTION( SINGLE_STEP )
		EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
		EXCEPTION( FLT_DENORMAL_OPERAND )
		EXCEPTION( FLT_DIVIDE_BY_ZERO )
		EXCEPTION( FLT_INEXACT_RESULT )
		EXCEPTION( FLT_INVALID_OPERATION )
		EXCEPTION( FLT_OVERFLOW )
		EXCEPTION( FLT_STACK_CHECK )
		EXCEPTION( FLT_UNDERFLOW )
		EXCEPTION( INT_DIVIDE_BY_ZERO )
		EXCEPTION( INT_OVERFLOW )
		EXCEPTION( PRIV_INSTRUCTION )
		EXCEPTION( IN_PAGE_ERROR )
		EXCEPTION( ILLEGAL_INSTRUCTION )
		EXCEPTION( NONCONTINUABLE_EXCEPTION )
		EXCEPTION( STACK_OVERFLOW )
		EXCEPTION( INVALID_DISPOSITION )
		EXCEPTION( GUARD_PAGE )
		EXCEPTION( INVALID_HANDLE )
	}

	static char buffer[512] = { 0 };

	FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
	GetModuleHandle("NTDLL.DLL"),
	code, 0, buffer, sizeof(buffer), 0 );

	return buffer;
}

// +--------------------------------------------------------------------+

BOOL
ExceptionHandler::GetLogicalAddress(void* addr, char* mod_name, int len, DWORD& section, DWORD& offset)
{
	MEMORY_BASIC_INFORMATION mbi;

	if (!VirtualQuery(addr, &mbi, sizeof(mbi)))
	return FALSE;

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if (!GetModuleFileName((HMODULE)hMod, mod_name, len))
	return FALSE;

	PIMAGE_DOS_HEADER       pDosHdr  = (PIMAGE_DOS_HEADER) hMod;
	PIMAGE_NT_HEADERS       pNtHdr   = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);
	PIMAGE_SECTION_HEADER   pSection = IMAGE_FIRST_SECTION( pNtHdr );

	DWORD rva = (DWORD)addr - hMod; // RVA is offset from module load address

	// Iterate through the section table, looking for the one that encompasses
	// the linear address.
	for (unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++ ) {
		DWORD sectionStart = pSection->VirtualAddress;
		DWORD sectionEnd = sectionStart
		+ max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

		// Is the address in this section???
		if ((rva >= sectionStart) && (rva <= sectionEnd)) {
			// Yes, address is in the section.  Calculate section and offset,
			// and store in the "section" & "offset" params, which were
			// passed by reference.
			section = i+1;
			offset = rva - sectionStart;
			return TRUE;
		}
	}

	return FALSE;   // Should never get here!
}

// +--------------------------------------------------------------------+

void
ExceptionHandler::IntelStackTrace(CONTEXT* context)
{
	Print("\nStack Trace (Intel):\n");
	Print("Address   Frame     Logical addr  Module\n");

	DWORD    pc = context->Eip;
	DWORD*   pFrame;
	DWORD*   pPrevFrame;

	pFrame = (DWORD*)context->Ebp;

	do {
		char mod_name[256] = { 0 };
		DWORD section = 0, offset = 0;

		GetLogicalAddress((void*)pc, mod_name, 256, section, offset);

		Print("%08X  %08X  %04X:%08X %s\n",
		pc, pFrame, section, offset, mod_name);

		pc = pFrame[1];
		pPrevFrame = pFrame;
		pFrame = (PDWORD)pFrame[0];   // proceed to next higher frame on stack

		if ((DWORD)pFrame & 3)        // Frame pointer must be aligned on a
		break;                     // DWORD boundary.  Bail if not so.

		if (pFrame <= pPrevFrame)
		break;

		// Can two DWORDs be read from the supposed frame address?          
		if (IsBadWritePtr(pFrame, sizeof(PVOID)*2))
		break;

	}
	while ( 1 );
}

// +--------------------------------------------------------------------+

void ExceptionHandler::ImageStackTrace(CONTEXT* context)
{
	Print("\nStack Trace (Symbolic):\n");
	Print("Address   Frame\n");

	// Could use SymSetOptions here to add the SYMOPT_DEFERRED_LOADS flag
	STACKFRAME sf;
	memset(&sf, 0, sizeof(sf));

	// Initialize the STACKFRAME structure for the first call.  This is only
	// necessary for Intel CPUs, and isn't mentioned in the documentation.
	sf.AddrPC.Offset       = context->Eip;
	sf.AddrPC.Mode         = AddrModeFlat;
	sf.AddrStack.Offset    = context->Esp;
	sf.AddrStack.Mode      = AddrModeFlat;
	sf.AddrFrame.Offset    = context->Ebp;
	sf.AddrFrame.Mode      = AddrModeFlat;

	while ( 1 ) {
		if (!StackTrace( IMAGE_FILE_MACHINE_I386,
					GetCurrentProcess(),
					GetCurrentThread(),
					&sf,
					context,
					0,
					SymFunctionTableAccess,
					SymGetModuleBase,
					0))
		break;

		if (sf.AddrFrame.Offset == 0) // Basic sanity check to make sure
		break;                     // the frame is OK.  Bail if not.

		Print("%08x  %08x  ", sf.AddrPC.Offset, sf.AddrFrame.Offset);

		// IMAGEHLP is wacky, and requires you to pass in a pointer to an
		// IMAGEHLP_SYMBOL structure.  The problem is that this structure is
		// variable length.  That is, you determine how big the structure is
		// at runtime.  This means that you can't use sizeof(struct).
		// So...make a buffer that's big enough, and make a pointer
		// to the buffer.  We also need to initialize not one, but TWO
		// members of the structure before it can be used.

		BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 512];
		PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
		pSymbol->SizeOfStruct = sizeof(symbolBuffer);
		pSymbol->MaxNameLength = 512;

		DWORD symDisplacement = 0;    // Displacement of the input address,
		// relative to the start of the symbol

		if (SymGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset,
					&symDisplacement, pSymbol)) {
			Print("%-40s [%04X]\n", pSymbol->Name, symDisplacement);
		}
		else {
			char mod_name[256] = { 0 };
			DWORD section = 0, offset = 0;

			GetLogicalAddress((PVOID)sf.AddrPC.Offset,
			mod_name, 256, section, offset );

			Print("%04X:%08X %s\n", section, offset, mod_name);
		}
	}
}

// +--------------------------------------------------------------------+

BOOL
ExceptionHandler::InitImageHelp()
{
	Print("\n");

	HMODULE h = LoadLibrary("IMAGEHLP.DLL");
	if (!h) {
		Print("--- could not load IMAGEHLP.DLL (%08x) ---\n", GetLastError());
		return FALSE;
	}

	SymInitialize = (SYMINITIALIZEPROC) GetProcAddress(h, "SymInitialize");
	if (!SymInitialize) {
		Print("--- could not find SymInitialize ---\n");
		return FALSE;
	}

	SymCleanup = (SYMCLEANUPPROC) GetProcAddress(h, "SymCleanup");
	if (!SymCleanup) {
		Print("--- could not find SymCleanup ---\n");
		return FALSE;
	}

	StackTrace = (STACKWALKPROC) GetProcAddress(h, "StackWalk");
	if (!StackTrace) {
		Print("--- could not find StackWalk ---\n");
		return FALSE;
	}

	SymFunctionTableAccess = (SYMFUNCTIONTABLEACCESSPROC)
	GetProcAddress(h, "SymFunctionTableAccess");

	if (!SymFunctionTableAccess) {
		Print("--- could not find SymFunctionTableAccess ---\n");
		return FALSE;
	}

	SymGetModuleBase = (SYMGETMODULEBASEPROC) GetProcAddress(h, "SymGetModuleBase");
	if (!SymGetModuleBase) {
		Print("--- could not find SymGetModuleBase ---\n");
		return FALSE;
	}

	SymGetSymFromAddr = (SYMGETSYMFROMADDRPROC) GetProcAddress(h, "SymGetSymFromAddr");
	if (!SymGetSymFromAddr) {
		Print("--- could not find SymGetSymFromAddr ---\n");
		return FALSE;
	}

	if (!SymInitialize(GetCurrentProcess(), 0, TRUE)) {
		Print("--- could not Initialize IMAGEHLP.DLL (%08x) ---\n", GetLastError());
		return FALSE;
	}

	Print("Loaded IMAGEHLP.DLL\n");
	return TRUE;        
}

