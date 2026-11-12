module builder_cmd:compile;

import std;

import :log;
import :cache;

import process;
import compiler_flags;
import compile_unit;

namespace fs = std::filesystem;

namespace drum::builder_cmd::compile {

  namespace {
    std::expected<void, std::string>
    compile_unit(const compile_unit::TranslationUnit &unit,
                 const compiler_flags::Flags &flags) {
      auto source_args = std::array{std::string{"-c"}, unit.source.string(),
                                    std::string("-o"), unit.object.string()};

      auto transform_bmi_to_arg = [](const compile_unit::BMI &bmi) {
        return std::format("-fmodule-file={}={}", bmi.logical_name,
                           bmi.path.string());
      };

      auto bmi_args = unit.dependency_bmis |
                      std::views::transform(transform_bmi_to_arg) |
                      std::ranges::to<std::vector>();

      auto args =
          std::array<std::span<const std::string>, 3>{std::span{flags.args()},
                                                      std::span{source_args},
                                                      std::span{bmi_args}} |
          std::views::join;

      return process::run_process("clang++", args).transform([](const auto &) {
      });
    }

  } // namespace

  std::expected<std::vector<fs::path>, std::string>
  compile(std::vector<compile_unit::TranslationUnit> units,
          const compiler_flags::Flags &flags,
          fs::file_time_type manifest_lastwrite) {
    std::vector<fs::path> objects{};
    objects.reserve(units.size());

    for (auto &unit : units) {

      if (cache::unit_is_stale(unit, manifest_lastwrite)) {
        log::compile(unit.object);
        if (const auto parent = unit.object.parent_path(); !parent.empty()) {
          std::error_code ec;
          fs::create_directories(parent, ec);
          if (ec)
            return std::unexpected{"Error in creating build directory"};
        }

        if (auto result = compile_unit(unit, flags); !result)
          return std::unexpected{std::move(result).error()};

      } else
        log::cache_hit(unit.object);

      objects.push_back(std::move(unit.object));
    }

    return objects;
  }
} // namespace drum::builder_cmd::compile
