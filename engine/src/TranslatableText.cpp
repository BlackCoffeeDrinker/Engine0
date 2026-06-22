
#include "TranslatableText.hpp"

namespace e00 {
TranslatableText::TranslatableText() {
}
bool TranslatableText::LocaleInMemory(std::string_view locale) const {
  return false;
}
void TranslatableText::ClearLocale(std::string_view locale) {
}
bool TranslatableText::AddText(std::string_view locale, int textCode, std::string_view text) {
}
void TranslatableText::SetLocale(std::string_view locale) {
}
std::string TranslatableText::GetText(int textCode) {
}
} // e00
