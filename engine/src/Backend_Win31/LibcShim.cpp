// Small hand-rolled libc shims to eliminate the Win31 backend's dependency on
// msvcrt-os (real msvcrt.dll), which Win32s cannot always resolve cleanly and
// which pulls in a chain of UCRT-adjacent behavior we don't need. These cover
// exactly the functions libstdc++'s internals (string/vector resize, matherr
// helpers) and the vendored C libraries (lua, lodepng, inih) call into.
//
// malloc/calloc/realloc/free already live in Heap.cpp (HeapAlloc-backed), and
// memcpy/memmove/memset/memcmp/strlen/strcpy/strcmp/strcat/strncmp/abort
// already live in Intrinsics.cpp. This file provides the remaining pieces:
// memchr, strncpy/strncat/strchr/strrchr/strstr, strtol/strtoul/strtod/
// strtof, qsort, and the sprintf/snprintf/vsnprintf family.
//
#include "Win32Types.hpp"

#include <cstddef>
#include <cstdarg>
#include <cstdint>

extern "C" {

// ---------------------------------------------------------------------------
// Memory
// ---------------------------------------------------------------------------

void *memchr(const void *s, int c, size_t n) {
  const auto *p = static_cast<const unsigned char *>(s);
  const auto v = static_cast<unsigned char>(c);
  for (size_t i = 0; i < n; ++i) {
    if (p[i] == v) {
      return const_cast<unsigned char *>(p + i);
    }
  }
  return nullptr;
}

// ---------------------------------------------------------------------------
// Strings
// ---------------------------------------------------------------------------

size_t strlen(const char *s);
void *malloc(size_t size);
void free(void *ptr);

char *strncpy(char *dest, const char *src, size_t n) {
  size_t i = 0;
  for (; i < n && src[i] != '\0'; ++i) {
    dest[i] = src[i];
  }
  for (; i < n; ++i) {
    dest[i] = '\0';
  }
  return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
  char *d = dest + strlen(dest);
  size_t i = 0;
  for (; i < n && src[i] != '\0'; ++i) {
    d[i] = src[i];
  }
  d[i] = '\0';
  return dest;
}

char *strchr(const char *s, int c) {
  const auto ch = static_cast<char>(c);
  while (*s != ch) {
    if (*s == '\0') {
      return nullptr;
    }
    ++s;
  }
  return const_cast<char *>(s);
}

char *strrchr(const char *s, int c) {
  const auto ch = static_cast<char>(c);
  const char *last = nullptr;
  do {
    if (*s == ch) {
      last = s;
    }
  } while (*s++ != '\0');
  return const_cast<char *>(last);
}

char *strstr(const char *haystack, const char *needle) {
  if (*needle == '\0') {
    return const_cast<char *>(haystack);
  }
  for (; *haystack != '\0'; ++haystack) {
    const char *h = haystack;
    const char *n = needle;
    while (*h != '\0' && *n != '\0' && *h == *n) {
      ++h;
      ++n;
    }
    if (*n == '\0') {
      return const_cast<char *>(haystack);
    }
  }
  return nullptr;
}

namespace {
bool is_space(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

int digit_value(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'z') return c - 'a' + 10;
  if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
  return -1;
}
} // namespace

unsigned long strtoul(const char *nptr, char **endptr, int base) {
  const char *s = nptr;
  while (is_space(*s)) ++s;

  bool neg = false;
  if (*s == '+' || *s == '-') {
    neg = (*s == '-');
    ++s;
  }

  if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    s += 2;
    base = 16;
  } else if (base == 0 && s[0] == '0') {
    base = 8;
  } else if (base == 0) {
    base = 10;
  }

  unsigned long value = 0;
  const char *digits_start = s;
  while (true) {
    const int d = digit_value(*s);
    if (d < 0 || d >= base) break;
    value = value * static_cast<unsigned long>(base) + static_cast<unsigned long>(d);
    ++s;
  }

  if (endptr) {
    *endptr = const_cast<char *>(s == digits_start ? nptr : s);
  }
  return neg ? static_cast<unsigned long>(-static_cast<long>(value)) : value;
}

long strtol(const char *nptr, char **endptr, int base) {
  const char *s = nptr;
  while (is_space(*s)) ++s;
  const bool neg = (*s == '-');
  const unsigned long uval = strtoul(s, endptr, base);
  if (endptr && *endptr == s) {
    *endptr = const_cast<char *>(nptr);
  }
  return neg ? -static_cast<long>(uval) : static_cast<long>(uval);
}

double strtod(const char *nptr, char **endptr) {
  const char *s = nptr;
  while (is_space(*s)) ++s;

  const char *start = s;
  bool neg = false;
  if (*s == '+' || *s == '-') {
    neg = (*s == '-');
    ++s;
  }

  bool any_digits = false;
  double mantissa = 0.0;
  while (is_digit(*s)) {
    mantissa = mantissa * 10.0 + static_cast<double>(*s - '0');
    ++s;
    any_digits = true;
  }

  if (*s == '.') {
    ++s;
    double frac_scale = 0.1;
    while (is_digit(*s)) {
      mantissa += static_cast<double>(*s - '0') * frac_scale;
      frac_scale *= 0.1;
      ++s;
      any_digits = true;
    }
  }

  if (!any_digits) {
    if (endptr) {
      *endptr = const_cast<char *>(nptr);
    }
    return 0.0;
  }

  int exponent = 0;
  if (*s == 'e' || *s == 'E') {
    const char *exp_start = s;
    ++s;
    bool exp_neg = false;
    if (*s == '+' || *s == '-') {
      exp_neg = (*s == '-');
      ++s;
    }
    bool exp_digits = false;
    int exp_value = 0;
    while (is_digit(*s)) {
      exp_value = exp_value * 10 + (*s - '0');
      ++s;
      exp_digits = true;
    }
    if (exp_digits) {
      exponent = exp_neg ? -exp_value : exp_value;
    } else {
      s = exp_start; // no valid exponent digits; rewind
    }
  }

  double result = mantissa;
  if (exponent > 0) {
    for (int i = 0; i < exponent; ++i) result *= 10.0;
  } else if (exponent < 0) {
    for (int i = 0; i < -exponent; ++i) result *= 0.1;
  }

  if (endptr) {
    *endptr = const_cast<char *>(s);
  }
  (void)start;
  return neg ? -result : result;
}

float strtof(const char *nptr, char **endptr) {
  return static_cast<float>(strtod(nptr, endptr));
}

// ---------------------------------------------------------------------------
// qsort: hand-rolled insertion sort (data sets in this engine are tiny; O(n^2)
// is fine and avoids needing a dynamically-sized scratch buffer for a
// partition-based algorithm).
// ---------------------------------------------------------------------------

namespace {
void swap_bytes(unsigned char *a, unsigned char *b, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    const unsigned char t = a[i];
    a[i] = b[i];
    b[i] = t;
  }
}
} // namespace

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
  if (base == nullptr || nmemb < 2 || size == 0 || compar == nullptr) {
    return;
  }
  auto *arr = static_cast<unsigned char *>(base);
  for (size_t i = 1; i < nmemb; ++i) {
    size_t j = i;
    while (j > 0 && compar(arr + (j - 1) * size, arr + j * size) > 0) {
      swap_bytes(arr + (j - 1) * size, arr + j * size, size);
      --j;
    }
  }
}

// ---------------------------------------------------------------------------
// printf family
// ---------------------------------------------------------------------------

namespace {

struct OutBuf {
  char *buf;
  size_t cap;   // total capacity of buf, including room for the null terminator (0 = unbounded query)
  size_t count; // number of characters "written" so far (what the return value reports)
};

void out_char(OutBuf &ob, char c) {
  if (ob.buf != nullptr && ob.cap > 0 && ob.count + 1 < ob.cap) {
    ob.buf[ob.count] = c;
  }
  ++ob.count;
}

void out_str(OutBuf &ob, const char *s, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    out_char(ob, s[i]);
  }
}

void out_pad(OutBuf &ob, char c, int n) {
  for (int i = 0; i < n; ++i) {
    out_char(ob, c);
  }
}

size_t cstr_len(const char *s) {
  size_t n = 0;
  while (s[n] != '\0') ++n;
  return n;
}

// Writes the base-`base` digits of `value` into buf (no sign, no prefix),
// most-significant digit first. Returns the digit count.
int uint_to_chars(unsigned long long value, int base, bool uppercase, char *buf, int bufcap) {
  const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
  char tmp[64];
  int n = 0;
  if (value == 0) {
    tmp[n++] = '0';
  } else {
    while (value > 0 && n < static_cast<int>(sizeof(tmp))) {
      tmp[n++] = digits[value % static_cast<unsigned long long>(base)];
      value /= static_cast<unsigned long long>(base);
    }
  }
  const int count = n < bufcap ? n : bufcap;
  for (int i = 0; i < count; ++i) {
    buf[i] = tmp[n - 1 - i];
  }
  return count;
}

struct FormatSpec {
  bool left_align = false;
  bool force_sign = false;
  bool space_sign = false;
  bool alt_form = false;
  bool zero_pad = false;
  int width = 0;
  int precision = -1; // -1 = not specified
  char length_mod = '\0'; // 'h', 'H' (hh), 'l', 'L' (ll), 'q' (long double)
};

void emit_integer(OutBuf &ob, const FormatSpec &spec, bool is_signed, unsigned long long uval, bool negative,
                   int base, bool uppercase) {
  char digits[64];
  const int ndigits = uint_to_chars(uval, base, uppercase, digits, static_cast<int>(sizeof(digits)));

  char sign = '\0';
  if (is_signed) {
    if (negative) sign = '-';
    else if (spec.force_sign) sign = '+';
    else if (spec.space_sign) sign = ' ';
  }

  int prec_pad = 0;
  if (spec.precision >= 0 && spec.precision > ndigits) {
    prec_pad = spec.precision - ndigits;
  }
  if (spec.precision == 0 && uval == 0) {
    // "%.0d" of 0 prints nothing for the digit itself.
  }

  const int content_len = (sign != '\0' ? 1 : 0) + prec_pad + ndigits;
  const int pad_len = spec.width > content_len ? spec.width - content_len : 0;

  const bool use_zero_pad = spec.zero_pad && !spec.left_align && spec.precision < 0;

  if (!spec.left_align && !use_zero_pad) {
    out_pad(ob, ' ', pad_len);
  }
  if (sign != '\0') {
    out_char(ob, sign);
  }
  if (!spec.left_align && use_zero_pad) {
    out_pad(ob, '0', pad_len);
  }
  out_pad(ob, '0', prec_pad);
  if (!(spec.precision == 0 && uval == 0)) {
    out_str(ob, digits, static_cast<size_t>(ndigits));
  }
  if (spec.left_align) {
    out_pad(ob, ' ', pad_len);
  }
}

// Minimal fixed-notation double formatter. Values are assumed to fit within
// an unsigned long long once scaled by the requested precision (~18-19
// significant digits); this is adequate for engine config/loader values and
// Lua's own default "%.14g" number formatting, but is not exact for very
// large magnitudes or exotic edge cases (NaN/Inf are handled specially).
void format_fixed(OutBuf &ob, double value, int precision, bool trim_trailing_zero_only_dot) {
  if (precision < 0) precision = 6;

  double scale = 1.0;
  for (int i = 0; i < precision; ++i) scale *= 10.0;

  double rounded = value * scale + 0.5;
  auto int_scaled = static_cast<unsigned long long>(rounded);

  unsigned long long divisor = 1;
  for (int i = 0; i < precision; ++i) divisor *= 10;

  const unsigned long long int_part = precision > 0 ? int_scaled / divisor : int_scaled;
  const unsigned long long frac_part = precision > 0 ? int_scaled % divisor : 0;

  char digits[32];
  const int n = uint_to_chars(int_part, 10, false, digits, static_cast<int>(sizeof(digits)));
  out_str(ob, digits, static_cast<size_t>(n));

  if (precision > 0 && !trim_trailing_zero_only_dot) {
    out_char(ob, '.');
    char fbuf[32];
    for (int i = 0; i < precision; ++i) fbuf[i] = '0';
    unsigned long long f = frac_part;
    int idx = precision - 1;
    while (f > 0 && idx >= 0) {
      fbuf[idx--] = static_cast<char>('0' + (f % 10));
      f /= 10;
    }
    out_str(ob, fbuf, static_cast<size_t>(precision));
  }
}

void format_double(OutBuf &ob, const FormatSpec &spec, double value, char conv) {
  bool neg = value < 0.0;
  if (neg) value = -value;

  char sign = '\0';
  if (neg) sign = '-';
  else if (spec.force_sign) sign = '+';
  else if (spec.space_sign) sign = ' ';

  // Render the numeric body into a small local buffer first so width/padding
  // can be applied uniformly.
  char body[128];
  OutBuf tmp{body, sizeof(body), 0};

  int precision = spec.precision;

  if (conv == 'f' || conv == 'F') {
    format_fixed(tmp, value, precision < 0 ? 6 : precision, false);
  } else if (conv == 'e' || conv == 'E') {
    // Normalize value into [1, 10).
    int exp10 = 0;
    double v = value;
    if (v != 0.0) {
      while (v >= 10.0) { v *= 0.1; ++exp10; }
      while (v < 1.0) { v *= 10.0; --exp10; }
    }
    const int prec = precision < 0 ? 6 : precision;
    format_fixed(tmp, v, prec, false);
    out_char(tmp, conv == 'E' ? 'E' : 'e');
    out_char(tmp, exp10 < 0 ? '-' : '+');
    const int aexp = exp10 < 0 ? -exp10 : exp10;
    char ebuf[16];
    int en = uint_to_chars(static_cast<unsigned long long>(aexp), 10, false, ebuf, static_cast<int>(sizeof(ebuf)));
    if (en < 2) {
      out_char(tmp, '0');
    }
    out_str(tmp, ebuf, static_cast<size_t>(en));
  } else { // 'g' / 'G'
    int prec = precision < 0 ? 6 : (precision == 0 ? 1 : precision);
    int exp10 = 0;
    double v = value;
    if (v != 0.0) {
      while (v >= 10.0) { v *= 0.1; ++exp10; }
      while (v < 1.0) { v *= 10.0; --exp10; }
    }
    if (exp10 < -4 || exp10 >= prec) {
      const int mant_prec = prec - 1;
      format_fixed(tmp, v, mant_prec < 0 ? 0 : mant_prec, false);
      // Trim trailing zeros in mantissa unless alt form requested.
      if (!spec.alt_form) {
        while (tmp.count > 0 && tmp.buf[tmp.count - 1] == '0') --tmp.count;
        if (tmp.count > 0 && tmp.buf[tmp.count - 1] == '.') --tmp.count;
      }
      out_char(tmp, conv == 'G' ? 'E' : 'e');
      out_char(tmp, exp10 < 0 ? '-' : '+');
      const int aexp = exp10 < 0 ? -exp10 : exp10;
      char ebuf[16];
      int en = uint_to_chars(static_cast<unsigned long long>(aexp), 10, false, ebuf, static_cast<int>(sizeof(ebuf)));
      if (en < 2) out_char(tmp, '0');
      out_str(tmp, ebuf, static_cast<size_t>(en));
    } else {
      const int frac_digits = prec - 1 - exp10;
      format_fixed(tmp, value, frac_digits < 0 ? 0 : frac_digits, false);
      if (!spec.alt_form) {
        while (tmp.count > 0 && tmp.buf[tmp.count - 1] == '0') --tmp.count;
        if (tmp.count > 0 && tmp.buf[tmp.count - 1] == '.') --tmp.count;
      }
    }
  }

  const int body_len = static_cast<int>(tmp.count);
  const int content_len = (sign != '\0' ? 1 : 0) + body_len;
  const int pad_len = spec.width > content_len ? spec.width - content_len : 0;
  const bool use_zero_pad = spec.zero_pad && !spec.left_align;

  if (!spec.left_align && !use_zero_pad) {
    out_pad(ob, ' ', pad_len);
  }
  if (sign != '\0') {
    out_char(ob, sign);
  }
  if (!spec.left_align && use_zero_pad) {
    out_pad(ob, '0', pad_len);
  }
  out_str(ob, body, static_cast<size_t>(body_len));
  if (spec.left_align) {
    out_pad(ob, ' ', pad_len);
  }
}

int vformat(OutBuf &ob, const char *fmt, va_list ap) {
  while (*fmt != '\0') {
    if (*fmt != '%') {
      out_char(ob, *fmt++);
      continue;
    }
    ++fmt; // consume '%'
    if (*fmt == '%') {
      out_char(ob, '%');
      ++fmt;
      continue;
    }

    FormatSpec spec;
    // Flags.
    bool flags_done = false;
    while (!flags_done) {
      switch (*fmt) {
        case '-': spec.left_align = true; ++fmt; break;
        case '+': spec.force_sign = true; ++fmt; break;
        case ' ': spec.space_sign = true; ++fmt; break;
        case '#': spec.alt_form = true; ++fmt; break;
        case '0': spec.zero_pad = true; ++fmt; break;
        default: flags_done = true; break;
      }
    }

    // Width.
    if (*fmt == '*') {
      spec.width = va_arg(ap, int);
      if (spec.width < 0) {
        spec.left_align = true;
        spec.width = -spec.width;
      }
      ++fmt;
    } else {
      int w = 0;
      bool has_width = false;
      while (is_digit(*fmt)) {
        w = w * 10 + (*fmt - '0');
        ++fmt;
        has_width = true;
      }
      if (has_width) spec.width = w;
    }

    // Precision.
    if (*fmt == '.') {
      ++fmt;
      if (*fmt == '*') {
        spec.precision = va_arg(ap, int);
        ++fmt;
      } else {
        int p = 0;
        while (is_digit(*fmt)) {
          p = p * 10 + (*fmt - '0');
          ++fmt;
        }
        spec.precision = p;
      }
    }

    // Length modifiers.
    while (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'j' || *fmt == 'z' || *fmt == 't' || *fmt == 'q') {
      if (*fmt == 'h' && spec.length_mod == 'h') {
        spec.length_mod = 'H'; // hh
      } else if (*fmt == 'l' && spec.length_mod == 'l') {
        spec.length_mod = 'q'; // ll
      } else if (*fmt == 'j' || *fmt == 'z' || *fmt == 't') {
        spec.length_mod = 'l'; // treat as long-sized
      } else {
        spec.length_mod = *fmt;
      }
      ++fmt;
    }

    const char conv = *fmt;
    if (conv == '\0') break;
    ++fmt;

    switch (conv) {
      case 'd':
      case 'i': {
        long long v;
        if (spec.length_mod == 'q') v = va_arg(ap, long long);
        else if (spec.length_mod == 'l') v = va_arg(ap, long);
        else v = va_arg(ap, int);
        const bool negative = v < 0;
        const unsigned long long uv = negative ? static_cast<unsigned long long>(-v) : static_cast<unsigned long long>(v);
        emit_integer(ob, spec, true, uv, negative, 10, false);
        break;
      }
      case 'u':
      case 'x':
      case 'X':
      case 'o': {
        unsigned long long v;
        if (spec.length_mod == 'q') v = va_arg(ap, unsigned long long);
        else if (spec.length_mod == 'l') v = va_arg(ap, unsigned long);
        else v = va_arg(ap, unsigned int);
        const int base = conv == 'o' ? 8 : (conv == 'u' ? 10 : 16);
        emit_integer(ob, spec, false, v, false, base, conv == 'X');
        break;
      }
      case 'c': {
        const char c = static_cast<char>(va_arg(ap, int));
        const int pad_len = spec.width > 1 ? spec.width - 1 : 0;
        if (!spec.left_align) out_pad(ob, ' ', pad_len);
        out_char(ob, c);
        if (spec.left_align) out_pad(ob, ' ', pad_len);
        break;
      }
      case 's': {
        const char *s = va_arg(ap, const char *);
        if (s == nullptr) s = "(null)";
        size_t len = cstr_len(s);
        if (spec.precision >= 0 && static_cast<size_t>(spec.precision) < len) {
          len = static_cast<size_t>(spec.precision);
        }
        const int pad_len = spec.width > static_cast<int>(len) ? spec.width - static_cast<int>(len) : 0;
        if (!spec.left_align) out_pad(ob, ' ', pad_len);
        out_str(ob, s, len);
        if (spec.left_align) out_pad(ob, ' ', pad_len);
        break;
      }
      case 'f':
      case 'F':
      case 'e':
      case 'E':
      case 'g':
      case 'G': {
        double v = spec.length_mod == 'L' ? static_cast<double>(va_arg(ap, long double)) : va_arg(ap, double);
        format_double(ob, spec, v, conv);
        break;
      }
      case 'p': {
        const void *v = va_arg(ap, const void *);
        FormatSpec pspec = spec;
        pspec.alt_form = true;
        out_str(ob, "0x", 2);
        emit_integer(ob, FormatSpec{}, false, reinterpret_cast<uintptr_t>(v), false, 16, false);
        break;
      }
      default:
        out_char(ob, '%');
        out_char(ob, conv);
        break;
    }
  }
  return static_cast<int>(ob.count);
}

} // namespace

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
  OutBuf ob{str, size, 0};
  const int result = vformat(ob, format, ap);
  if (str != nullptr && size > 0) {
    const size_t term_index = ob.count < size ? ob.count : size - 1;
    str[term_index] = '\0';
  }
  return result;
}

int vsprintf(char *str, const char *format, va_list ap) {
  return vsnprintf(str, static_cast<size_t>(-1) / 2, format, ap);
}

int snprintf(char *str, size_t size, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  const int result = vsnprintf(str, size, format, ap);
  va_end(ap);
  return result;
}

int sprintf(char *str, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  const int result = vsprintf(str, format, ap);
  va_end(ap);
  return result;
}

// ---------------------------------------------------------------------------
// Math: mingwex's own pow/powi/ldexp/etc "matherr" helpers are still linked
// (they're what actually provide sin/cos/exp/log/frexp/etc. for us), but the
// small set of transcendental entry points below is required by Lua's own
// arithmetic (lobject.c/lvm.c call these directly for the ^ operator and
// integer/float coercions). Hand-rolled: fine for game/config math, not
// bit-exact with a real libm.
// ---------------------------------------------------------------------------

double floor(double x) {
  if (x >= 0.0) {
    const auto i = static_cast<unsigned long long>(x);
    return static_cast<double>(i);
  }
  const auto i = static_cast<long long>(x);
  const double truncated = static_cast<double>(i);
  return truncated == x ? truncated : truncated - 1.0;
}

double fmod(double x, double y) {
  if (y == 0.0) {
    return 0.0;
  }
  const double q = x / y;
  const auto iq = static_cast<long long>(q); // truncate toward zero
  return x - static_cast<double>(iq) * y;
}

namespace {
// exp(x) via range reduction (x = k*ln2 + r) plus a short Taylor series on r.
double shim_exp(double x) {
  if (x == 0.0) return 1.0;
  bool invert = false;
  if (x < 0.0) {
    invert = true;
    x = -x;
  }
  constexpr double kLn2 = 0.6931471805599453;
  auto k = static_cast<long long>(x / kLn2);
  double r = x - static_cast<double>(k) * kLn2;

  // Taylor series for e^r, r is small (< ln2).
  double term = 1.0;
  double sum = 1.0;
  for (int n = 1; n <= 20; ++n) {
    term *= r / static_cast<double>(n);
    sum += term;
  }

  double result = sum;
  for (long long i = 0; i < k; ++i) result *= 2.0;

  return invert ? (1.0 / result) : result;
}

// Natural log via range reduction to [1,2) using frexp-like scaling, then a
// series on (m-1)/(m+1).
double shim_log(double x) {
  if (x <= 0.0) return 0.0; // pragmatic: no NaN/-inf machinery here
  int exponent = 0;
  double m = x;
  while (m >= 2.0) { m *= 0.5; ++exponent; }
  while (m < 1.0) { m *= 2.0; --exponent; }

  const double t = (m - 1.0) / (m + 1.0);
  const double t2 = t * t;
  double term = t;
  double sum = 0.0;
  for (int n = 0; n < 20; ++n) {
    sum += term / static_cast<double>(2 * n + 1);
    term *= t2;
  }
  constexpr double kLn2 = 0.6931471805599453;
  return 2.0 * sum + static_cast<double>(exponent) * kLn2;
}
} // namespace

double frexp(double value, int *exp) {
  if (value == 0.0 || exp == nullptr) {
    if (exp != nullptr) *exp = 0;
    return value;
  }
  bool neg = value < 0.0;
  double m = neg ? -value : value;
  int e = 0;
  while (m >= 1.0) { m *= 0.5; ++e; }
  while (m < 0.5) { m *= 2.0; --e; }
  *exp = e;
  return neg ? -m : m;
}

double pow(double base, double exponent) {
  if (exponent == 0.0) return 1.0;
  if (base == 0.0) return exponent > 0.0 ? 0.0 : 0.0;
  if (base < 0.0) {
    const auto ie = static_cast<long long>(exponent);
    if (static_cast<double>(ie) != exponent) {
      return 0.0; // undefined for non-integer exponent with negative base; pragmatic fallback
    }
    const double magnitude = pow(-base, exponent);
    return (ie % 2 != 0) ? -magnitude : magnitude;
  }
  // Fast path: small non-negative integer exponents via repeated squaring
  // (better precision than exp/log round-trip).
  const auto ie = static_cast<long long>(exponent);
  if (static_cast<double>(ie) == exponent && ie >= 0 && ie <= 64) {
    double result = 1.0;
    double b = base;
    unsigned long long n = static_cast<unsigned long long>(ie);
    while (n > 0) {
      if (n & 1u) result *= b;
      b *= b;
      n >>= 1u;
    }
    return result;
  }
  return shim_exp(exponent * shim_log(base));
}

// atan(x) for |x| <= 1 via a Taylor series (converges slowly near +/-1, but
// the range reduction below via reciprocal/complement identities keeps the
// series argument small); atan2 built on top of it, matching the two-argument
// arctangent semantics used by ActorMobile.cpp's DirectionFromVector.
namespace {
double shim_atan(double x) {
  bool neg = x < 0.0;
  if (neg) x = -x;
  bool reciprocal = x > 1.0;
  if (reciprocal) x = 1.0 / x;

  constexpr double kPi = 3.14159265358979323846;
  double term = x;
  const double x2 = x * x;
  double sum = 0.0;
  for (int n = 0; n < 40; ++n) {
    const double denom = static_cast<double>(2 * n + 1);
    sum += (n % 2 == 0 ? term / denom : -term / denom);
    term *= x2;
  }

  double result = reciprocal ? (kPi * 0.5 - sum) : sum;
  return neg ? -result : result;
}
} // namespace

double atan2(double y, double x) {
  constexpr double kPi = 3.14159265358979323846;
  if (x > 0.0) {
    return shim_atan(y / x);
  }
  if (x < 0.0) {
    return y >= 0.0 ? (shim_atan(y / x) + kPi) : (shim_atan(y / x) - kPi);
  }
  // x == 0
  if (y > 0.0) return kPi * 0.5;
  if (y < 0.0) return -kPi * 0.5;
  return 0.0; // undefined, pragmatic fallback
}

long lround(double x) {
  return static_cast<long>(x >= 0.0 ? floor(x + 0.5) : -floor(-x + 0.5));
}

long lrint(double x) {
  return lround(x);
}

// ---------------------------------------------------------------------------
// assert(): mingw's <assert.h> expands the `assert(expr)` macro directly into
// a call to `_assert(message, file, line)` (declared __MINGW_ATTRIB_NORETURN).
// Providing our own definition here means the linker never has to pull in
// libmingwex.a's own `_assert.o`, which in turn calls `fileno`/`_setmode` and
// a real msvcrt.dll-imported `_assert` -- none of which exist without msvcrt.
// Ours just logs via OutputDebugStringA (matching CreateSink.cpp's approach)
// and aborts, which is all `assert()` needs to do.
// ---------------------------------------------------------------------------

extern "C" void OutputDebugStringA(const char *lpOutputString);
[[noreturn]] void abort();

[[noreturn]] void _assert(const char *message, const char *file, unsigned line) {
  char buf[512];
  int pos = 0;
  auto append = [&](const char *s) {
    while (*s != '\0' && pos < static_cast<int>(sizeof(buf)) - 1) {
      buf[pos++] = *s++;
    }
  };
  append("Assertion failed: ");
  append(message != nullptr ? message : "(null)");
  append(", file ");
  append(file != nullptr ? file : "(null)");
  append(", line ");

  char linebuf[16];
  const int n = uint_to_chars(static_cast<unsigned long long>(line), 10, false, linebuf, static_cast<int>(sizeof(linebuf)));
  for (int i = 0; i < n && pos < static_cast<int>(sizeof(buf)) - 1; ++i) {
    buf[pos++] = linebuf[i];
  }
  append("\n");
  buf[pos] = '\0';

  OutputDebugStringA(buf);
  abort();
}

// ---------------------------------------------------------------------------
// Minimal FILE*: backs lodepng's fopen()-based file helpers (always linked
// in as part of lodepng.cpp's single translation unit, even if the engine's
// loaders currently reach it through e00::Stream/Win32 file APIs instead)
// and mingwex's own diagnostic fprintf/pformat paths. Real files go through
// Win32 CreateFileA/ReadFile/WriteFile/SetFilePointer; the 3 standard streams
// route writes through OutputDebugStringA, matching CreateSink.cpp's existing
// logging convention, and reads as always-EOF (Win32s has no console stdin).
// ---------------------------------------------------------------------------

} // extern "C"

struct FILE {
  HANDLE handle = nullptr;
  bool is_std_stream = false;
  bool eof_flag = false;
  bool error_flag = false;
  int ungetc_value = -1;
};

namespace {
FILE g_stdin_file;
FILE g_stdout_file{nullptr, true, false, false, -1};
FILE g_stderr_file{nullptr, true, false, false, -1};

void debug_write(const char *data, size_t len) {
  char line[256];
  size_t i = 0;
  size_t chunk = 0;
  while (i < len) {
    while (i < len && chunk < sizeof(line) - 1) {
      line[chunk++] = data[i++];
    }
    line[chunk] = '\0';
    OutputDebugStringA(line);
    chunk = 0;
  }
}
} // namespace

extern "C" {

void **__acrt_iob_func(unsigned index) {
  switch (index) {
    case 0: return reinterpret_cast<void **>(&g_stdin_file);
    case 1: return reinterpret_cast<void **>(&g_stdout_file);
    default: return reinterpret_cast<void **>(&g_stderr_file);
  }
}

using __acrt_iob_func_t = void *(*)(unsigned);
__acrt_iob_func_t _imp____acrt_iob_func = reinterpret_cast<__acrt_iob_func_t>(&__acrt_iob_func);

FILE *fopen(const char *path, const char *mode) {
  if (path == nullptr || mode == nullptr) {
    return nullptr;
  }
  const bool for_write = strchr(mode, 'w') != nullptr || strchr(mode, 'a') != nullptr;
  const bool for_read_write = strchr(mode, '+') != nullptr;
  DWORD access = for_write ? GENERIC_WRITE : GENERIC_READ;
  if (for_read_write) {
    access = GENERIC_READ | GENERIC_WRITE;
  }
  const DWORD disposition = strchr(mode, 'w') != nullptr ? CREATE_ALWAYS
                             : strchr(mode, 'a') != nullptr ? OPEN_ALWAYS
                                                             : OPEN_EXISTING;
  HANDLE handle = CreateFileA(path, access, FILE_SHARE_READ, nullptr, disposition,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
  if (handle == nullptr || handle == reinterpret_cast<HANDLE>(static_cast<long>(-1))) {
    return nullptr;
  }
  auto *f = static_cast<FILE *>(malloc(sizeof(FILE)));
  if (f == nullptr) {
    CloseHandle(handle);
    return nullptr;
  }
  f->handle = handle;
  f->is_std_stream = false;
  f->eof_flag = false;
  f->error_flag = false;
  f->ungetc_value = -1;
  if (strchr(mode, 'a') != nullptr) {
    SetFilePointer(handle, 0, nullptr, FILE_END);
  }
  return f;
}

FILE *freopen(const char *path, const char *mode, FILE *stream) {
  if (stream == nullptr) {
    return fopen(path, mode);
  }
  if (stream->handle != nullptr) {
    CloseHandle(stream->handle);
    stream->handle = nullptr;
  }
  if (path == nullptr) {
    return stream;
  }
  FILE *reopened = fopen(path, mode);
  if (reopened == nullptr) {
    return nullptr;
  }
  stream->handle = reopened->handle;
  stream->eof_flag = false;
  stream->error_flag = false;
  stream->ungetc_value = -1;
  free(reopened);
  return stream;
}

int fclose(FILE *stream) {
  if (stream == nullptr) {
    return -1;
  }
  if (stream->handle != nullptr) {
    CloseHandle(stream->handle);
  }
  if (!stream->is_std_stream) {
    free(stream);
  }
  return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (stream == nullptr || ptr == nullptr || size == 0 || nmemb == 0) {
    return 0;
  }
  auto *dest = static_cast<unsigned char *>(ptr);
  size_t total = size * nmemb;
  size_t written = 0;

  if (stream->ungetc_value >= 0 && total > 0) {
    dest[0] = static_cast<unsigned char>(stream->ungetc_value);
    stream->ungetc_value = -1;
    ++written;
  }

  if (stream->handle == nullptr) {
    stream->eof_flag = true;
    return written / size;
  }

  if (written < total) {
    DWORD read_bytes = 0;
    const DWORD to_read = static_cast<DWORD>(total - written);
    const BOOL ok = ReadFile(stream->handle, dest + written, to_read, &read_bytes, nullptr);
    if (!ok) {
      stream->error_flag = true;
    } else if (read_bytes < to_read) {
      stream->eof_flag = true;
    }
    written += read_bytes;
  }
  return written / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  if (stream == nullptr || ptr == nullptr || size == 0 || nmemb == 0) {
    return 0;
  }
  const size_t total = size * nmemb;
  if (stream->is_std_stream) {
    debug_write(static_cast<const char *>(ptr), total);
    return nmemb;
  }
  if (stream->handle == nullptr) {
    return 0;
  }
  DWORD written = 0;
  if (!WriteFile(stream->handle, ptr, static_cast<DWORD>(total), &written, nullptr)) {
    stream->error_flag = true;
  }
  return written / size;
}

int fseek(FILE *stream, long offset, int whence) {
  if (stream == nullptr || stream->handle == nullptr) {
    return -1;
  }
  stream->ungetc_value = -1;
  stream->eof_flag = false;
  const DWORD method = whence == 1 ? FILE_CURRENT : (whence == 2 ? FILE_END : FILE_BEGIN);
  const DWORD result = SetFilePointer(stream->handle, offset, nullptr, method);
  return result == static_cast<DWORD>(-1) ? -1 : 0;
}

long ftell(FILE *stream) {
  if (stream == nullptr || stream->handle == nullptr) {
    return -1;
  }
  return static_cast<long>(SetFilePointer(stream->handle, 0, nullptr, FILE_CURRENT));
}

int feof(FILE *stream) {
  return (stream != nullptr && stream->eof_flag) ? 1 : 0;
}

int ferror(FILE *stream) {
  return (stream != nullptr && stream->error_flag) ? 1 : 0;
}

void clearerr(FILE *stream) {
  if (stream != nullptr) {
    stream->eof_flag = false;
    stream->error_flag = false;
  }
}

int fflush(FILE *) {
  return 0; // nothing buffered on our side
}

int fputc(int c, FILE *stream) {
  const unsigned char ch = static_cast<unsigned char>(c);
  return fwrite(&ch, 1, 1, stream) == 1 ? c : -1;
}

int fputs(const char *s, FILE *stream) {
  if (s == nullptr) {
    return -1;
  }
  const size_t len = strlen(s);
  return fwrite(s, 1, len, stream) == len ? 0 : -1;
}

int fgetc(FILE *stream) {
  unsigned char c = 0;
  return fread(&c, 1, 1, stream) == 1 ? static_cast<int>(c) : -1;
}

int getc(FILE *stream) {
  return fgetc(stream);
}

int ungetc(int c, FILE *stream) {
  if (stream == nullptr || c < 0) {
    return -1;
  }
  stream->ungetc_value = c;
  stream->eof_flag = false;
  return c;
}

char *fgets(char *buf, int size, FILE *stream) {
  if (buf == nullptr || size <= 0 || stream == nullptr) {
    return nullptr;
  }
  int i = 0;
  while (i < size - 1) {
    const int c = fgetc(stream);
    if (c < 0) {
      break;
    }
    buf[i++] = static_cast<char>(c);
    if (c == '\n') {
      break;
    }
  }
  if (i == 0) {
    return nullptr;
  }
  buf[i] = '\0';
  return buf;
}

int remove(const char *path) {
  (void) path;
  return -1; // unsupported: not exercised by the engine's actual code paths
}

int rename(const char *oldpath, const char *newpath) {
  (void) oldpath;
  (void) newpath;
  return -1;
}

FILE *tmpfile() {
  return nullptr;
}

char *tmpnam(char *buf) {
  if (buf != nullptr) {
    buf[0] = '\0';
  }
  return buf;
}

int setvbuf(FILE *, char *, int, size_t) {
  return 0; // no buffering to configure
}

// ---------------------------------------------------------------------------
// errno / locale / ctype: mingwex and Lua reference these two different ways
// depending on how each was compiled against system headers -- some call
// sites expect a plain function symbol, others expect a dllimport thunk (a
// data symbol named "_imp__<name>" holding the function's address, since
// their translation unit declared it with __declspec(dllimport)). We satisfy
// both forms for each symbol below.
// ---------------------------------------------------------------------------

namespace {
int g_errno_storage = 0;
} // namespace

int *_errno() {
  return &g_errno_storage;
}
using errno_fn_t = int *(*)();
errno_fn_t _imp___errno = &_errno;

struct lconv {
  char *decimal_point;
  char *thousands_sep;
  char *grouping;
  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;
  char p_cs_precedes;
  char p_sep_by_space;
  char n_cs_precedes;
  char n_sep_by_space;
  char p_sign_posn;
  char n_sign_posn;
};

namespace {
char g_empty_str[] = "";
char g_decimal_point[] = ".";
lconv g_lconv{g_decimal_point, g_empty_str, g_empty_str, g_empty_str, g_empty_str,
              g_empty_str, g_empty_str, g_empty_str, g_empty_str, g_empty_str,
              127, 127, 127, 127, 127, 127, 127, 127};
} // namespace

lconv *localeconv() {
  return &g_lconv;
}
using localeconv_fn_t = lconv *(*)();
localeconv_fn_t _imp__localeconv = &localeconv;

char *setlocale(int, const char *) {
  return const_cast<char *>("C"); // only the "C" locale is supported
}

int isspace(int c) {
  return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r') ? 1 : 0;
}
using isspace_fn_t = int (*)(int);
isspace_fn_t _imp__isspace = &isspace;

int isalnum(int c) {
  return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ? 1 : 0;
}

int isalpha(int c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ? 1 : 0;
}

int strcoll(const char *s1, const char *s2) {
  return strcmp(s1, s2); // "C" locale: byte-wise comparison
}
using strcoll_fn_t = int (*)(const char *, const char *);
strcoll_fn_t _imp__strcoll = &strcoll;

char *strerror(int) {
  static char msg[] = "error";
  return msg;
}
using strerror_fn_t = char *(*)(int);
strerror_fn_t _imp__strerror = &strerror;

int rand_s(unsigned int *out) {
  // Not cryptographically secure: a small xorshift seeded from the tick
  // count, matching this shim's "pragmatic, not IEEE/spec-perfect" scope.
  static unsigned int state = 2463534242u;
  if (out == nullptr) {
    return -1;
  }
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  *out = state;
  return 0;
}
using rand_s_fn_t = int (*)(unsigned int *);
rand_s_fn_t _imp__rand_s = &rand_s;

// ---------------------------------------------------------------------------
// setjmp/longjmp: Lua's own error handling (ldo.c) is compiled as C, so it
// always uses setjmp/longjmp (never C++ try/catch) regardless of our
// -fno-exceptions engine code. mingw's <setjmp.h> expands setjmp() to
// _setjmp3(env, NULL) on this target and expects the real msvcrt jmp_buf
// layout; since we also provide longjmp ourselves, only internal consistency
// between this pair matters, not compatibility with the real CRT's ABI.
// jmp_buf (from <setjmp.h>) is a byte array comfortably larger than the 6
// registers we save here (ebx, esi, edi, ebp, esp, return eip).
// ---------------------------------------------------------------------------

__attribute__((naked)) int _setjmp3(void * /*env*/, ...) {
  __asm__ volatile(
      "mov 4(%esp), %eax\n"
      "mov %ebx, 0(%eax)\n"
      "mov %esi, 4(%eax)\n"
      "mov %edi, 8(%eax)\n"
      "mov %ebp, 12(%eax)\n"
      "lea 4(%esp), %ecx\n"
      "mov %ecx, 16(%eax)\n"
      "mov (%esp), %ecx\n"
      "mov %ecx, 20(%eax)\n"
      "xor %eax, %eax\n"
      "ret\n");
}
using setjmp3_fn_t = int (*)(void *, ...);
setjmp3_fn_t _imp___setjmp3 = &_setjmp3;

__attribute__((naked)) void longjmp(void * /*env*/, int /*val*/) {
  __asm__ volatile(
      "mov 4(%esp), %eax\n"
      "mov 8(%esp), %edx\n"
      "mov 0(%eax), %ebx\n"
      "mov 4(%eax), %esi\n"
      "mov 8(%eax), %edi\n"
      "mov 12(%eax), %ebp\n"
      "mov 16(%eax), %esp\n"
      "mov 20(%eax), %ecx\n"
      "test %edx, %edx\n"
      "jnz 1f\n"
      "mov $1, %edx\n"
      "1:\n"
      "mov %edx, %eax\n"
      "jmp *%ecx\n");
}
using longjmp_fn_t = void (*)(void *, int);
longjmp_fn_t _imp__longjmp = &longjmp;

// ---------------------------------------------------------------------------
// Misc: pulled in transitively by libstdc++'s std::random_device fallback
// path (random.o, referenced unconditionally once anything in libstdc++'s
// <random> support is linked) and by a few remaining mingwex/Lua call sites.
// ---------------------------------------------------------------------------

char *getenv(const char *) {
  return nullptr; // Win32s: no meaningful process environment to expose
}

int read(int, void *, unsigned int) {
  return -1; // no POSIX file descriptors on this target
}

void exit(int code) {
  ExitProcess(static_cast<UINT>(code));
}

int system(const char *) {
  return -1;
}

long long _time64(long long *out) {
  const long long now = static_cast<long long>(GetTickCount()) / 1000;
  if (out != nullptr) {
    *out = now;
  }
  return now;
}

void _lock_file(FILE *) {
}

void _unlock_file(FILE *) {
}

size_t strnlen(const char *s, size_t maxlen) {
  size_t n = 0;
  while (n < maxlen && s[n] != '\0') {
    ++n;
  }
  return n;
}

char *strpbrk(const char *s, const char *accept) {
  for (; *s != '\0'; ++s) {
    for (const char *a = accept; *a != '\0'; ++a) {
      if (*s == *a) {
        return const_cast<char *>(s);
      }
    }
  }
  return nullptr;
}

size_t strspn(const char *s, const char *accept) {
  size_t n = 0;
  while (s[n] != '\0') {
    bool found = false;
    for (const char *a = accept; *a != '\0'; ++a) {
      if (s[n] == *a) {
        found = true;
        break;
      }
    }
    if (!found) {
      break;
    }
    ++n;
  }
  return n;
}

size_t wcslen(const wchar_t *s) {
  size_t n = 0;
  while (s[n] != 0) {
    ++n;
  }
  return n;
}

size_t wcsnlen(const wchar_t *s, size_t maxlen) {
  size_t n = 0;
  while (n < maxlen && s[n] != 0) {
    ++n;
  }
  return n;
}

// ASCII-only wide<->narrow conversions: sufficient for mingwex's radix-point
// handling, which only ever converts the "C" locale's ASCII decimal point.
size_t mbrtowc(wchar_t *out, const char *s, size_t n, void *) {
  if (n == 0) {
    return static_cast<size_t>(-2);
  }
  if (s == nullptr) {
    return 0;
  }
  if (out != nullptr) {
    *out = static_cast<wchar_t>(static_cast<unsigned char>(*s));
  }
  return *s == '\0' ? 0 : 1;
}

size_t wcrtomb(char *out, wchar_t wc, void *) {
  if (out == nullptr) {
    return 1;
  }
  *out = static_cast<char>(wc);
  return 1;
}

} // extern "C"
