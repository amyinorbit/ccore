//===--------------------------------------------------------------------------------------------===
// debug.c - Stack trace and other utilitiess
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/debug.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_STACK_DEPTH 32

#if WIN32


#else
#include <execinfo.h>
#endif


#if WIN32

/*
 * Given a module path in `filename' and a relative module address in `addr',
 * attempts to resolve the symbol name and relative symbol address. This is
 * done by looking for a syms.txt file in the same directory as the module's
 * filename.
 * If found, the symbol name + symbol relative address is placed into
 * `symname' in the "symbol+offset" format.
 * This function is deliberately designed to be simple and use as little
 * memory as possible, because when called from an exception handler, the
 * process' memory state can be assumed to be quite broken already.
 */
static void
find_symbol(const char *filename, void *addr, char *symname,
    size_t symname_cap)
{
	/*
	 * Note the `static' here is deliberate to cause these to become
	 * BSS-allocated variables instead of stack-allocated. When parsing
	 * through a stack trace we are in a pretty precarious state, so we
	 * can't rely on having much stack space available.
	 */
	static char symstxtname[MAX_PATH];
	static char prevsym[SYMNAME_MAXLEN + 1];
	static const char *sep;
	static FILE *fp;
	static void *prevptr = NULL;
	static void *image_base = NULL;

	*symname = 0;
	*prevsym = 0;

	sep = strrchr(filename, DIRSEP);
	if (sep == NULL)
		return;
	lacf_strlcpy(symstxtname, filename, MIN((uintptr_t)(sep - filename) + 1,
	    sizeof (symstxtname)));
	strncat(symstxtname, DIRSEP_S "syms.txt", sizeof (symstxtname));
	fp = fopen(symstxtname, "rb");
	if (fp == NULL)
		return;

	while (!feof(fp)) {
		static char unused_c;
		static void *ptr;
		static char sym[SYMNAME_MAXLEN + 1];

		if (fscanf(fp, "%p %c %" SCANF_STR_AUTOLEN(SYMNAME_MAXLEN) "s",
		    &ptr, &unused_c, sym) != 3) {
			break;
		}
		if (strcmp(sym, "__image_base__") == 0) {
			image_base = ptr;
			continue;
		}
		ptr = (void *)(ptr - image_base);
		if (addr >= prevptr && addr < ptr) {
			snprintf(symname, symname_cap, "%s+%x", prevsym,
			    (unsigned)(addr - prevptr));
			break;
		}
		prevptr = ptr;
		lacf_strlcpy(prevsym, sym, sizeof (prevsym));
	}
	fclose(fp);
}

static HMODULE
find_module(LPVOID pc, DWORD64 *module_base)
{
	static DWORD i;
	for (i = 0; i < num_modules; i++) {
		static LPVOID start, end;
		start = mi[i].lpBaseOfDll;
		end = start + mi[i].SizeOfImage;
		if (start <= pc && end > pc) {
			*module_base = (DWORD64)start;
			return (modules[i]);
		}
	}
	*module_base = 0;
	return (NULL);
}

static void
gather_module_info(void) {
	HANDLE process = GetCurrentProcess();

	EnumProcessModules(process, modules, sizeof (HMODULE) * MAX_MODULES,
	    &num_modules);
	num_modules = MIN(num_modules, MAX_MODULES);
	for (DWORD i = 0; i < num_modules; i++)
		GetModuleInformation(process, modules[i], &mi[i], sizeof (*mi));
}

void
log_backtrace(int skip_frames)
{
	static unsigned frames;
	static void *stack[MAX_STACK_FRAMES];
	static SYMBOL_INFO *symbol;
	static HANDLE process;
	static DWORD displacement;
	static IMAGEHLP_LINE64 *line;
	static char filename[MAX_PATH];

	frames = RtlCaptureStackBackTrace(skip_frames + 1, MAX_STACK_FRAMES,
	    stack, NULL);

	process = GetCurrentProcess();
	mutex_enter(&backtrace_lock);

	SymInitialize(process, NULL, TRUE);
	SymSetOptions(SYMOPT_LOAD_LINES);

	gather_module_info();

	memset(symbol_buf, 0, sizeof (symbol_buf));
	memset(line_buf, 0, sizeof (line_buf));

	symbol = (SYMBOL_INFO *)symbol_buf;
	symbol->MaxNameLen = MAX_SYM_NAME_LEN - 1;
	symbol->SizeOfStruct = sizeof (SYMBOL_INFO);

	line = (IMAGEHLP_LINE64 *)line_buf;
	line->SizeOfStruct = sizeof (*line);

	backtrace_buf[0] = '\0';
	lacf_strlcpy(backtrace_buf, BACKTRACE_STR, sizeof (backtrace_buf));

	for (unsigned frame_nr = 0; frame_nr < frames; frame_nr++) {
		static DWORD64 address;
		static int fill;

		address = (DWORD64)(uintptr_t)stack[frame_nr];
		fill = strlen(backtrace_buf);

		memset(symbol_buf, 0, sizeof (symbol_buf));
		/*
		 * Try to grab the symbol name from the stored %rip data.
		 */
		if (!SymFromAddr(process, address, 0, symbol)) {
			static DWORD64 start;
			static HMODULE module;

			module = find_module((void *)address, &start);
			if (module != NULL) {
				static char symname[SYMNAME_MAXLEN + 1];

				GetModuleFileNameA(module, filename,
				    sizeof (filename));
				find_symbol(filename, stack[frame_nr] - start,
				    symname, sizeof (symname));
				fill += snprintf(&backtrace_buf[fill],
				    sizeof (backtrace_buf) - fill,
				    "%d %p %s+%p (%s)\n", frame_nr,
				    stack[frame_nr], filename,
				    stack[frame_nr] - start, symname);
			} else {
				fill += snprintf(&backtrace_buf[fill],
				    sizeof (backtrace_buf) - fill,
				    "%d %p <unknown module>\n", frame_nr,
				    stack[frame_nr]);
			}
			continue;
		}
		/*
		 * See if we have debug info available with file names and
		 * line numbers.
		 */
		if (SymGetLineFromAddr64(process, address, &displacement,
		    line)) {
			snprintf(&backtrace_buf[fill], sizeof (backtrace_buf) -
			    fill, "%d: %s (0x%lx) [%s:%d]\n", frame_nr,
			    symbol->Name, (unsigned long)symbol->Address,
			    line->FileName, (int)line->LineNumber);
		} else {
			snprintf(&backtrace_buf[fill], sizeof (backtrace_buf) -
			    fill, "%d: %s - 0x%lx\n", frame_nr, symbol->Name,
			    (unsigned long)symbol->Address);
		}
	}

	if (log_func == NULL)
		abort();
	log_func(backtrace_buf);
	fputs(backtrace_buf, stderr);
	fflush(stderr);
	SymCleanup(process);

	mutex_exit(&backtrace_lock);
}

void
log_backtrace_sw64(PCONTEXT ctx)
{
	static char filename[MAX_PATH];
	static DWORD64 pcs[MAX_STACK_FRAMES];
	static unsigned num_stack_frames;
	static STACKFRAME64 sf;
	static HANDLE process, thread;
	static DWORD machine;

	process = GetCurrentProcess();
	thread = GetCurrentThread();

	mutex_enter(&backtrace_lock);

	SymInitialize(process, NULL, TRUE);
	SymSetOptions(SYMOPT_LOAD_LINES);

	gather_module_info();

	memset(&sf, 0, sizeof (sf));
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Mode = AddrModeFlat;
#if	defined(_M_IX86)
	machine = IMAGE_FILE_MACHINE_I386;
	sf.AddrPC.Offset = ctx->Eip;
	sf.AddrStack.Offset = ctx->Esp;
	sf.AddrFrame.Offset = ctx->Ebp;
#elif	defined(_M_X64)
	machine = IMAGE_FILE_MACHINE_AMD64;
	sf.AddrPC.Offset = ctx->Rip;
	sf.AddrStack.Offset = ctx->Rsp;
	sf.AddrFrame.Offset = ctx->Rbp;
#elif	defined(_M_IA64)
	machine = IMAGE_FILE_MACHINE_IA64;
	sf.AddrPC.Offset = ctx->StIIP;
	sf.AddrFrame.Offset = ctx->IntSp;
	sf.AddrBStore.Offset = ctx->RsBSP;
	sf.AddrBStore.Mode = AddrModeFlat;
	sf.AddrStack.Offset = ctx->IntSp;
#else
#error	"Unsupported architecture"
#endif	/* _M_X64 */

	for (num_stack_frames = 0; num_stack_frames < MAX_STACK_FRAMES;
	    num_stack_frames++) {
		if (!StackWalk64(machine, process, thread, &sf, ctx, NULL,
		    SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			break;
		}
		pcs[num_stack_frames] = sf.AddrPC.Offset;
	}

	backtrace_buf[0] = '\0';
	lacf_strlcpy(backtrace_buf, BACKTRACE_STR, sizeof (backtrace_buf));

	for (unsigned i = 0; i < num_stack_frames; i++) {
		static int fill;
		static DWORD64 pc;
		static char symname[SYMNAME_MAXLEN + 1];
		static HMODULE module;
		static DWORD64 mbase;

		fill = strlen(backtrace_buf);
		pc = pcs[i];

		module = find_module((LPVOID)pc, &mbase);
		GetModuleFileNameA(module, filename, sizeof (filename));
		find_symbol(filename, (void *)(pc - mbase),
		    symname, sizeof (symname));
		fill += snprintf(&backtrace_buf[fill],
		    sizeof (backtrace_buf) - fill,
		    "%d %p %s+%p (%s)\n", i, (void *)pc, filename,
		    (void *)(pc - mbase), symname);
	}

	if (log_func == NULL)
		abort();
	log_func(backtrace_buf);
	fputs(backtrace_buf, stderr);
	fflush(stderr);
	SymCleanup(process);

	mutex_exit(&backtrace_lock);
}


#else

void cc_print_stack() {
    void *array[MAX_STACK_DEPTH];
    size_t size = backtrace(array, MAX_STACK_DEPTH);
    fprintf(stderr, "=== stack trace ===\n");
    backtrace_symbols_fd(array, size, STDERR_FILENO);
}

#endif
