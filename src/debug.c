/*
 * CDDL HEADER START
 *
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2017 Saso Kiselkov. All rights reserved.
 */
#include <ccore/debug.h>
#include <ccore/math.h>
#include <ccore/log.h>
#include <ccore/string.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_STACK_DEPTH 128

#if WIN32
#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>
#else
#include <execinfo.h>
#endif

#define	SCANF_STR_AUTOLEN_IMPL(_str_)	#_str_
#define	SCANF_STR_AUTOLEN(_str_)	SCANF_STR_AUTOLEN_IMPL(_str_)
#define	MAX_STACK_FRAMES	128
#define	MAX_MODULES		1024
#define	BACKTRACE_STR		"\nBacktrace is:\n"
#if	defined(__GNUC__) || defined(__clang__)
#define	BACKTRACE_STRLEN	__builtin_strlen(BACKTRACE_STR)
#else	/* !__GNUC__ && !__clang__ */
#define	BACKTRACE_STRLEN	strlen(BACKTRACE_STR)
#endif	/* !__GNUC__ && !__clang__ */

#if WIN32
/*
 * Since while dumping stack we are most likely in a fairly compromised
 * state, we statically pre-allocate these buffers to try and avoid having
 * to call into the VM subsystem.
 */
#define	MAX_SYM_NAME_LEN	1024
#define	MAX_BACKTRACE_LEN	(64 * 1024)
static char backtrace_buf[MAX_BACKTRACE_LEN] = { 0 };
static char symbol_buf[sizeof (SYMBOL_INFO) +
    MAX_SYM_NAME_LEN * sizeof (TCHAR)];
static char line_buf[sizeof (IMAGEHLP_LINE64)];
/* DbgHelp is not thread-safe, so avoid concurrency */
static pthread_mutex_t backtrace_lock = PTHREAD_MUTEX_INITIALIZER;

static HMODULE modules[MAX_MODULES];
static MODULEINFO mi[MAX_MODULES];
static DWORD num_modules;

#define	SYMNAME_MAXLEN	1023	/* C++ symbols can be HUUUUGE */
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

	sep = strrchr(filename, '\\');
	if (sep == NULL)
		return;
	string_copy(symstxtname, filename, MIN((uintptr_t)(sep - filename) + 1,
	    sizeof (symstxtname)));
	strncat(symstxtname, "\\" "syms.txt", sizeof (symstxtname));
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
		string_copy(prevsym, sym, sizeof (prevsym));
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
gather_module_info(void)
{
	HANDLE process = GetCurrentProcess();

	EnumProcessModules(process, modules, sizeof (HMODULE) * MAX_MODULES,
	    &num_modules);
	num_modules = MIN(num_modules, MAX_MODULES);
	for (DWORD i = 0; i < num_modules; i++)
		GetModuleInformation(process, modules[i], &mi[i], sizeof (*mi));
}

void cc_print_stack(int skip_frames) {
    static unsigned frames;
	static void *stack[MAX_STACK_FRAMES];
	static SYMBOL_INFO *symbol;
	static HANDLE process;
	static DWORD displacement;
	static IMAGEHLP_LINE64 *line;
	static char filename[MAX_PATH];

	frames = RtlCaptureStackBackTrace(skip_frames + 1, MAX_STACK_FRAMES, stack, NULL);

	process = GetCurrentProcess();
    pthread_mutex_lock(&backtrace_lock);

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
	string_copy(backtrace_buf, BACKTRACE_STR, sizeof (backtrace_buf));

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
    
	cc_print(backtrace_buf);
	fputs(backtrace_buf, stderr);
	fflush(stderr);
	SymCleanup(process);

    pthread_mutex_unlock(&backtrace_lock);
}

void
cc_print_stack_sw64(PCONTEXT ctx)
{
	static char filename[MAX_PATH];
	static DWORD64 pcs[MAX_STACK_FRAMES];
	static unsigned num_stack_frames;
	static STACKFRAME64 sf;
	static HANDLE process, thread;
	static DWORD machine;

	process = GetCurrentProcess();
	thread = GetCurrentThread();

    pthread_mutex_lock(&backtrace_lock);

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
	string_copy(backtrace_buf, BACKTRACE_STR, sizeof (backtrace_buf));

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

	cc_print(backtrace_buf);
    // fputs(backtrace_buf, stderr);
	fflush(stderr);
	SymCleanup(process);

    pthread_mutex_unlock(&backtrace_lock);
}

#else

void
cc_print_stack(int skip_frames)
{
	static char *msg;
	static size_t msg_len;
	static void *trace[MAX_STACK_FRAMES];
	static size_t i, j, sz;
	static char **fnames;

	sz = backtrace(trace, MAX_STACK_FRAMES);
	fnames = backtrace_symbols(trace, sz);

	for (i = 1 + skip_frames, msg_len = BACKTRACE_STRLEN; i < sz; i++)
		msg_len += snprintf(NULL, 0, "%s\n", fnames[i]);

	msg = (char *)malloc(msg_len + 1);
	strcpy(msg, BACKTRACE_STR);
	for (i = 1 + skip_frames, j = BACKTRACE_STRLEN; i < sz; i++)
		j += sprintf(&msg[j], "%s\n", fnames[i]);

	cc_print(msg);
    // fputs(msg, stderr);

	free(msg);
	free(fnames);
}

#endif
