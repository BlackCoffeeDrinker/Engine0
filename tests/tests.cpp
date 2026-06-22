#include <catch2/catch_all.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "tests.hpp"

#include "Loaders/BitmapLoader.hpp"
#include "Loaders/WorldLoader.hpp"
#include "Loaders/PngLoader.hpp"

class SetupResourceManager : public Catch::EventListenerBase {
public:
  using Catch::EventListenerBase::EventListenerBase;

  void testRunStarting(Catch::TestRunInfo const &) override {
    e00::StreamFactory::GlobalStreamFactory().SetResourceDirectory("");
    (void) e00::ResourceManager::GlobalResourceManager().AddLoader<e00::impl::BitmapLoader>();
    (void) e00::ResourceManager::GlobalResourceManager().AddLoader<e00::impl::PNGLoader>();
    (void) e00::ResourceManager::GlobalResourceManager().AddLoader<e00::impl::WorldLoader>();
  }
};

CATCH_REGISTER_LISTENER(SetupResourceManager)

namespace platform {
std::unique_ptr<e00::LoggerSink> CreateSink(const std::string &name) {
  class Sink : public e00::LoggerSink {
    std::string _name;

  public:
    explicit Sink(std::string name) : _name(std::move(name)) {
    }

    void log(const e00::detail::LogMessage &msg) override {
      fprintf(stderr, "[%s] %s\n", _name.c_str(), msg.payload.data());
      fflush(stderr);
    }

    void flush() override {
      fflush(stderr);
    }
  };
  return std::make_unique<Sink>(name);
}

std::unique_ptr<e00::Stream> OpenStream(const std::string_view &name) {
  return TestFileStream::CreateFromFilename(name, false);
}

std::unique_ptr<e00::WritableStream> OpenStreamForWrite(const std::string_view &name) {
  return TestFileStream::CreateFromFilename(name, true);
}


}// namespace platform
