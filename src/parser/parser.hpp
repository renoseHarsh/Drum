#ifndef PARSER_HPP
#define PARSER_HPP

#include <expected>
#include <variant>

#include "args.hpp"

namespace parser {
  using Error = std::string;
  template <typename T> using Result = std::expected<T, Error>;

  using Command = std::variant<args::NewArgs>;

  Result<Command> parse_arguments(int argc, const char *const argv[]);

} // namespace parser

#endif // !PARSER_H
