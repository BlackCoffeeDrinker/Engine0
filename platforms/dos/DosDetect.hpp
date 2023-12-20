#pragma once

enum class SystemType {
  UNSUPPORTED,
  DOS,
  WINDOWS_3X,
  WINDOWS_95,
  WINDOWS_98,
  WINDOWS_ME,
  WINDOWS_NT,
  STEALTH_WINDOWS,
  OS2_WARP,
  OS2,
  DOSEMU,
  OPENDOS,
  OPENDOS_EMM386
};

struct VirtualDMASpec {
  enum ProductNumber {
    UNKNOWN,
    HPMM,// Quadtel's QMAPS and Hewlett-Packard's HPMM.SYS
    EMM386,// Microsoft's EMM386.EXE
    WIN386,// Windows 3.x WIN386.EXE
    OS2,// OS/2 (all versions to date)
    DRDOS_EMM386,// for DR DOS 6.0 EMM386.SYS
    QUALITAS_386MAX,// Qualitas' 386MAX
    MEMORY_COMMANDER,// V Communications' Memory Commander
    QEMM_386,// Quarterdeck's QEMM-386
    RM386,// Helix's Netroom RM386
  };

  int majorVersion;
  int minorVersion;
  ProductNumber productNumber;
  int productRevision;
};

struct SystemInfo {
  SystemType type;
  VirtualDMASpec virtual_dma_spec;
};

/**
 *
 * @return
 */
SystemInfo guessSystemInfo();
