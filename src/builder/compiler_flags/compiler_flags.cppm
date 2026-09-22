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

    Flags &set_warnings(Manifest::Build::Warnings warnings) {
      using enum Manifest::Build::Warnings;

      switch (warnings) {
      case none:
        args_.push_back("-w");
        break;

      case default_:
        break;

      case all:
        args_.push_back("-Wall");
        args_.push_back("-Wextra");
        break;

      case pedantic:
        args_.push_back("-Wall");
        args_.push_back("-Wextra");
        args_.push_back("-Wpedantic");
        break;
      }

      return *this;
    }

    Flags &set_warnings_as_errors(bool enabled) {
      if (enabled)
        args_.push_back("-Werror");

      return *this;
    }

    Flags &add_extra_flags(std::vector<std::string> flags) {
      args_.append_range(flags | std::views::as_rvalue);

      return *this;
    }

    Flags &set_optimization(Manifest::Profile::Optimization optimization) {
      using enum Manifest::Profile::Optimization;

      constexpr std::array optimizations{
          "-O0",
          "-O1",
          "-O2",
          "-O3",
      };

      args_.push_back(optimizations[static_cast<std::size_t>(
          std::to_underlying(optimization))]);

      return *this;
    }

    Flags &set_debug(bool debug) {
      if (debug)
        args_.push_back("-g");
      else
        args_.push_back("-DNDEBUG");

      return *this;
    }

    const std::vector<std::string> &args() const { return args_; }

  private:
    std::vector<std::string> args_{"-MMD"};
  };
} // namespace drum::compiler_flags
