module;

#include <catch2/catch_test_macros.hpp>

module module_deps:test_topo_sort;

import std;

import :topo_sort;
import :node;

namespace drum::builder_cmd::module_deps::topo_sort::test {

  namespace {

    node::Node a{"a.cpp"};
    node::Node b{"b.cpp"};
    node::Node c{"c.cpp"};
    node::Node d{"d.cpp"};
    node::Node e{"e.cpp"};
    node::Node f{"f.cpp"};

  } // namespace

  TEST_CASE("Empty vector of nodes") {
    std::vector<node::Node> nodes{};

    auto result = topo_sort(nodes);

    REQUIRE(result);
    REQUIRE(result->empty());
  }

  TEST_CASE("Linear chain has exact order") {
    std::vector<node::Node> nodes{a, b, c, d, e};

    auto &a = nodes[0];
    auto &b = nodes[1];
    auto &c = nodes[2];
    auto &d = nodes[3];
    auto &e = nodes[4];

    a.dependents.push_back(&b);
    b.dependencies.push_back(&a);

    b.dependents.push_back(&c);
    c.dependencies.push_back(&b);

    c.dependents.push_back(&d);
    d.dependencies.push_back(&c);

    d.dependents.push_back(&e);
    e.dependencies.push_back(&d);

    auto result = topo_sort(nodes);

    REQUIRE(result);
    REQUIRE(result->size() == nodes.size());
    REQUIRE(std::ranges::equal(nodes, *result, {}, [](auto &n) { return &n; }));
  }

  TEST_CASE("Diamond with disconnected chain") {
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

    auto result = topo_sort(nodes);

    REQUIRE(result);
    REQUIRE(result->size() == nodes.size());

    REQUIRE(std::ranges::contains(*result, &e));

    auto pos = [&](node::Node *n) { return std::ranges::find(*result, n); };

    REQUIRE(pos(&a) < pos(&b));
    REQUIRE(pos(&a) < pos(&c));
    REQUIRE(pos(&a) < pos(&d));

    REQUIRE(pos(&b) < pos(&d));
    REQUIRE(pos(&c) < pos(&d));

    REQUIRE(pos(&e) < pos(&f));
  }

  TEST_CASE("Independent nodes have no order constraint") {
    std::vector<node::Node> nodes{a, b, c};

    auto result = topo_sort(nodes);

    REQUIRE(result);
    REQUIRE(result->size() == nodes.size());

    REQUIRE(std::ranges::contains(*result, &nodes[0]));
    REQUIRE(std::ranges::contains(*result, &nodes[1]));
    REQUIRE(std::ranges::contains(*result, &nodes[2]));
  }

  TEST_CASE("Cycle with tail is detected") {
    std::vector<node::Node> nodes{a, b, c, d, e, f};
    auto &a = nodes[0];
    auto &b = nodes[1];
    auto &c = nodes[2];
    auto &d = nodes[3];
    auto &e = nodes[4];
    auto &f = nodes[5];

    a.dependents.push_back(&b);
    b.dependencies.push_back(&a);

    b.dependents.push_back(&c);
    c.dependencies.push_back(&b);

    c.dependents.push_back(&d);
    d.dependencies.push_back(&c);

    d.dependents.push_back(&e);
    e.dependencies.push_back(&d);

    e.dependents.push_back(&f);
    f.dependencies.push_back(&e);

    f.dependents.push_back(&c);
    c.dependencies.push_back(&f);

    auto result = topo_sort(nodes);

    REQUIRE_FALSE(result);
  }

} // namespace drum::builder_cmd::module_deps::topo_sort::test
