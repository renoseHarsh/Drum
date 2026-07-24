import std;

import parser;
import new_cmd;

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int main(int argc, char *argv[]) {
  if (const auto result = drum::parser::parse_arguments(argc, argv); result) {

    auto res =
        std::visit(overloaded{[](const drum::new_cmd::NewArgs &arg) {
                                return drum::new_cmd::execute(arg);
                              },
                              [](const drum::builder_cmd::BuildArgs &arg) {
                                return drum::builder_cmd::execute(arg);
                              }},
                   result.value());

    if (!res.has_value()) {
      std::println("{}", res.error());
      return 1;
    }
  } else {
    std::println("{}", result.error());
    return 1;
  }
}
