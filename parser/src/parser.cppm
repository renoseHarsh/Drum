export module parser;
export import commands;
import std;

export namespace drum::parser {
  using Command = std::variant<commands::NewArgs>;
  std::expected<Command, std::string> parse_arguments(int argc,
                                                      const char *const argv[]);
} // namespace drum::parser
