#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using branch_t = std::vector<std::string>;
using branches_t = std::vector<branch_t>;

struct CallGrapNode;
using CallGraph = std::map<std::string, CallGrapNode>;

struct CallGrapNode {
  CallGrapNode() {}
  CallGrapNode(const CallGraph &t) : children(t) {}
  CallGraph children;
};

using CallGraph = std::map<std::string, CallGrapNode>;

const char *ws = " \t\n\r\f\v";

inline std::string &rtrim(std::string &s, const char *t = ws) {
  s.erase(s.find_last_not_of(t) + 1);
  return s;
}

inline std::string &ltrim(std::string &s, const char *t = ws) {
  s.erase(0, s.find_first_not_of(t));
  return s;
}

inline std::string &trim(std::string &s, const char *t = ws) {
  return ltrim(rtrim(s, t), t);
}

void traverse_branches(const branches_t &branches) {
  std::cout << "number of branches:" << branches.size() << std::endl;
  for (const auto &branch : branches) {
    auto reversed_branch = branch;
    std::reverse(reversed_branch.begin(), reversed_branch.end());
    for (const auto &call : reversed_branch) {
      std::cout << call << std::endl;
    }
  }
}

void merge_subtree(CallGraph &callgraph, const branch_t &branch) {
  CallGraph *current = &callgraph;
  for (int index = 0; index != branch.size(); ++index) {
    CallGraph children;
    auto ins_res = current->insert(std::make_pair(branch[index], children));
    current = &ins_res.first->second.children;
  }
}

CallGraph merge_branches(const branches_t &branches) {
  CallGraph result;
  for (const auto &branch : branches) {
    auto reversed_branch = branch;
    std::reverse(reversed_branch.begin(), reversed_branch.end());
    merge_subtree(result, reversed_branch);
  }
  return result;
}

void traverse_graph(const CallGraph &graph, int &depth) {
  for (auto it = graph.begin(); it != graph.end(); it++) {
    for (int i = 0; i != depth; ++i) {
      std::cout << "  ";
    }
    std::cout << it->first << std::endl;
    ++depth;
    traverse_graph(it->second.children, depth);
    --depth;
  }
}

void test() {
  CallGraph callgraph = {{"a", CallGrapNode({
                                   {"b", CallGrapNode({{"d", CallGrapNode()}})},
                                   {"c", CallGrapNode()},
                               })}};
  int depth = 0;
  CallGraph copy;
  branch_t b1 = {"a", "b", "c"};
  branch_t b2 = {"a", "c", "c"};
  merge_subtree(copy, b1);
  merge_subtree(copy, b2);
  traverse_graph(copy, depth);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "driver filename" << std::endl;
    return -1;
  }
  std::ifstream in(argv[1]);
  if (!in.is_open()) {
    throw 1;
  }
  std::cout << argv[1] << std::endl;
  std::string line;
  branches_t branches;
  while (std::getline(in, line)) {
    if (line.find("stack:") != std::string::npos) {
      branch_t branch;
      std::string call =
          line.substr(std::string("stack:").size(),
                      line.size() - std::string("stack:").size());
      branch.insert(branch.begin(), trim(call));
      branches.push_back(branch);
    } else {
      branches.back().push_back(trim(line));
    }
  }
  auto mergedGraph = merge_branches(branches);
  int depth = 0;
  traverse_graph(mergedGraph, depth);
  return 0;
}
