export module parser;
export import commands;
import std;

namespace drum::parser {
  export std::expected<std::variant<commands::NewArgs>, std::string>
  parse_arguments(int argc, const char *const argv[]);

} // namespace drum::parser
