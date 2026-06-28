#include "parser/parser.hpp"
#include <format>

namespace drum::parser {

  namespace {
    Result<commands::NewArgs> parse_new_args(int argc,
                                             const char *const argv[]) {
      if (argc <= 2)
        return std::unexpected{"new command missing package name"};

      commands::NewArgs new_args{};
      for (int i{2}; i < argc; i++) {
        std::string_view arg = argv[i];
        if (arg == "--lib") {
          new_args.pkg_type = commands::NewArgs::PackageType::library;
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

} // namespace drum::parser
