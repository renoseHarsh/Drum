module builder_cmd;

import std;

import :validate;

namespace drum::builder_cmd {
  std::expected<void, std::string> execute(const BuildArgs &_) {
    auto pkg = validate::validate();
    if (!pkg.has_value()) {
      return std::unexpected{std::move(pkg).error()};
    }
    return std::unexpected{"Not implemented"};
  }
} // namespace drum::builder_cmd
