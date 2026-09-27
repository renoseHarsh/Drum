module mod_deps;

import std;

import :comp_db;
import :scan_deps;
import :units;
import :node;

import compiler_flags;

namespace fs = std::filesystem;

namespace drum::builder_cmd::mod_deps {

  [[nodiscard]]
  std::expected<void, std::string>
  scan_modules(const std::vector<SourceObject> &source_objects,
               const compiler_flags::Flags &flags, const fs::path &output_dir) {
    return comp_db::generate_compile_database(source_objects, flags, output_dir)
        .and_then(scan_deps::scan_deps)
        .and_then(scan_deps::parse)
        .and_then([&](glaze::p1689_types::DependencyInfo info) {
          return units::collect_translation_units(info, source_objects);
        })
        .transform(node::build_nodes)
        .and_then([](std::vector<node::Node> nodes) {
          return node::build_provide_map(nodes).and_then([&](auto provide_map) {
            return node::link_dependencies(provide_map, nodes);
          });
        })
        .and_then([](auto) -> std::expected<void, std::string> { return {}; });
  }

} // namespace drum::builder_cmd::mod_deps
