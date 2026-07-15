import std;
import parser;
import executor;

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int main(int argc, char *argv[]) {
  if (const auto result = drum::parser::parse_arguments(argc, argv); result) {

    auto res = std::visit(
        overloaded{
            [](const drum::commands::NewArgs &arg) {
              return drum::executor::execute_new(arg);
            },
        },
        result.value());

    if (res) {
      std::println("{}", res.value());
      return 1;
    }
  } else {
    std::println("{}", result.error());
    return 1;
  }
}
