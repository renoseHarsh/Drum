module builder_cmd;

import std;

import :validate;
import :discover;
import :compile;
import :link;

namespace drum::builder_cmd {
  std::expected<void, std::string> execute(const BuildArgs &_) {
    auto pkg = validate::validate();
    if (!pkg.has_value()) {
      return std::unexpected{std::move(pkg).error()};
    }

    auto src_result = discover::discover();

    auto obj_result = compile::compile(src_result);
    if (!obj_result.has_value()) {
      return std::unexpected{std::move(obj_result).error()};
    }

    if (auto result = link::link(obj_result.value()); !result.has_value()) {
      return std::unexpected{std::move(result).error()};
    }

    return {};
  }
} // namespace drum::builder_cmd
