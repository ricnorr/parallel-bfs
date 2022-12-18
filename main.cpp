#include <iostream>
#include <vector>
#include <queue>
#include <atomic>
#include <array>
#include <cilk/cilk.h>
#include <mutex>
#include <cassert>
#include <functional>

using namespace std;
using namespace std::chrono;

int BLOCK = 1000;

vector<int> seq_bfs(const vector<vector<int>>& graph, int start) {
  vector<int> res = vector<int>(graph.size(), 0);
  vector<bool> visited = vector<bool>(graph.size(), false);
  queue<int> q;
  visited[start] = true;
  q.push(start);
  while (!q.empty()) {
    int cur = q.front();
    q.pop();
    for (int i : graph[cur]) {
      if (!visited[i]) {
        visited[i] = true;
        res[i] = res[cur] + 1;
        q.push(i);
      }
    }
  }
  return res;
}

void prefix_sum_build_tree(int treeNode, int l, int r, vector<int>& from, vector<int>& target) {
  if (r - l < BLOCK) {
    int acc = 0;
    for (int i = l; i < r; i++) {
      acc += from[i];
    }
    target[treeNode] = acc;
  } else {
    int m = (l + r) / 2;
    prefix_sum_build_tree(treeNode * 2, l, m, from, target);
    prefix_sum_build_tree(treeNode * 2 + 1, m, r, from, target);
    target[treeNode] = target[treeNode * 2] + target[treeNode * 2 + 1];
  }
}

void prefix_sum_fill_target(int treeNode, int l, int r, int acc, const vector<int>& from, vector<int>& tree, vector<int>& target) {
  if (r - l < BLOCK) {
    int res = acc;
    for (int i = l; i < r; i++) {
      target[i] = res;
      res += from[i];
    }
    target[r] = res;
  } else {
    int m = (l + r) / 2;
    prefix_sum_fill_target(treeNode * 2, l, m, acc, from, tree, target);
    prefix_sum_fill_target(treeNode * 2 + 1, m, r, acc + tree[treeNode * 2], from, tree, target);
  }
}

vector<int> prefix_sum(vector<int>& arr) {
  vector<int> seg_tree(arr.size() * 2 + 1, 0);
  vector<int> res(arr.size() + 1, 0);
  prefix_sum_build_tree(1, 0, arr.size(), arr, seg_tree);
  prefix_sum_fill_target(1, 0, arr.size(), 0, arr, seg_tree, res);
  return res;
}

vector<int> filter(const vector<int>& x, const function<bool(int)>& predicate) {
  vector<int> flags(x.size(), 0);
  cilk_for(int i = 0; i < x.size(); i++) {
    if (predicate(x[i])) {
      flags[i] = true;
    }
  }
  vector<int> indexes = prefix_sum(flags);
  vector<int> res(indexes[indexes.size() - 1], -1);
  cilk_for(int i = 0; i < x.size(); i++) {
    if (flags[i] == 1) {
      int pos = indexes[i];
      res[pos] = x[i];
    }
  }
  return res;
}

vector<int> par_bfs(const vector<vector<int>>& graph, int start) {
  vector<atomic_bool> a(graph.size());
  vector<int> result(graph.size(), 0);
  vector<int> frontier;
  frontier.push_back(start);
  bool expect = false;
  a[start].compare_exchange_strong(expect, true);
  while (!frontier.empty()) {
    vector<int> deg(frontier.size(), 0);
    cilk_for(int i = 0; i < frontier.size(); i++) {
      deg[i] = graph[frontier[i]].size();
    }
    vector<int> f = prefix_sum(deg);
    vector<int> new_frontier(f[f.size() - 1], -1);
    cilk_for(int i = 0; i < frontier.size(); i++) {
      int frontier_el = frontier[i];
      int shift = f[i];
      for (int v : graph[frontier_el]) {
        bool exp = false;
        if (a[v].compare_exchange_strong(exp, true)) {
          result[v] = result[frontier_el] + 1;
          new_frontier[shift] = v;
          shift++;
        }
      }
    }
    frontier = filter(new_frontier, [](int x) {
      return x != -1;
    });
  }
  return result;
}



//int main() {
//  vector<int> x = {1, 2, 3, 4, 5, 7, 8, 9, 10};
//  auto y = prefix_sum(x);
//  for (auto t : y) {
//    cout << t << " ";
//  }
//}
int main() {
  int size = 500;
  vector<vector<int>> graph;
  graph.reserve((size * size * size) + 1);
  int cnt = 0;
  for (int x = 0; x < size; x++) {
    for (int y = 0; y < size; y++) {
      for (int z = 0; z < size; z++) {
        vector<int> cur_edges = {};
        if (x < size - 1) {
          cur_edges.push_back((x + 1) + (size * y) + (size * size * z));
          cnt++;
        }
        if (x > 0) {
          cur_edges.push_back((x - 1) + (size * y) + (size * size * z));
          cnt++;
        }
        if (y < size - 1) {
          cur_edges.push_back(x + (size * (y + 1)) + (size * size * z));
          cnt++;
        }
        if (y > 0) {
          cur_edges.push_back((x  + (size * (y - 1)) + (size * size * z)));
          cnt++;
        }
        if (z < size - 1) {
          cur_edges.push_back(x + (size * y) + (size * size * (z + 1)));
          cnt++;
        }
        if (z > 0) {
          cur_edges.push_back((x  + (size * y) + (size * size * (z - 1))));
          cnt++;
        }
        cur_edges.resize(cur_edges.size());
        graph.push_back(cur_edges);
        std::cout << x << " " << y << " " << z << endl;
        std::cout << "total count " << cnt << endl;
      }
    }
  }

//  auto res1 = seq_bfs(graph, 0);
//  auto res2 = par_bfs(graph, 0);
//  assert(res1 == res1);

  long parSum = 0;
  for (int i = 0; i < 5; i++) {
    auto start_par = high_resolution_clock::now();
    par_bfs(graph, 0);
    auto stop_par = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop_par - start_par);
    std::cout << "duration_par: " << duration.count() << std::endl;
    parSum += duration.count();
  }

  cout << "duration_par_avg: " << parSum / 5 << std::endl;

  long seqSum = 0;
  for (int i = 0; i < 5; i++) {
    auto start_seq = high_resolution_clock::now();
    seq_bfs(graph, 0);
    auto stop_seq = high_resolution_clock::now();
    auto duration_seq = duration_cast<milliseconds>(stop_seq - start_seq);
    std::cout << "duration_seq: " << duration_seq.count() << std::endl;
    seqSum += duration_seq.count();
  }

  cout << "duration_seq_avg: " << seqSum / 5 << std::endl;

  return 0;
}

