#include "DosModeVesa.hpp"

#include <vector>

#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>

namespace {
struct VESA_INFO {
  unsigned char VESASignature[4] __attribute__((packed));
  unsigned short VESAVersion __attribute__((packed));
  unsigned long OEMStringPtr __attribute__((packed));
  unsigned char Capabilities[4] __attribute__((packed));
  unsigned long VideoModePtr __attribute__((packed));
  unsigned short TotalMemory __attribute__((packed));
  unsigned short OemSoftwareRev __attribute__((packed));
  unsigned long OemVendorNamePtr __attribute__((packed));
  unsigned long OemProductNamePtr __attribute__((packed));
  unsigned long OemProductRevPtr __attribute__((packed));
  unsigned char Reserved[222] __attribute__((packed));
  unsigned char OemData[256] __attribute__((packed));

  explicit operator bool() const noexcept {
    return VESASignature[0] == 'V' && VESASignature[1] == 'E' && VESASignature[2] == 'S' && VESASignature[3] == 'A';
  }
};

struct MODE_INFO {
  enum class MemoryModelType : unsigned char {
    Unknown = 0xFF,
    TextMode = 0,
    CGA = 1,
    Hercules = 2,
    Planar4Planes = 3,
    PackedPixels = 4,
    NonChain4_256Colors = 5,
    DirectColor = 6,
    YUV = 7,
  };

  unsigned short ModeAttributes __attribute__((packed));
  unsigned char WinAAttributes __attribute__((packed));
  unsigned char WinBAttributes __attribute__((packed));
  unsigned short WinGranularity __attribute__((packed));
  unsigned short WinSize __attribute__((packed));
  unsigned short WinASegment __attribute__((packed));
  unsigned short WinBSegment __attribute__((packed));
  unsigned long WinFuncPtr __attribute__((packed));
  unsigned short BytesPerScanLine __attribute__((packed));
  unsigned short XResolution __attribute__((packed));
  unsigned short YResolution __attribute__((packed));
  unsigned char XCharSize __attribute__((packed));
  unsigned char YCharSize __attribute__((packed));
  unsigned char NumberOfPlanes __attribute__((packed));
  unsigned char BitsPerPixel __attribute__((packed));
  unsigned char NumberOfBanks __attribute__((packed));
  unsigned char MemoryModel __attribute__((packed));
  unsigned char BankSize __attribute__((packed));
  unsigned char NumberOfImagePages __attribute__((packed));
  unsigned char Reserved_page __attribute__((packed));
  unsigned char RedMaskSize __attribute__((packed));
  unsigned char RedMaskPos __attribute__((packed));
  unsigned char GreenMaskSize __attribute__((packed));
  unsigned char GreenMaskPos __attribute__((packed));
  unsigned char BlueMaskSize __attribute__((packed));
  unsigned char BlueMaskPos __attribute__((packed));
  unsigned char ReservedMaskSize __attribute__((packed));
  unsigned char ReservedMaskPos __attribute__((packed));
  unsigned char DirectColorModeInfo __attribute__((packed));
  unsigned long PhysBasePtr __attribute__((packed));
  unsigned long OffScreenMemOffset __attribute__((packed));
  unsigned short OffScreenMemSize __attribute__((packed));
  unsigned char Reserved[206] __attribute__((packed));

  MemoryModelType ModeType() const noexcept {
    if (MemoryModel <= 7)
      return static_cast<MemoryModelType>(MemoryModel);
    return MemoryModelType::Unknown;
  }

  bool SupportedHardware() const noexcept { return (ModeAttributes & 0b00000001) != 0; }
  bool SupportedOutputBIOS() const noexcept { return (ModeAttributes & 0b00000100) != 0; }
  bool IsColorMode() const noexcept { return (ModeAttributes & 0b00001000) != 0; }
  bool IsGraphicsMode() const noexcept { return (ModeAttributes & 0b00010000) != 0; }
};

struct VesaCardInfo {
  struct Mode {
    uint16_t mode_index;
    MODE_INFO info;
  };
  VESA_INFO info;
  std::vector<Mode> modes;
};

VESA_INFO get_vesa_info() {
  /* use the conventional memory transfer buffer */
  unsigned long dosbuf = __tb & 0xFFFFF;

  /* initialize the buffer to zero */
  for (size_t c = 0; c < sizeof(VESA_INFO); c++)
    _farpokeb(_dos_ds, dosbuf + c, 0);

  dosmemput("VBE2", 4, dosbuf);

  /* call the VESA function */
  __dpmi_regs r;
  r.x.ax = 0x4F00;
  r.x.di = dosbuf & 0xF;
  r.x.es = (dosbuf >> 4) & 0xFFFF;
  __dpmi_int(0x10, &r);

  /* quit if there was an error */
  if (r.h.ah)
    return {};

  /* copy the resulting data into our structure */
  VESA_INFO vesa_info;
  dosmemget(dosbuf, sizeof(VESA_INFO), &vesa_info);

  /* check that we got the right magic marker value */
  if (!vesa_info)
    return {};

  /* it worked! */
  return vesa_info;
}

MODE_INFO get_mode_info(uint16_t mode) {
  /* use the conventional memory transfer buffer */
  unsigned long dosbuf = __tb & 0xFFFFF;

  /* initialize the buffer to zero */
  for (size_t c = 0; c < sizeof(MODE_INFO); c++)
    _farpokeb(_dos_ds, dosbuf + c, 0);

  /* call the VESA function */
  __dpmi_regs r;
  r.x.ax = 0x4F01;
  r.x.di = dosbuf & 0xF;
  r.x.es = (dosbuf >> 4) & 0xFFFF;
  r.x.cx = mode;
  __dpmi_int(0x10, &r);

  /* quit if there was an error */
  if (r.h.ah)
    return {};

  /* copy the resulting data into our structure */
  MODE_INFO mode_info;
  dosmemget(dosbuf, sizeof(MODE_INFO), &mode_info);

  /* it worked! */
  return mode_info;
}

VesaCardInfo BuildVesaCardInfo() {
  if (const auto info = get_vesa_info()) {
    auto mode_ptr = ((info.VideoModePtr & 0xFFFF0000) >> 12) + (info.VideoModePtr & 0xFFFF);
    size_t number_of_modes = 0;
    uint16_t mode_list[256];

    /* read the list of available modes */
    while (_farpeekw(_dos_ds, mode_ptr) != 0xFFFF && number_of_modes < 255) {
      mode_list[number_of_modes] = _farpeekw(_dos_ds, mode_ptr);
      number_of_modes++;
      mode_ptr += 2;
    }

    /* populate modes */
    std::vector<VesaCardInfo::Mode> modes(number_of_modes);
    for (size_t c = 0; c < number_of_modes; c++) {
      modes[c].mode_index = mode_list[c];
      modes[c].info = get_mode_info(mode_list[c]);
    }

    return { info, std::move(modes) };
  }

  return {};
}
}// namespace

std::error_code vesa_init() {
  auto info = BuildVesaCardInfo();
  if (info.info) {
    printf("VESA Info: %s, Total memory: %d\n", info.info.OemData, info.info.TotalMemory);
    for (const auto &i : info.modes) {
      if (!i.info.SupportedHardware()) continue;
      if (!i.info.IsGraphicsMode()) continue;

      printf("Mode %d: %dx%dx%d %s, %s, %s\n",
        i.mode_index,
        i.info.XResolution,
        i.info.YResolution,
        i.info.BitsPerPixel,
        (i.info.SupportedOutputBIOS() ? "BIOS Ok" : "Not BIOS"),
        (i.info.IsColorMode() ? "Color" : "Monochrome"),
        (i.info.IsGraphicsMode() ? "GFX" : "Text"));
    }

    return {};
  }

  return std::make_error_code(std::errc::function_not_supported);
}
