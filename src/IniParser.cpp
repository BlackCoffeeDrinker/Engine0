
#include "IniParser.hpp"

#include <EngineError.hpp>

#include <ini.h>

namespace e00::impl {

std::error_code IniParser::Parse(const std::unique_ptr<Stream> &stream, const std::function<std::error_code(const Item &)>& fn) {
  // Make sure we have a stream
  if (!stream) {
    return make_error_code(EngineErrorCode::invalid_argument);
  }

  // Configure the reader
  const ini_reader reader = [](char *str, int num, void *streamuser) -> char * {
    auto *s = static_cast<Stream *>(streamuser);
    return s->read_line_into(str, num);
  };

  // User data structure to pass to the handler
  struct HandlerUserData {
    const std::function<std::error_code(const Item &)>& fn;
    std::error_code errc;
  };

  // Handler
  const ini_handler handler = [](void *user, const char *section, const char *name, const char *value) -> int {
    auto *hud = static_cast<HandlerUserData *>(user);

    if (!hud->errc) {
      Item item;
      item.category = section ? std::string_view(section) : std::string_view();
      item.key = std::string_view(name);
      item.value = std::string_view(value);

      hud->errc = hud->fn(item);
    }

    return hud->errc ? 0 : 1;
  };

  // Make user data struct
  HandlerUserData hud { fn, {} };

  // Parse
  switch (ini_parse_stream(reader, stream.get(), handler, &hud)) {
    case 0:// No error
      return {};

    case -1:// Cannot open file
      return make_error_code(EngineErrorCode::bad_configuration_file);

    case -2:// Memory error
      return make_error_code(EngineErrorCode::out_of_memory);

    default:
      // Non zero indicates which lines the error happens,
      // if we have custom error report it otherwise, report generic error
      if (hud.errc) return hud.errc;
      return make_error_code(EngineErrorCode::bad_configuration_file);
  }

  // Unreachable
}
}// namespace e00::impl
