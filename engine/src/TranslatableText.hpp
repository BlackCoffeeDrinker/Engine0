#pragma once

#include <string>
#include <string_view>

namespace e00 {

/**
 * Class TranslatableText
 */
class TranslatableText {

public:
  TranslatableText();

  ~TranslatableText() = default;

  bool LocaleInMemory(std::string_view locale) const;

  void ClearLocale(std::string_view locale);

  bool AddText(std::string_view locale, int textCode, std::string_view text);
  
  void SetLocale(std::string_view locale);

  std::string GetText(int textCode);
};

}// namespace e00
