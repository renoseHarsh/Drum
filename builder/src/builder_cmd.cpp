module builder_cmd;

import std;

import tomlplusplus;

namespace drum::builder_cmd {
  std::expected<void, std::string> execute(const BuildArgs &_) {
    return std::unexpected{"Not Implemented"};
  }
} // namespace drum::builder_cmd
