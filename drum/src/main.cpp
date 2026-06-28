#include "executor/executor.hpp"
#include "parser/parser.hpp"
#include <print>
int main(int argc, char *argv[]) {
  if (const auto result = drum::parser::parse_arguments(argc, argv); result) {
    std::visit(drum::executor::execute, result.value());
  } else {
    std::println("{}", result.error());
    return 1;
  }
}
