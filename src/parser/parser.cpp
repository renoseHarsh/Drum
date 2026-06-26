#include "parser/parser.hpp"
#include <format>

namespace parser {

  namespace {
    Result<args::NewArgs> parse_new_args(int argc, const char *const argv[]) {
      if (argc <= 2)
        return std::unexpected{"new command missing package name"};

      args::NewArgs new_args{};
      for (size_t i{2}; i < argc; i++) {
        std::string_view arg = argv[i];
        if (arg == "--lib") {
          new_args.pkg_type = args::NewArgs::PackageType::library;
        } else if (arg.starts_with("--")) {
          return std::unexpected{
              std::format("Unknown option for new: {}", arg)};
        } else if (new_args.pkg_name.empty()) {
          new_args.pkg_name = arg;
        } else
          return std::unexpected{std::format("Unknown arg: {}", arg)};
      }

      if (new_args.pkg_name.empty()) {
        return std::unexpected{"new command missing package name"};
      }

      return new_args;
    }

  } // namespace

  Result<Command> parse_arguments(int argc, const char *const argv[]) {
    if (argc <= 1)
      return std::unexpected{"Provide a command"};

    std::string cmd = argv[1];
    if (cmd == "new") {
      return parse_new_args(argc, argv);
    }
    return std::unexpected{std::format("Unknown command: {}", cmd)};
  }

} // namespace parser
