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
    //  if (n == 4 && char_values[0]==2 && char_values[1]==9 && char_values[2] ==2 && char_values[3]==9) {
    //   cout<<"called for " <<endl;
    //   for(int i = 0; i < char_values.size(); i++) {
    //     cout << char_values[i];
    //   }
    //   cout<<endl;
    // }
    assert(n == char_values.size());
    Score score;
    for(int i = 0; i < n; i++) {
      score.ocr += log2(ocr[img_values[i]][char_values[i]]);
      if(i > 0) {
        score.trans += log2(trans[char_values[i-1]][char_values[i]]);
      }
      for(int j = i+1; j < n; j++) {
        if(img_values[i] != img_values[j]) continue;
        if(char_values[j] == char_values[i]) score.combined += log2(skip_factor1);
        else score.combined += log2(skip_factor2);
      }
    }
    score.trans += score.ocr;
    score.combined += score.trans;
    // if (n == 4 && char_values[0]==2 && char_values[1]==9 && char_values[2] ==2 && char_values[3]==9) {
    //     cout << "--- " << score.ocr << endl; 
    // }
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
      total.ocr += pow(2,score.ocr);
      total.trans += pow(2,score.trans);
      total.combined += pow(2,score.combined);
      // total.ocr += score.ocr;
      // total.trans += score.trans;
      // total.combined += score.combined;
    }
    if(best){
      best[0] = m_ocr;
      best[1] = m_trans;
      best[2] = m_combined;
    }
    // cout << "Max prob: " << max_ocr <<" " << max_trans << " " << max_combined << endl;
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
    // score.ocr = score.ocr/z.ocr;  
    // score.trans = score.trans/z.trans;
    // score.combined = score.combined/z.trans;
    score.ocr = pow(2,score.ocr)/z.ocr;
    score.trans = pow(2,score.trans)/z.trans;
    score.combined = pow(2,score.combined)/z.trans;
    return score;
  } 
  void getPrediction(const string& img_data, const string& word_data,
                     const string& out_file) {
    cout << "Prediction for file: " << img_data << endl;
    ifstream fin(img_data, ifstream::in);
    ifstream fin1(word_data, ifstream::in);
    ofstream fout(out_file, ofstream::out);
    string s;
    vector<int> img;
    vector<int> chr;
    int best[3];
    int n_sample = 0;
    Score LL;
    while(getline(fin,s)) {
      cout << n_sample << "\n";
      stringstream ss(s);
      fin1 >> s;
      img.clear();
      int n;
      while(ss >> n) img.push_back(n);
      Score z = getZ(img,best);
      chr.resize(img.size());
      for(int i = 0; i < img.size(); i++) {
        chr[i] = id[s[i]-'a'];
      }
      Score prob = getUnNormalized(img,chr);
      LL.ocr += prob.ocr - log2(z.ocr);
      LL.trans += prob.trans - log2(z.trans);
      LL.combined += prob.combined - log2(z.combined);
      for(int i = 0; i < 3; i++) {
        int word = best[i];
        for(int j=0;j < img.size();j++) {
          fout << chars[word%10];
          word /= 10;
        }
        if(i!=2) fout << " ";
      }
      fout << endl;
      n_sample++;
    }
    LL.ocr /= n_sample;
    LL.trans /= n_sample;
    LL.combined /= n_sample;
    fout << "\n==== Avg. Log Likelihood =====" << endl;
    fout << LL.ocr << " " << LL.trans << " " << LL.combined;
    fin.close();
    fin1.close();
    fout.close();
  }
};

int main() {
  Model m;
  m.LoadModel("OCRdataset/potentials/ocr.dat","OCRdataset/potentials/trans.dat");
  m.getPrediction("OCRdataset/data/small/images.dat","OCRdataset/data/small/words.dat",
                  "predictions/small.prediction");
  m.getPrediction("OCRdataset/data/large/allimages1.dat","OCRdataset/data/large/allwords.dat",
                  "predictions/large1.prediction");
  m.getPrediction("OCRdataset/data/large/allimages2.dat","OCRdataset/data/large/allwords.dat",
                  "predictions/large2.prediction");
  m.getPrediction("OCRdataset/data/large/allimages3.dat","OCRdataset/data/large/allwords.dat",
                  "predictions/large3.prediction");
  m.getPrediction("OCRdataset/data/large/allimages4.dat","OCRdataset/data/large/allwords.dat",
                  "predictions/large4.prediction");
  m.getPrediction("OCRdataset/data/large/allimages5.dat","OCRdataset/data/large/allwords.dat",
                  "predictions/large5.prediction");
  return 0;
}