module builder_cmd:log;

import std;

namespace fs = std::filesystem;

namespace drum::builder_cmd::log {
  void compile(const fs::path &source) {
    std::println("[compile] {}", source.string());
  }

  void cache_hit(const fs::path &object) {
    std::println("[cached] {}", object.string());
  }

  void archive(const fs::path &output) {
    std::println("[archive] {}", output.string());
  }

  void link(const fs::path &output) {
    std::println("[link] {}", output.string());
  }
} // namespace drum::builder_cmd::log
