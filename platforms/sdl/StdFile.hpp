#pragma once

#include <cstdio>
#include <string>
#include <memory>

#include <Engine.hpp>

class StdFile : public e00::Stream {
  FILE *const _file;

public:
  using e00::Stream::read;

  static std::unique_ptr<StdFile> CreateFromFilename(const std::string &fileName);

  explicit StdFile(FILE *fp, long size);

  ~StdFile() override;

protected:
  std::error_code real_read(size_t size, void *data) override;

  std::error_code real_seek(size_t size) override;
};
