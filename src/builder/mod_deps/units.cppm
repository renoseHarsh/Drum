module mod_deps:units;

import std;

import glaze;

namespace fs = std::filesystem;

namespace drum::builder_cmd::mod_deps::units {

  struct TranslationUnit {
    fs::path source{};
    fs::path object{};

    struct Module {
      std::string name{};
      fs::path bmi{};
    };
    std::optional<Module> module{};

    std::vector<std::string> imports{};
  };

  namespace {

    using SourceObject = std::pair<fs::path, fs::path>;

    std::expected<TranslationUnit, std::string>
    make_translation_unit(glaze::p1689_types::Rule &rule,
                          const std::vector<SourceObject> &source_objects) {
      fs::path output{std::move(rule.primary_output)};

      auto mapping = std::ranges::find_if(
          source_objects, [&](const auto &so) { return so.second == output; });

      if (mapping == source_objects.end()) [[unlikely]] {
        return std::unexpected{
            std::format("No source mapped to output: {}", output.string())};
      }

      std::error_code ec{};
      if (!fs::exists(mapping->first, ec)) [[unlikely]] {
        return std::unexpected{std::format(
            "Source file not accessible: {} ({})", mapping->first.string(),
            ec ? ec.message() : "does not exist")};
      }

      TranslationUnit unit{
          .source = mapping->first,
          .object = mapping->second,
      };

      if (rule.provides && !rule.provides->empty()) {
        fs::path bmi{unit.object};
        bmi.replace_extension(".pcm");

        unit.module = TranslationUnit::Module{
            .name = std::move(rule.provides->front().logical_name),
            .bmi = std::move(bmi)};
      }

      if (rule.requires_modules) {
        unit.imports.reserve(rule.requires_modules->size());

        std::ranges::transform(
            *rule.requires_modules, std::back_inserter(unit.imports),
            [](auto &dep) { return std::move(dep.logical_name); });
      }

      return unit;
    }

  } // namespace

  std::expected<std::vector<TranslationUnit>, std::string>
  collect_translation_units(glaze::p1689_types::DependencyInfo info,
                            const std::vector<SourceObject> &source_objects) {
    std::vector<TranslationUnit> sources{};
    sources.reserve(info.rules.size());

    for (glaze::p1689_types::Rule &rule : info.rules) {
      auto result = make_translation_unit(rule, source_objects);

      if (!result) [[unlikely]]
        return std::unexpected{std::move(result).error()};

      sources.push_back(std::move(*result));
    }

    return sources;
  }

} // namespace drum::builder_cmd::mod_deps::units
