#include <iostream>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

char chars[10] = {'e','t','a','o','i','n','s','h','r','d'};
int ten[] = {1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};
class Model {
 public:
  int id[26];
  double ocr[1001][11];
  double trans[11][11];
  double skip_factor1;
  double skip_factor2;
  struct Score {
    double ocr;
    double trans;
    double combined;
    Score() {
      ocr = trans = combined = 0.0;
    }
  };
  Model() {
    fill(id, id+26, -1);
    for(int i = 0; i < 10; i++) id[chars[i]-'a'] = i;
    skip_factor2 = 1.0;
    skip_factor1 = 5.0;
  }
  void LoadModel(const string& ocr_file, const string& trans_file) {
    ifstream fin(ocr_file, ifstream::in);
    string s;
    while(getline(fin,s)) {
      stringstream ss(s);
      int img; char ch; double val;
      ss >> img >> ch >> val;
      ocr[img][id[ch-'a']] = val;
      assert(id[ch-'a'] >= 0);
      assert(img < 1001);
    }
    fin.close();
    ifstream f(trans_file, ifstream::in);
    while(getline(f,s)) {
      stringstream ss(s);
      char ch1, ch2; double val;
      ss >> ch1 >> ch2 >>  val;
      trans[id[ch1-'a']][id[ch2-'a']] = val;
      assert(id[ch1-'a'] >= 0);
      assert(id[ch2-'a'] >= 0);
      cout << ch1 << ch2 << val <<  endl;
    }
    f.close();
  }
  Score getUnNormalized(const vector<int>& img_values, const vector<int>& char_values) {
    int n = img_values.size();
    assert(n == char_values.size());
    Score score;
    for(int i = 0; i < n; i++) {
      score.ocr += log(ocr[img_values[i]][char_values[i]]);
      if(i > 0) {
        score.trans += log(trans[char_values[i]][char_values[i-1]]);
      }
      for(int j = i+1; j < n; j++) {
        if(img_values[i] != img_values[j]) continue;
        if(char_values[j] == char_values[i]) score.combined += log(skip_factor1);
        else score.combined += log(skip_factor2);
      }
    }
    score.trans += score.ocr;
    score.combined += score.trans;
    // score.ocr = pow(2, score.ocr);
    // score.trans = pow(2, score.trans);
    // score.combined = pow(2, score.combined);
    return score;    
  }
  // does everything.
  Score getZ(const vector<int>& img, int* best) {
    int n = img.size();
    int m = ten[n];
    Score total;
    double max_ocr = -1e15, max_trans = -1e15, max_combined = -1e15;
    int m_ocr,m_trans,m_combined;
    vector<int> chr(n);
    // generate all numbers from 0, 10^n - 1;
    for(int w = 0; w < m; w++) {
      int tmp = w;
      for(int i = 0; i < n; i++) {
        chr[i] = tmp%10;
        // cout << chr[i] << " ";
        tmp /= 10;
      }
      // cout<<endl;
      Score score = getUnNormalized(img,chr);
      if(score.ocr > max_ocr) {
        max_ocr = score.ocr;
        m_ocr = w;
      }
      if(score.trans > max_trans) {
        max_trans = score.trans;
        m_trans = w;
      }
      if(score.combined > max_combined) {
        max_combined = score.combined;
        // cout << w << " " << max_combined << endl;
        m_combined = w;
      }
      total.ocr += exp(score.ocr);
      total.trans += exp(score.trans);
      total.combined += exp(score.combined);
    }
    total.ocr /= m;
    total.trans /= m;
    total.combined /= m;
    if(best){
      best[0] = m_ocr;
      best[1] = m_trans;
      best[2] = m_combined;
    }
    cout << "Max prob: " << max_ocr <<" " << max_trans << " " << max_combined << endl;
    return total;
  }
  Score getProb(const vector<int>& img, const string& word) {
    assert(word.size() == img.size());
    vector<int> chr(img.size());
    for(int i = 0; i < chr.size(); i++) {
      chr[i] = id[word[i]-'a'];
    }
    Score score = getUnNormalized(img,chr);
    Score z = getZ(img,NULL);
    score.ocr = pow(2,score.ocr)/z.ocr;
    score.trans = pow(2,score.trans)/z.trans;
    score.combined = pow(2,score.combined)/z.trans;
    return score;
  } 
  void getPrediction(const string& img_data) {
    ifstream fin(img_data, ifstream::in);
    ofstream fout(img_data+".predict", ofstream::out);
    string s;
    vector<int> img;
    int best[3];
    int sample = 1;
    while(getline(fin,s)) {
      stringstream ss(s);
      img.clear();
      int n;
      cout << sample++ << endl;
      while(ss >> n) img.push_back(n);
      getZ(img,best);
      for(int i = 0; i < 3; i++) {
        int word = best[i];
        // fout << word;
        for(int j=0;j < img.size();j++) {
          fout << chars[word%10];
          word /= 10;
        }
        fout << " ";
      }
      fout << endl;
    }
    fin.close();
    fout.close();
  }
};

int main() {
  Model m;
  m.LoadModel("OCRdataset/potentials/ocr.dat","OCRdataset/potentials/trans.dat");
  m.getPrediction("OCRdataset/data/small/images.dat");
  return 0;
}