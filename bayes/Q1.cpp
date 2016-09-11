#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdlib>

using namespace std;

void ParseList(const string& list, vector<int>* res) {
  assert(list.size() > 1);
  int n = list.size();
  assert(list[0] == '[' && list[n-1] == ']');
  if(n == 2) return;
  stringstream ss(list);
  ss.get();
  int i;
  while(ss >> i) {  
    if (ss.peek() == ',') ss.ignore();
    res->push_back(i-1);
  }
}

class BNet {
public:
  int n;
  vector<vector<int>> edges;
  vector<vector<int>> rev_edges;
  void LoadFrom(const string &filename) {
    ifstream fin(filename, ifstream::in);
    fin >> n;
    assert(n > 0);
    edges.resize(n);
    int v;  
    while(fin >> v) {
      string list;
      fin >> list;
      v--;
      ParseList(list, &edges[v]);
    }
    rev_edges.resize(n);
    for(int i = 0; i < n; i++) {
      for(int j : edges[i]) rev_edges[j].push_back(i); 
    }
    fin.close();
  }

  void Print() {
    cout << "Total Nodes:" << n << endl;
    for(int i = 0; i < n; i++) {
      cout << i + 1 << "--- ";
      for(int j : edges[i]) cout << j + 1 << " ";
      cout << endl;
    }
  }

  void Dump(const string& filename) {
    ofstream fout(filename, ofstream::out);
    fout << n << endl;
    for(int i = 0; i < n; i++) {
      fout << i + 1 << " [";
      for(int j = 0; j < edges[i].size(); j++) {
        if (j > 0) fout << ",";
        fout << edges[i][j] + 1;
      }
      fout << "]" << endl;
    }
    fout.close();
  }

  void GenerateRandom(int _n, int k) {
    n = _n;
    edges.resize(n);
    for(int i = 0; i < n; i++) {
      int deg = rand() % (k+1);
      int seen = 0;
      int j;
      // Add the first deg nodes.
      for(j = i+1; j < n; j++) {
        if(seen == deg) break;
        seen++;
        edges[i].push_back(j);
      }
      // Swap out some nodes with new nodes, with some prob. (Reservoir Sampling)
      for(; j < n; j++) {
        int swap = rand() % (seen+1);
        if(swap == seen) continue;  // Don't include j in the children.
        seen++;
        edges[i][swap] = j;
      }
    }
    rev_edges.resize(n);
    for(int i = 0; i < n; i++) {
      for(int j : edges[i]) rev_edges[j].push_back(i); 
    }
  }
};

int main() {
  // srand(time(NULL));
  string filename;
  // cin >> filename;
  BNet net;
  //net.LoadFrom("sample-bn.txt");
  net.GenerateRandom(5, 1);
  net.Print();
  net.Dump("sample-dump.txt");
  return 0;
}