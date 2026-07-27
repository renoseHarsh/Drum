module parser;

import std;

import new_cmd;
import builder_cmd;

namespace drum::parser {
  namespace {
    std::expected<new_cmd::NewArgs, std::string>
    parse_new_args(int argc, const char *const argv[]) {
      if (argc <= 2)
        return std::unexpected{"new command missing package name"};

      new_cmd::NewArgs new_args{};
      for (int i{2}; i < argc; i++) {
        const std::string_view arg{argv[i]};
        if (arg == "--lib") {
          new_args.pkg_type = new_cmd::NewArgs::PackageType::library;
        } else if (arg.starts_with("--")) {
          return std::unexpected{
              std::format("Unknown option for new: {}", arg)};
        } else if (new_args.pkg_name.empty()) {
          if (!std::ranges::all_of(
                  arg, [](char c) { return std::isalpha(c) || c == '_'; }))
            return std::unexpected(std::format("Invalid package name {}", arg));

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

  std::expected<Command, std::string>
  parse_arguments(int argc, const char *const argv[]) {
    if (argc <= 1)
      return std::unexpected{"Provide a command"};

    const std::string cmd{argv[1]};
    if (cmd == "new") {
      return parse_new_args(argc, argv);
    } else if (cmd == "build") {
      return builder_cmd::BuildArgs{};
    } else if (cmd == "run") {
      return run_cmd::RunArgs{};
    }
    return std::unexpected{std::format("Unknown command: {}", cmd)};
  }

} // namespace drum::parser
