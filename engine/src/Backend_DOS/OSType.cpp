
#include "OSType.hpp"

#include <cstdlib>
#include <cstring>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>

namespace {
// Helper: Standard real-mode / V86 signature detection for Windows 3.x / 9x
bool check_windows_9x(DOS::OSInfo &info) {
  __dpmi_regs r = {};
  r.x.ax = 0x1600;
  __dpmi_int(0x2F, &r);

  // Filter out negative/fallback responses
  if (r.h.al != 0 && r.h.al != 1 && r.h.al != 0x80 && r.h.al != 0xFF) {
    info.version = r.h.al;
    info.revision = r.h.ah;

    if (info.version == 4) {
      if (info.revision == 90) info.type = DOS::OSType::Type_WINME;
      else if (info.revision == 10)
        info.type = DOS::OSType::Type_WIN98;
      else
        info.type = DOS::OSType::Type_WIN95;
    } else {
      info.type = DOS::OSType::Type_WIN3;
    }
    return true;
  }
  return false;
}

// Helper: Check for NT platform kernels (Win NT 4.0, 2000, XP, etc.)
bool check_windows_nt(DOS::OSInfo &info) {
  const auto *env_os = std::getenv("OS");

  // Note: Replacing historical stricmp with standard POSIX strcasecmp or basic check
  if ((env_os && strcasecmp(env_os, "Windows_NT") == 0) || (_get_dos_version(1) == 0x0532)) {
    info.type = DOS::OSType::Type_WINNT;
    return true;
  }
  return false;
}

// Helper: Check for IBM OS/2 and OS/2 Warp platforms
bool check_os2(DOS::OSInfo &info) {
  __dpmi_regs r = {};
  r.x.ax = 0x4010;
  __dpmi_int(0x2F, &r);

  if (r.x.ax != 0x4010) {
    info.type = (r.x.ax == 0) ? DOS::OSType::Type_WARP : DOS::OSType::Type_OS2;
    return true;
  }
  return false;
}

// Helper: Check for Early Linux DOSEMU virtualization structures
bool check_dosemu(DOS::OSInfo &info) {
  _farsetsel(_dos_ds);

  char buf[9] = {0};
  for (int i = 0; i < 8; i++) {
    buf[i] = _farnspeekb(0xFFFF5 + i);
  }

  if (std::strcmp(buf, "02/25/93") == 0) {
    __dpmi_regs r = {};
    r.x.ax = 0;
    __dpmi_int(0xE6, &r);

    if (r.x.ax == 0xAA55) {
      info.type = DOS::OSType::Type_DOSEMU;
      return true;
    }
  }
  return false;
}

// Helper: Check for Digital Research / Novell OpenDOS architectures
bool check_opendos(DOS::OSInfo &info) {
  __dpmi_regs r = {};
  r.x.ax = 0x4452;// "DR"
  __dpmi_int(0x21, &r);

  if ((r.x.ax >= 0x1072) && !(r.x.flags & 1)) {
    info.type = DOS::OSType::Type_OPENDOS;

    // Check specifically for OpenDOS expanded memory extensions
    __dpmi_regs r_emm = {};
    r_emm.x.ax = 0x12FF;
    r_emm.x.bx = 0x0106;
    __dpmi_int(0x2F, &r_emm);

    if ((r_emm.x.ax == 0) && (r_emm.x.bx == 0xEDC0)) {
      info.type = DOS::OSType::Type_OPENDOS_EMM386;
    }
    return true;
  }
  return false;
}

// Helper: Check for Windows 95 safe/stealth V86 Virtual DMA API execution overrides
bool check_win95_stealth_mode(DOS::OSInfo &info) {
  __dpmi_regs r = {};
  r.x.ax = 0x8102;
  r.x.bx = 0;
  r.x.dx = 0;
  __dpmi_int(0x4B, &r);

  if ((r.x.bx == 3) && !(r.x.flags & 1)) {
    info.type = DOS::OSType::Type_WIN95;
    return true;
  }
  return false;
}

// Fallback: Populate details using the native real-mode INT 21h layer
void fetch_pure_dos_version(DOS::OSInfo &info) {
  __dpmi_regs r = {};
  r.x.ax = 0x3000;
  __dpmi_int(0x21, &r);
  info.type = DOS::OSType::Type_PURE_DOS;
  info.version = r.h.al;
  info.revision = r.h.ah;
}

bool check_dosbox_x_native(DOS::OSInfo &info) {
  __dpmi_regs r = {};
  r.x.ax = 0x12FF;// DOSBox-X Integration API hook
  r.x.bx = 0x0000;// API Status Query
  __dpmi_int(0x2F, &r);

  // If DOSBox-X API integration is active, BX returns modified status flags (often 0x4442)
  if (r.x.bx == 0x4442 || r.x.ax == 0x0000) {
    // Run a validation string check to verify
    info.type = DOS::OSType::Type_DOSBOX_X;
    return true;
  }
  return false;
}

}// namespace

namespace DOS {
/* detect_os:
 *  Completely refactored, highly readable main entry waterfall loop.
 */
OSInfo DetectOS() {
  OSInfo info;

  // Emulators must be intercepted first before they spoof standard OS reports
  if (check_dosbox_x_native(info)) return info;

  // Waterfall evaluation: order from high priority extensions down to raw bare-metal
  if (check_windows_9x(info)) return info;
  if (check_windows_nt(info)) return info;
  if (check_os2(info)) return info;
  if (check_dosemu(info)) return info;
  if (check_opendos(info)) return info;
  if (check_win95_stealth_mode(info)) return info;

  // Standard fallback if no custom wrapper trapped the identity signatures
  fetch_pure_dos_version(info);
  return info;
}
}// namespace DOS
