#include "DosDetect.hpp"

#include <utility>
#include <dpmi.h>
#include <dos.h>
#include <cstdlib>
#include <cstring>
#include <go32.h>
#include <sys/farptr.h>

namespace {
/**
 * Returns current Windows version that this app is running under
 * Returns as a pair of major, minor version.
 *
 * Windows 95, 98 and ME report as major version 4.
 *
 * Windows ME is major 4, minor version 90
 * Windows 98 is major 4, minor version 10
 * Windows 95 is major 4
 *
 * @return pair.first = major version (3 or 4), pair.second = minor version
 */
std::pair<int, int> detectWindows() {
  /* Check Windows 3.X or 9X */
  __dpmi_regs r;
  r.x.ax = 0x1600;
  __dpmi_int(0x2F, &r);

  return std::make_pair(r.h.al, r.h.ah);
}

/**
 *
 * @return true if under Windows NT
 */
bool detectWindowsNt() {
  // Check NT
  if (_get_dos_version(1) == 0x0532) {
    return true;
  }

  auto p = std::getenv("OS");
  return p && std::strcmp(p, "Windows_NT") == 0;
}

/**
 *
 * @return pair.first if running under OS2, pair.second OS2 version if available
 */
std::pair<bool, int> detectOs2Version() {
  // Check OS2
  __dpmi_regs r;

  r.x.ax = 0x4010;
  __dpmi_int(0x2F, &r);

  /*
   Return:
    AX = 4010h if OS/2 not installed
    AX = 0000h for OS/2 Warp 3.0
    BX = OS/2 version if installed
   */

  if (r.x.ax == 0x4010) {
    return std::make_pair(false, 0);
  }

  if (r.x.ax == 0) {
    return std::make_pair(true, 0);
  }

  return std::make_pair(true, r.x.bx);
}

/**
 *
 * @return true if running under DOSEmu
 */
bool detectDosEmu() {
  // check for Linux DOSEMU
  _farsetsel(_dos_ds);

  char buf[9];
  for (unsigned char i = 0; i < 8; i++)
    buf[i] = static_cast<char>(_farnspeekb(i + 0xFFFF5u));

  buf[8] = 0;

  // Check for the BIOS date string "02/25/93" at F000:FFF5 before
  // calling this function. In addition, the segment address of this
  // vector should be F000h (for existing versions of DOSemu,
  // the vector is F000h:0E60h)
  if (!strcmp(buf, "02/25/93")) {
    __dpmi_regs r;
    r.x.ax = 0;
    __dpmi_int(0xE6, &r);
    return r.x.ax == 0xAA55;
  }

  return false;
}

/**
 *
 * @return pair.first is if running under OpenDOS, pair.second = EMM386 detected
 */
std::pair<bool, bool> detectOpenDOS() {
  __dpmi_regs r;
  r.x.ax = 0x4452;
  __dpmi_int(0x21, &r);

  if ((r.x.ax >= 0x1072u) && !(r.x.flags & 1u)) {
    /* now check for OpenDOS EMM386.EXE */
    r.x.ax = 0x12FF;
    r.x.bx = 0x0106;
    __dpmi_int(0x2F, &r);

    return std::make_pair(true, (r.x.ax == 0) && (r.x.bx == 0xEDC0));
  }

  return std::make_pair(false, false);
}

/**
 * Try to guess the current DOS system
 * @return
 */
SystemType guessSystemType() {
  if (detectDosEmu()) {
    return SystemType::DOSEMU;
  }

  // Detect NT
  if (detectWindowsNt()) {
    return SystemType::WINDOWS_NT;
  }

  // Are we running OS/2 ?
  const auto os2Detect = detectOs2Version();
  if (os2Detect.first) {
    if (os2Detect.second == 0)
      return SystemType::OS2;
    else
      return SystemType::OS2_WARP;
  }

  const auto openDOS = detectOpenDOS();
  if (openDOS.first) {
    if (openDOS.second)
      return SystemType::OPENDOS_EMM386;
    else
      return SystemType::OPENDOS;
  }

  // Check Windows ENH mode
  const auto dpmiInstallCheck = detectWindows();
  switch (dpmiInstallCheck.first) {
    case 3:
      return SystemType::WINDOWS_3X;
    case 4:
      switch (dpmiInstallCheck.second) {
        case 90:
          return SystemType::WINDOWS_ME;
        case 10:
          return SystemType::WINDOWS_98;
        default:
          return SystemType::WINDOWS_95;
      }
  }

  return SystemType::DOS;
}

/**
 * Query VDS (Virtual DMA Services)
 *
 * @return
 */
VirtualDMASpec queryVDS() {
  __dpmi_regs r;
  r.x.ax = 0x8102;
  r.x.bx = 0;
  r.x.dx = 0;
  __dpmi_int(0x4B, &r);

  /**
   Returns

   If function was successful:

      Carry flag is Clear.
      AH = Major specification version number (binary, currently 1)
      AL = Minor specification version number (binary, currently 0)
      BX = Product number
      CX = Product revision number
      SI:DI = Maximum DMA buffer size supported (size in bytes)
      DX = Flags
          Bit 0 = 1 If PC/XT bus architecture (DMA in first megabyte
                    only)
          Bit 1 = 1 If physical buffer/remap region is in first
                    megabyte
          Bit 2 = 1 If automatic remap supported
          Bit 3 = 1 If all memory physically contiguous
          Other flags are reserved and must be 0.

   If function was not successful:

      Carry flag is set.
      AL = Error code
          10h = Reserved flag bits set in DX

Notes

   -  The version numbers returned in the AH and AL registers
      determine the functionality supported by a VDS implementation.
      Drivers that support the functionality described in this
      document return AX=0100h.

   -  Bit 3 of the flag word is set only in an environment that runs
      in protected mode but does not perform paging. One example of
      such an environment is an 80286 MS-DOS extender. When bit 3 is
      set, the VDS services only convert selector:offset linear
      address pairs into physical addresses.
   */

  if (!(r.x.flags & 1u)) {
    VirtualDMASpec spec{};

    spec.majorVersion = r.h.ah;
    spec.minorVersion = r.h.al;
    spec.productRevision = r.x.cx;
    spec.productNumber = VirtualDMASpec::UNKNOWN;
    switch (r.x.bx) {
      case 0x0000:
        spec.productNumber = VirtualDMASpec::HPMM;
        break;
      case 0x0001:
        spec.productNumber = VirtualDMASpec::EMM386;
        break;
      case 0x0003:
        spec.productNumber = VirtualDMASpec::WIN386;
        break;
      case 0x0030:
        spec.productNumber = VirtualDMASpec::OS2;
        break;
      case 0x0EDC:
        spec.productNumber = VirtualDMASpec::DRDOS_EMM386;
        break;
      case 0x4560:
        spec.productNumber = VirtualDMASpec::QUALITAS_386MAX;
        break;
      case 0x4D43:
        spec.productNumber = VirtualDMASpec::MEMORY_COMMANDER;
        break;
      case 0x5145:
        spec.productNumber = VirtualDMASpec::QEMM_386;
        break;
      case 0x524D:
        spec.productNumber = VirtualDMASpec::RM386;
        break;
    }


    // Bitfields for VDS flags (DX):
    //
    // Bit(s)  Description     (Table 03219)
    // 0      PC/XT bus (DMA in first megabyte only)
    // 1      physical buffer/remap region in first megabyte
    // 2      automatic remap enabled
    // 3      all memory is physically contiguous
    // 4-15   reserved (zero)
    if (r.x.dx) {
    }

    return spec;
  }

  return {};
}
}// namespace

SystemInfo guessSystemInfo() {
  SystemInfo sysinfo{};

  // Setup sane defaults
  sysinfo.type = SystemType::DOS;
  sysinfo.virtual_dma_spec.productNumber = VirtualDMASpec::UNKNOWN;

  // Query VDS
  sysinfo.virtual_dma_spec = queryVDS();

  // Best guess our system
  sysinfo.type = guessSystemType();

  // Check for "stealth" DOS mode in Windows 9X
  if (sysinfo.virtual_dma_spec.productNumber == VirtualDMASpec::WIN386) {
    if (sysinfo.type == SystemType::DOS || sysinfo.type == SystemType::UNSUPPORTED) {
      sysinfo.type = SystemType::STEALTH_WINDOWS;
    }
  }

  return sysinfo;
}
