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
    // cout << n << "|" << k << endl;
    edges.resize(n);
    for(int i = 0; i < n; i++) {
      int deg = rand() % (k+1);
      int seen = 0;
      int j;
      // Add the first deg nodes.
      for(j = i+1; j < n; j++) {
        if(seen == deg) break;
        seen++;
        // cout << j << " " << deg << endl;
        edges[i].push_back(j);
      }
      // Swap out some nodes with new nodes, with some prob. (Reservoir Sampling)
      for(; j < n; j++) {
        int swap = rand() % (seen+1);
        // cout << swap << "~~~" << endl;
        if(swap >= deg) continue;  // Don't include j in the children.
        seen++;
        edges[i][swap] = j;
      }
    }
    rev_edges.resize(n);
    for(int i = 0; i < n; i++) {
      for(int j : edges[i]) rev_edges[j].push_back(i); 
    }
  }

  bool observed_desc(int i, vector<int>& visited,
                     const vector<int> observed,
                     vector<int>& obs_desc) {
    visited[i] = 1;
    bool res = false;
    if(observed[i]) res = true;
    for(int j : edges[i]) {
      if(visited[j]) continue;
      res |= observed_desc(j,visited,observed,obs_desc);
    }
    obs_desc[i] = res;
    return res;
  }

  bool active_trail(int v, int p, int dest,vector<int>& active,
                    vector<int> &visited, const vector<int>& od,
                    const vector<int> observed, vector<int>& trail) {
    trail.push_back(v);
    active[v] = 1;
    if (v == dest) return true;
    bool fwd = false, rev = false;
    if(p == -1) {
      // this is the source vertex.
      visited[v] = 3;
      fwd = rev = true;
    }
    else {
      visited[v] += p;
    }
    if(p == 1) {
      if(!observed[v]){
        fwd = true;
      }
      if(od[v]) {
        rev = true;
      }
    }
    else if (p==2 && !observed[v]) {
        fwd = rev = true;
    }
    if(fwd) {
      for(int i = 0; i < edges[v].size(); i++) {
        int j = edges[v][i];
        if(visited[j] == 1 || visited[j] == 3 || active[j]) continue;
        if(active_trail(j,1,dest,active,visited,od,observed,trail)) return true;
      }
    }
    if(rev) {
      for(int i = 0; i < rev_edges[v].size(); i++) {
        int j = rev_edges[v][i];
        if(visited[j] == 2 || visited[j] == 3 || active[j]) continue;
        if(active_trail(j,2,dest,active,visited,od,observed,trail)) return true;
      }
    }
    active[v] = 0;
    trail.pop_back();
    return false;
  }
  vector<int> ActiveTrail(int u, int v, const vector<int>& obs) {
    vector<int> observed(n, 0);
    for(int v : obs) observed[v] = 1;
    if(observed[u] || observed[v]) return {};
    vector<int> od(n,0);
    vector<int> visited(n, 0);
    for(int i = 0; i < n; i++) {
      if(visited[i]) continue;
      observed_desc(i, visited, observed, od);
    }
    cout<<"~~~~"<<od[2]<<endl;
    vector<int> res;
    fill(visited.begin(), visited.end(), 0);
    vector<int> active(n,0);
    if(active_trail(u,-1,v,active,visited,od,observed,res)) {
      return res;
    }
    else {
      assert(res.size() == 0);
      return {};
    }
  }
};

void query(BNet& net, const string& input, const string& output) {
  ifstream fin(input, ifstream::in);
  ofstream fout(output, ofstream::out);
  string q;
  int i = 0;
  while(getline(fin,q)) {
    stringstream ss(q);
    int u,v;
    ss >> u >> v;
    u--; v--;
    cout << "Solving query :" << q << endl;
    cout << "source : " << u << "dest: " << v << endl;
    string list;
    ss >> list;
    vector<int> observed;
    ParseList(list, &observed);
    cout << "Observed: ";
    for(int i : observed) cout << i <<" ";
    cout << endl;
    vector<int> trail = net.ActiveTrail(u,v,observed);
    if(i>0) fout << endl;
    fout << q << endl;
    if(trail.size()) {
      fout << "no [";
      for(int i = 0; i < trail.size(); i++){
        if(i > 0) fout << ",";
        fout << trail[i]+1;
      }
      fout << "]";
    }
    else fout << "yes";
    i++;
  }
}
int main(int argc, char** argv) {
  // srand(time(NULL));
  // cin >> filename;
  BNet net;
  assert(argc == 4);
  if(atoi(argv[1]) == 1) {
    int n = atoi(argv[2]), k = atoi(argv[3]);
    net.GenerateRandom(n, k);
    net.Dump("bn.txt");
  }
  else {
    string input(argv[2]), q(argv[3]);
    net.LoadFrom(input);
    net.Print();
    query(net, q, "out.txt");
  }
  return 0;
}