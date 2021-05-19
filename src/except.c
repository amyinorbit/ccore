/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license in the file COPYING
 * or http://www.opensource.org/licenses/CDDL-1.0.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file COPYING.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2018 Saso Kiselkov. All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>

#if	WIN32
#include <windows.h>
#else	/* !APL && !LIN */
#include <signal.h>
#include <err.h>
#endif	/* !APL && !LIN */

#include <ccore/log.h>
#include <ccore/debug.h>

static bool inited = false;

#if	WIN32


static LPTOP_LEVEL_EXCEPTION_FILTER prev_windows_except_handler = NULL;

LONG WINAPI
handle_windows_exception(EXCEPTION_POINTERS *ei)
{
	switch(ei->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_ACCESS_VIOLATION:
		cc_printf("Caught EXCEPTION_ACCESS_VIOLATION");
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		cc_printf("Caught EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
		break;
	case EXCEPTION_BREAKPOINT:
		cc_printf("Caught EXCEPTION_BREAKPOINT");
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		cc_printf("Caught EXCEPTION_DATATYPE_MISALIGNMENT");
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		cc_printf("Caught EXCEPTION_FLT_DENORMAL_OPERAND");
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		cc_printf("Caught EXCEPTION_FLT_DIVIDE_BY_ZERO");
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		cc_printf("Caught EXCEPTION_FLT_INEXACT_RESULT");
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		cc_printf("Caught EXCEPTION_FLT_INVALID_OPERATION");
		break;
	case EXCEPTION_FLT_OVERFLOW:
		cc_printf("Caught EXCEPTION_FLT_OVERFLOW");
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		cc_printf("Caught EXCEPTION_FLT_STACK_CHECK");
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		cc_printf("Caught EXCEPTION_FLT_UNDERFLOW");
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		cc_printf("Caught EXCEPTION_ILLEGAL_INSTRUCTION");
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		cc_printf("Caught EXCEPTION_IN_PAGE_ERROR");
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		cc_printf("Caught EXCEPTION_INT_DIVIDE_BY_ZERO");
		break;
	case EXCEPTION_INT_OVERFLOW:
		cc_printf("Caught EXCEPTION_INT_OVERFLOW");
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		cc_printf("Caught EXCEPTION_INVALID_DISPOSITION");
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		cc_printf("Caught EXCEPTION_NONCONTINUABLE_EXCEPTION");
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		cc_printf("Caught EXCEPTION_PRIV_INSTRUCTION");
		break;
	case EXCEPTION_SINGLE_STEP:
		cc_printf("Caught EXCEPTION_SINGLE_STEP");
		break;
	case EXCEPTION_STACK_OVERFLOW:
		cc_printf("Caught EXCEPTION_STACK_OVERFLOW");
		break;
	default:
		cc_printf("Caught unknown exception %lx",
		    ei->ExceptionRecord->ExceptionCode);
		break;
	}
    cc_print_stack_sw64(ei->ContextRecord);

	if (prev_windows_except_handler != NULL)
		return (prev_windows_except_handler(ei));

	return (EXCEPTION_CONTINUE_SEARCH);
}


#else

static struct sigaction old_sigsegv = {};
static struct sigaction old_sigabrt = {};
static struct sigaction old_sigfpe = {};
static struct sigaction old_sigint = {};
static struct sigaction old_sigill = {};
static struct sigaction old_sigterm = {};

static const char *
sigfpe2str(int si_code)
{
	switch (si_code) {
	case FPE_INTDIV:
		return ("integer divide by zero");
	case FPE_INTOVF:
		return ("integer overflow");
	case FPE_FLTDIV:
		return ("floating-point divide by zero");
	case FPE_FLTOVF:
		return ("floating-point overflow");
	case FPE_FLTUND:
		return ("floating-point underflow");
	case FPE_FLTRES:
		return ("floating-point inexact result");
	case FPE_FLTINV:
		return ("floating-point invalid operation");
	case FPE_FLTSUB:
		return ("subscript out of range");
	default:
		return ("general arithmetic exception");
	}
}

static const char *
sigill2str(int si_code)
{
	switch(si_code) {
	case ILL_ILLOPC:
		return ("illegal opcode");
	case ILL_ILLOPN:
		return ("illegal operand");
	case ILL_ILLADR:
		return ("illegal addressing mode");
	case ILL_ILLTRP:
		return ("illegal trap");
	case ILL_PRVOPC:
		return ("privileged opcode");
	case ILL_PRVREG:
		return ("privileged register");
	case ILL_COPROC:
		return ("coprocessor error");
	case ILL_BADSTK:
		return ("internal stack error");
	default:
		return ("unknown error");
	}
}

static void
handle_posix_sig(int sig, siginfo_t *siginfo, void *context)
{
#define	SIGNAL_FORWARD(sigact) \
	do { \
		if ((sigact)->sa_sigaction != NULL && \
		    ((sigact)->sa_flags & SA_SIGINFO)) { \
			(sigact)->sa_sigaction(sig, siginfo, context); \
		} else if ((sigact)->sa_handler != NULL) { \
			(sigact)->sa_handler(sig); \
		} \
	} while (0)
	switch (sig) {
	case SIGSEGV:
		cc_printf("Caught SIGSEGV: segmentation fault (%p)",
		    siginfo->si_addr);
		break;
	case SIGABRT:
		cc_printf("Caught SIGABORT: abort (%p)", siginfo->si_addr);
		break;
	case SIGFPE:
		cc_printf("Caught SIGFPE: floating point exception (%s)",
		    sigfpe2str(siginfo->si_code));
		break;
	case SIGILL:
		cc_printf("Caught SIGILL: illegal instruction (%s)",
		    sigill2str(siginfo->si_code));
		break;
	case SIGTERM:
		cc_printf("Caught SIGTERM: terminated");
		break;
	default:
		cc_printf("Caught signal %d", sig);
		break;
	}

	cc_print_stack(1);

	switch (sig) {
	case SIGSEGV:
		SIGNAL_FORWARD(&old_sigsegv);
		break;
	case SIGABRT:
		SIGNAL_FORWARD(&old_sigabrt);
		break;
	case SIGFPE:
		SIGNAL_FORWARD(&old_sigfpe);
		break;
	case SIGILL:
		SIGNAL_FORWARD(&old_sigill);
		break;
	case SIGTERM:
		SIGNAL_FORWARD(&old_sigterm);
		break;
	}

	exit(EXIT_FAILURE);
}

static void
signal_handler_init(void)
{
	struct sigaction sig_action = { .sa_sigaction = handle_posix_sig };

	sigemptyset(&sig_action.sa_mask);

#if	__linux__
	static uint8_t alternate_stack[SIGSTKSZ];
	stack_t ss = {
	    .ss_sp = (void*)alternate_stack,
	    .ss_size = SIGSTKSZ,
	    .ss_flags = 0
	};

	CCASSERT(sigaltstack(&ss, NULL) == 0);
	sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
#else	/* !LIN */
	sig_action.sa_flags = SA_SIGINFO;
#endif	/* !LIN */

	CCASSERT(sigaction(SIGSEGV, &sig_action, &old_sigsegv) == 0);
	CCASSERT(sigaction(SIGABRT, &sig_action, &old_sigabrt) == 0);
	CCASSERT(sigaction(SIGFPE, &sig_action, &old_sigfpe) == 0);
	CCASSERT(sigaction(SIGINT, &sig_action, &old_sigint) == 0);
	CCASSERT(sigaction(SIGILL, &sig_action, &old_sigill) == 0);
	CCASSERT(sigaction(SIGTERM, &sig_action, &old_sigterm) == 0);
}

static void
signal_handler_fini(void)
{
	CCASSERT(sigaction(SIGSEGV, &old_sigsegv, NULL) == 0);
	CCASSERT(sigaction(SIGABRT, &old_sigabrt, NULL) == 0);
	CCASSERT(sigaction(SIGFPE, &old_sigfpe, NULL) == 0);
	CCASSERT(sigaction(SIGINT, &old_sigint, NULL) == 0);
	CCASSERT(sigaction(SIGILL, &old_sigill, NULL) == 0);
	CCASSERT(sigaction(SIGTERM, &old_sigterm, NULL) == 0);
}

#endif	/* APL || LIN */

void
cc_except_init(void)
{
	CCASSERT(!inited);
	inited = true;

#if	WIN32
	prev_windows_except_handler =
	    SetUnhandledExceptionFilter(handle_windows_exception);
#else	/* !LIN && !APL */
	signal_handler_init();
#endif	/* !LIN && !APL */
}

void
cc_except_fini(void)
{
	if (!inited)
		return;
	inited = false;

#if	WIN32
	SetUnhandledExceptionFilter(prev_windows_except_handler);
#else	/* !LIN && !APL */
	signal_handler_fini();
#endif	/* !LIN && !APL */
}