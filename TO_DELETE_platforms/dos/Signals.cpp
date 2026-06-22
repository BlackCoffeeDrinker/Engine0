#include "Signals.hpp"

#include <csignal>

using SIGNAL_HANDLER = void (*)(int);

namespace {
/* previous signal handlers */
SIGNAL_HANDLER old_sig_abrt = nullptr;
SIGNAL_HANDLER old_sig_fpe = nullptr;
SIGNAL_HANDLER old_sig_ill = nullptr;
SIGNAL_HANDLER old_sig_segv = nullptr;
SIGNAL_HANDLER old_sig_term = nullptr;
SIGNAL_HANDLER old_sig_int = nullptr;
SIGNAL_HANDLER old_sig_kill = nullptr;
SIGNAL_HANDLER old_sig_quit = nullptr;
SIGNAL_HANDLER old_sig_trap = nullptr;
}// namespace

void signals_init(void (*_func)(int)) {
  old_sig_abrt = signal(SIGABRT, _func);
  old_sig_fpe = signal(SIGFPE, _func);
  old_sig_ill = signal(SIGILL, _func);
  old_sig_segv = signal(SIGSEGV, _func);
  old_sig_term = signal(SIGTERM, _func);
  old_sig_int = signal(SIGINT, _func);

#ifdef SIGKILL
  old_sig_kill = signal(SIGKILL, _func);
#endif
#ifdef SIGQUIT
  old_sig_quit = signal(SIGQUIT, _func);
#endif
#ifdef SIGTRAP
  old_sig_trap = signal(SIGTRAP, _func);
#endif
}

void signals_shutdown() {
  // Remove signal handlers
  signal(SIGABRT, old_sig_abrt);
  signal(SIGFPE, old_sig_fpe);
  signal(SIGILL, old_sig_ill);
  signal(SIGSEGV, old_sig_segv);
  signal(SIGTERM, old_sig_term);
  signal(SIGINT, old_sig_int);

#ifdef SIGKILL
  signal(SIGKILL, old_sig_kill);
#endif
#ifdef SIGQUIT
  signal(SIGQUIT, old_sig_quit);
#endif
#ifdef SIGTRAP
  signal(SIGTRAP, old_sig_trap);
#endif
}
