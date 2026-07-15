module executor;
import filesystem;
import blueprint;

namespace drum::executor {
  std::optional<std::string> execute_new(const commands::NewArgs &cmd) {
    auto package = bp::create_new_package(
        cmd.pkg_name, cmd.pkg_type == commands::NewArgs::PackageType::library);
    if (auto error_msg = fs::build_file_tree(package)) {
      return error_msg.value();
    }
    return std::nullopt;
  }
} // namespace drum::executor
