export module test_environment;

import std;

namespace fs = std::filesystem;

export namespace drum::test_env {
  class TestEnvironment {
  public:
    TestEnvironment()
        : old_cwd{fs::current_path()},
          temp_dir{fs::temp_directory_path() / "drum_test"} {
      fs::create_directories(temp_dir);
      fs::current_path(temp_dir);
    }

    ~TestEnvironment() {
      fs::current_path(old_cwd);
      fs::remove_all(temp_dir);
    }

    TestEnvironment(const TestEnvironment &) = delete;
    TestEnvironment &operator=(const TestEnvironment &) = delete;
    TestEnvironment(TestEnvironment &&) = delete;
    TestEnvironment &operator=(TestEnvironment &&) = delete;

  private:
    fs::path temp_dir{};
    fs::path old_cwd{};
  };
} // namespace drum::test_env
