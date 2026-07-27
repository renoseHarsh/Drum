module builder_cmd;

import std;

import :validate;
import :discover;
import :compile;
import :link;

namespace drum::builder_cmd {
  std::expected<void, std::string> execute(const BuildArgs &_) {
    const auto pkg = validate::validate();
    if (!pkg) {
      return std::unexpected{std::move(pkg).error()};
    }

    const auto src_result = discover::discover();

    const auto obj_result = compile::compile(src_result);
    if (!obj_result) {
      return std::unexpected{std::move(obj_result).error()};
    }

    if (const auto result = link::link(obj_result.value()); !result) {
      return std::unexpected{std::move(result).error()};
    }

    return {};
  }
} // namespace drum::builder_cmd
