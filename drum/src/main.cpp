import std;

import parser;
import new_cmd;
import builder_cmd;
import manifest;

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int main(int argc, char *argv[]) {
  if (const auto result = drum::parser::parse_arguments(argc, argv); result) {

    const auto res = std::visit(
        overloaded{[](const drum::new_cmd::NewArgs &arg) {
                     return drum::new_cmd::execute(arg);
                   },
                   [](const drum::builder_cmd::BuildArgs &arg)
                       -> std::expected<void, std::string> {
                     auto manifest = drum::manifest::parse();
                     if (!manifest) {
                       return std::unexpected(std::move(manifest).error());
                     }
                     return drum::builder_cmd::execute(arg, *manifest);
                   },
                   [](const drum::run_cmd::RunArgs &arg)
                       -> std::expected<void, std::string> {
                     auto manifest = drum::manifest::parse();
                     if (!manifest) {
                       return std::unexpected(std::move(manifest).error());
                     }
                     const auto &m = *manifest;
                     if (m.type == drum::manifest::Manifest::Type::lib) {
                       return std::unexpected{"Can't run a library package"};
                     }

                     auto builder_res = drum::builder_cmd::execute({}, m);
                     if (!builder_res) {
                       return std::unexpected{std::move(builder_res).error()};
                     }

                     return drum::run_cmd::execute(arg, m);
                   }},
        *result);

    if (!res) {
      if (!res.error().empty())
        std::println("{}", res.error());
      return 1;
    }
  } else {
    std::println("{}", result.error());
    return 1;
  }
}
