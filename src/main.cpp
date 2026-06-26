#include "command/command.hpp"
#include "parser/parser.hpp"
#include <print>
int main(int argc, char *argv[]) {
  if (const auto result = parser::parse_arguments(argc, argv); result) {
    std::visit(command::execute, result.value());
  } else {
    std::println("{}", result.error());
  }
}
