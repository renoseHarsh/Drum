export module manifest;

import std;

import tomlplusplus;

export namespace drum::manifest {
  struct Manifest {
    enum class Type { lib, exec };
    std::string name{};
    std::string version{};
    Type type{};
    std::filesystem::file_time_type timestamp;
  };
  std::expected<Manifest, std::string> parse();
}; // namespace drum::manifest
