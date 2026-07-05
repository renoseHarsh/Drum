import std;
import parser;
import executor;

int main(int argc, char *argv[]) {
  if (const auto result = drum::parser::parse_arguments(argc, argv); result) {
    auto res = std::visit(drum::executor::execute, result.value());
    if (res) {
      std::println("{}", res.value());
      return 1;
    }
  } else {
    std::println("{}", result.error());
    return 1;
  }
}
