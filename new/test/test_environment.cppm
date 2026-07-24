module;

#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

module new_cmd:test_environment;

import std;

namespace drum::new_cmd::test_env {
  void check_valid_dir(const std::filesystem::path &path) {
    REQUIRE(std::filesystem::exists(path));
    REQUIRE(std::filesystem::is_directory(path));
  }

  class TestEnvironment : public Catch::EventListenerBase {
  public:
    using Catch::EventListenerBase::EventListenerBase;
    void testRunStarting(Catch::TestRunInfo const &) override {
      temp_dir = std::filesystem::temp_directory_path() / "drum_test";
      std::filesystem::remove_all(temp_dir);
      std::filesystem::create_directory(temp_dir);

      old_cwd = std::filesystem::current_path();
      std::filesystem::current_path(temp_dir);
    }

    void testRunEnded(Catch::TestRunStats const &) override {
      std::filesystem::current_path(old_cwd);
      std::filesystem::remove_all(temp_dir);
    }

  private:
    using path = std::filesystem::path;
    path temp_dir;
    path old_cwd;
  };
  CATCH_REGISTER_LISTENER(TestEnvironment);
} // namespace drum::new_cmd::test_env
