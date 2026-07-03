#ifndef DRUM_PARSER_HPP
#define DRUM_PARSER_HPP

#include <expected>
#include <string>
#include <variant>

#include "commands/commands.hpp"

namespace drum::parser {
  template <typename T> using Result = std::expected<T, std::string>;

  using Command = std::variant<commands::NewArgs>;

  Result<Command> parse_arguments(int argc, const char *const argv[]);

} // namespace drum::parser

#endif // !DRUM_PARSER_H
