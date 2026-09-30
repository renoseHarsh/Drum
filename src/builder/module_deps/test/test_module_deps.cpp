#include <catch2/catch_test_macros.hpp>

import std;

import test_util;
import compile_unit;
import module_deps;
import compiler_flags;

namespace drum::builder_cmd::module_deps::test {

  namespace {
    auto plain = compile_unit::TranslationUnit{"plain.cpp", "plain.cpp.o"};
    auto module_a = compile_unit::TranslationUnit{"alpha.cppm", "alpha.cppm.o"};
    auto module_b = compile_unit::TranslationUnit{"beta.cppm", "beta.cppm.o"};
    auto module_c = compile_unit::TranslationUnit{"gamma.cppm", "gamma.cppm.o"};
    auto import_a =
        compile_unit::TranslationUnit{"importer_a.cpp", "importer_a.cpp.o"};
    auto import_a_b =
        compile_unit::TranslationUnit{"importer_ab.cpp", "importer_ab.cpp.o"};
    auto import_b_c =
        compile_unit::TranslationUnit{"importer_bc.cpp", "importer_bc.cpp.o"};

    compiler_flags::Flags get_flags() {
      compiler_flags::Flags flags{};
      flags.set_standard(manifest::Manifest::Build::Standard::cpp23);

      return flags;
    }

    using Graph = std::vector<std::pair<std::string, std::vector<std::string>>>;
    auto graph(const std::vector<compile_unit::TranslationUnit> &units) {
      auto view = units | std::views::transform([](const auto &unit) {
                    auto deps =
                        unit.dependency_bmis |
                        std::views::transform([](const auto &bmi) {
                          return bmi.logical_name + "=" + bmi.path.string();
                        }) |
                        std::ranges::to<std::vector<std::string>>();

                    std::ranges::sort(deps);

                    return std::pair{unit.source.string(), std::move(deps)};
                  });

      return std::ranges::to<Graph>(view);
    }

  } // namespace

  TEST_CASE("Single source file") {
    test_util::TestEnvironment env{};
    test_util::write_file(plain.source, "");

    auto result = order({plain}, get_flags(), {});

    REQUIRE(result);
    REQUIRE(result->size() == 1);

    REQUIRE((*result)[0].source == plain.source);
  }

  TEST_CASE("Single module dependency") {
    test_util::TestEnvironment env{};
    test_util::write_file(import_a.source, "import alpha;");
    test_util::write_file(module_a.source, "export module alpha;");

    auto result = order({import_a, module_a}, get_flags(), {});

    REQUIRE(result);
    REQUIRE(graph(*result) ==
            Graph{{"alpha.cppm", {}},
                  {"importer_a.cpp", {"alpha=alpha.cppm.pcm"}}});
  }

  TEST_CASE("Multiple levels of dependencies") {
    test_util::TestEnvironment env{};
    test_util::write_file(import_a.source, "import alpha;");
    test_util::write_file(module_a.source,
                          "export module alpha;\nimport beta;");
    test_util::write_file(module_b.source, "export module beta;");

    auto result = order({import_a, module_a, module_b}, get_flags(), {});

    REQUIRE(result);
    REQUIRE(graph(*result) ==
            Graph{{"beta.cppm", {}},
                  {"alpha.cppm", {"beta=beta.cppm.pcm"}},
                  {"importer_a.cpp",
                   {"alpha=alpha.cppm.pcm", "beta=beta.cppm.pcm"}}});
  }

  TEST_CASE("Diamond dependency") {
    test_util::TestEnvironment env{};
    test_util::write_file(module_a.source, "export module alpha;");
    test_util::write_file(module_b.source,
                          "export module beta;\nimport alpha;");
    test_util::write_file(module_c.source,
                          "export module gamma;\nimport alpha;");
    test_util::write_file(import_b_c.source, "import beta;\nimport gamma;");

    auto result =
        order({import_b_c, module_b, module_c, module_a}, get_flags(), {});

    REQUIRE(result);

    auto pos = [&](compile_unit::TranslationUnit &n) {
      return std::ranges::find(*result, n.source,
                               &compile_unit::TranslationUnit::source);
    };

    REQUIRE(pos(module_a) < pos(module_b));
    REQUIRE(pos(module_a) < pos(module_c));
    REQUIRE(pos(module_b) < pos(import_b_c));
    REQUIRE(pos(module_c) < pos(import_b_c));

    auto g = graph(*result);
    std::ranges::sort(g);

    REQUIRE(g == Graph{{"alpha.cppm", {}},
                       {"beta.cppm", {"alpha=alpha.cppm.pcm"}},
                       {"gamma.cppm", {"alpha=alpha.cppm.pcm"}},
                       {"importer_bc.cpp",
                        {"alpha=alpha.cppm.pcm", "beta=beta.cppm.pcm",
                         "gamma=gamma.cppm.pcm"}}});
  }

  TEST_CASE("Independent modules") {
    test_util::TestEnvironment env{};
    test_util::write_file(module_a.source, "export module alpha;");
    test_util::write_file(module_b.source, "export module beta;");
    test_util::write_file(plain.source, "");

    auto result = order({plain, module_a, module_b}, get_flags(), {});

    REQUIRE(result);

    auto g = graph(*result);
    std::ranges::sort(g);

    REQUIRE(g == Graph{
                     {"alpha.cppm", {}},
                     {"beta.cppm", {}},
                     {"plain.cpp", {}},
                 });
  }

  TEST_CASE("Missing imported module") {
    test_util::TestEnvironment env{};
    test_util::write_file(module_a.source, "export module alpha;");
    test_util::write_file(import_a_b.source, "import alpha;\nimport gamma;");

    auto result = order({import_a_b, module_a}, get_flags(), {});

    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Module not found: gamma");
  }

  TEST_CASE("Cyclic module dependencies") {
    test_util::TestEnvironment env{};
    test_util::write_file(module_a.source,
                          "export module alpha;\nimport beta;");
    test_util::write_file(module_b.source,
                          "export module beta;\nimport alpha;");

    auto result = order({module_a, module_b}, get_flags(), {});

    REQUIRE_FALSE(result);
    REQUIRE(result.error().starts_with("Cycle detected: "));
  }

  TEST_CASE("Mixed modules and source files") {
    test_util::TestEnvironment env{};
    test_util::write_file(module_a.source, "export module alpha;");
    test_util::write_file(plain.source, "");
    test_util::write_file(import_a.source, "import alpha;");

    auto result = order({import_a, plain, module_a}, get_flags(), {});

    auto pos = [&](compile_unit::TranslationUnit &n) {
      return std::ranges::find(*result, n.source,
                               &compile_unit::TranslationUnit::source);
    };

    REQUIRE(result);
    REQUIRE(pos(module_a) < pos(import_a));

    auto g = graph(*result);
    std::ranges::sort(g);

    REQUIRE(g == Graph{{"alpha.cppm", {}},
                       {"importer_a.cpp", {"alpha=alpha.cppm.pcm"}},
                       {"plain.cpp", {}}});
  }

} // namespace drum::builder_cmd::module_deps::test
