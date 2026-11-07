export module compiler_flags;

import std;

export import manifest;

using Manifest = drum::manifest::Manifest;

export namespace drum::compiler_flags {
  class Flags {
  public:
    Flags &add_include_directory(const std::filesystem::path &directory) {
      args_.push_back(std::format("-I{}", directory.string()));

      return *this;
    }

    Flags &set_standard(Manifest::Build::Standard standard) {
      using enum Manifest::Build::Standard;

      constexpr std::array standards{
          "c++11", "c++14", "c++17", "c++20", "c++23", "c++26",
      };

      const auto index = static_cast<std::size_t>(std::to_underlying(standard));

      args_.push_back(std::format("-std={}", standards[index]));

      return *this;
    }

    Flags &add_flags(const std::vector<std::string> &args) {
      args_.append_range(args);
      return *this;
    }

    const std::vector<std::string> &args() const { return args_; }

  private:
    std::vector<std::string> args_{"-MMD"};
  };
} // namespace drum::compiler_flags
