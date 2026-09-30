export module compile_unit;

import std;

namespace fs = std::filesystem;

export namespace drum::builder_cmd::compile_unit {

  struct BMI {
    std::string logical_name;
    fs::path path;
  };

  struct TranslationUnit {
    fs::path source{};
    fs::path object{};

    std::vector<BMI> dependency_bmis{};
  };

} // namespace drum::builder_cmd::compile_unit
