export module parser;

export import new_cmd;
export import builder_cmd;
export import run_cmd;

import std;

export namespace drum::parser {
  using Command =
      std::variant<new_cmd::NewArgs, builder_cmd::BuildArgs, run_cmd::RunArgs>;
  std::expected<Command, std::string> parse_arguments(int argc,
                                                      const char *const argv[]);
} // namespace drum::parser
