module;

#include <catch2/catch_test_macros.hpp>

module module_deps:test_flatten;

import std;

import :flatten;
import :node;

namespace drum::builder_cmd::module_deps::flatten::test {

  namespace {

    node::Node a{"a.cpp"};
    node::Node b{"b.cpp"};
    node::Node c{"c.cpp"};
    node::Node d{"d.cpp"};
    node::Node e{"e.cpp"};
    node::Node f{"f.cpp"};

  } // namespace

  TEST_CASE("Empty nodes") {
    std::vector<node::Node *> nodes_ptr{};

    flatten_transitive(nodes_ptr);

    REQUIRE(nodes_ptr.empty());
  }

  TEST_CASE("Linear chain accumulates full chain") {
    std::vector<node::Node> nodes{a, b, c, d};

    auto &a = nodes[0];
    auto &b = nodes[1];
    auto &c = nodes[2];
    auto &d = nodes[3];

    a.dependents.push_back(&b);
    b.dependencies.push_back(&a);

    b.dependents.push_back(&c);
    c.dependencies.push_back(&b);

    c.dependents.push_back(&d);
    d.dependencies.push_back(&c);

    std::vector<node::Node *> sorted{&a, &b, &c, &d};

    flatten_transitive(sorted);

    REQUIRE(a.transitive_deps.empty());
    REQUIRE(b.transitive_deps == std::set<node::Node *>{&a});
    REQUIRE(c.transitive_deps == std::set<node::Node *>{&a, &b});
    REQUIRE(d.transitive_deps == std::set<node::Node *>{&a, &b, &c});
  }

  TEST_CASE("Two disconnected diamonds have no duplicates or leaks") {
    std::vector<node::Node> nodes{a, b, c, d, e, f};
    auto &a = nodes[0];
    auto &b = nodes[1];
    auto &c = nodes[2];
    auto &d = nodes[3];
    auto &e = nodes[4];
    auto &f = nodes[5];

    a.dependents.push_back(&b);
    a.dependents.push_back(&c);

    b.dependencies.push_back(&a);
    c.dependencies.push_back(&a);

    d.dependencies.push_back(&b);
    d.dependencies.push_back(&c);

    b.dependents.push_back(&d);
    c.dependents.push_back(&d);

    e.dependents.push_back(&f);
    f.dependencies.push_back(&e);

    std::vector<node::Node *> sorted{&a, &b, &c, &d, &e, &f};

    flatten_transitive(sorted);

    REQUIRE(a.transitive_deps.empty());
    REQUIRE(b.transitive_deps == std::set<node::Node *>{&a});
    REQUIRE(c.transitive_deps == std::set<node::Node *>{&a});
    REQUIRE(d.transitive_deps == std::set<node::Node *>{&a, &b, &c});

    REQUIRE(e.transitive_deps.empty());
    REQUIRE(f.transitive_deps == std::set<node::Node *>{&e});

    REQUIRE(!std::ranges::contains(d.transitive_deps, &e));
    REQUIRE(!std::ranges::contains(d.transitive_deps, &f));
    REQUIRE(!std::ranges::contains(f.transitive_deps, &a));
    REQUIRE(!std::ranges::contains(f.transitive_deps, &b));
    REQUIRE(!std::ranges::contains(f.transitive_deps, &c));
    REQUIRE(!std::ranges::contains(f.transitive_deps, &d));
  }

} // namespace drum::builder_cmd::module_deps::flatten::test
