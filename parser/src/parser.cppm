export module parser;
export import commands;
import std;

namespace drum::parser {
  export using Command = std::variant<commands::NewArgs>;
  export std::expected<Command, std::string>
  parse_arguments(int argc, const char *const argv[]);

} // namespace drum::parser
