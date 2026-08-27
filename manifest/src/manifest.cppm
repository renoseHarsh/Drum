export module manifest;

import std;

import tomlplusplus;

export namespace drum::manifest {
  struct Manifest {
    enum class Type { lib, exec };

    struct Build {
      enum class Standard {
        cpp11,
        cpp14,
        cpp17,
        cpp20,
        cpp23,
        cpp26,
      };

      enum class Warnings { none, default_, all, pedantic };

      Standard standard{Standard::cpp23};
      Warnings warnings{Warnings::default_};
      bool warnings_as_errors{false};
    };

    std::string name{};
    std::string version{};
    Type type{};

    Build build{};

    std::filesystem::file_time_type timestamp{};
  };
  std::expected<Manifest, std::string> parse();
}; // namespace drum::manifest
