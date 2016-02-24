#include <unistd.h>
#include <vector>
using std::vector;
#include <thread>
using std::thread;
#include <chrono>
#include <mutex>
using std::mutex;
using std::unique_lock;
#include <condition_variable>
using std::condition_variable;
#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
using std::ostream;
#include <queue>
using std::queue;
#include <string>
using std::string;
using std::to_string;
#include <functional>
using std::ref;
using std::bind;
using std::less;
using namespace std::placeholders;
#include <algorithm>
#include <fstream>
using std::ifstream;
#include <bitset>
using std::bitset;

mutex M;

vector<string> split(const string& s, char c) {
  vector<string> v;
  string::size_type i = 0;
  string::size_type j = s.find(c);

  if (j == string::npos)
    v.push_back(s);

  while (j != string::npos) {
    v.push_back(s.substr(i, j-i));
    i = ++j;
    j = s.find(c, j);

    if (j == string::npos)
      v.push_back(s.substr(i, s.length()));
  }

  return v;
}

int pow2(int i) {
  return 2 << (i-1);
}

template <typename T>
ostream& operator<<(ostream& out, const vector<T>& v) {
  out << '[';
	for(int i = 0; i < v.size(); ++i) {
    if (i != 0)
      out << ", ";
		out << v[i];
	}
  out << "]\n";

	return out;
}

template <typename Collection>
Collection tailIt(Collection &col) {
  col.erase(col.begin());
  return  col;
}

template <typename Collection>
Collection tail(Collection col) {
  return  tailIt(col);
}

template <typename Collection,typename unop>
void for_each(const Collection &col, unop op){
  std::for_each(col.begin(),col.end(),op);
}

template <typename Collection,typename unop>
Collection mapIt(unop op, Collection &col) {
  std::transform(col.begin(),col.end(),col.begin(),op);
  return col;
}

template <typename Collection,typename unop>
Collection map(unop op, Collection col) {
  return mapIt(op, col);
}

template <typename Collection,typename Predicate>
Collection filterIt( Predicate npred, Collection &col) {
  auto pred = [npred](typename Collection::value_type i) { return !npred(i);};
  auto returnIterator = std::remove_if(col.begin(), col.end(), pred);
  col.erase(returnIterator, std::end(col));
  return col;
}

template <typename Collection,typename Predicate>
Collection filter(Predicate npred, Collection col) {
  return filterIt(npred, col);
}

template <typename T>
class myBlockingQueue {
private:
  mutex mutex_;
  queue<T> queue_;
  condition_variable cond_;
  int readers_;
public:
  T pop() {
  unique_lock<mutex> mlock(mutex_);
  while (queue_.empty()) {
    cond_.wait(mlock);
  }
  auto item = queue_.front();
  queue_.pop();
  return item;
  /*  this->mutex_.lock();
    T value = this->queue_.front();
    this->queue_.pop();
    this->mutex_.unlock();
    return value;*/
  }

  myBlockingQueue(int r){
    if (r < 0) {
      cerr << "Need positive number";
      exit(1);
    }
    readers_ = r;
  }

  bool any() {
    return readers_ > 0;
  }

  void done() {
    this->mutex_.lock();
    this->readers_--;
    this->mutex_.unlock();
  }

  void push(const T& item) {
    unique_lock<mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }

  bool empty() {
    this->mutex_.lock();
    bool check = this->queue_.empty();
    this->mutex_.unlock();
    return check;
  }
};

void Reader(int i, int n_vars, myBlockingQueue<string> &q) {
  M.lock();
  cout << "Hello, Reader " << i << "!\n";
  M.unlock();
  string fname = "poly_" + std::to_string(i+1) + ".txt";
  ifstream poly(fname);
  string s;
  while (poly >> s) {

    M.lock();
    std::cout << "R" << i << " " << s << std::endl;
    M.unlock();

    q.push(s);
  }
  q.done();
  //cout<<"All done\n";
}

string conv(int number, int size) {
  string ret(size, '0');
  int i = 0, base = 2;
  do {
    ret[size - 1 - i++] = char('0') + number % base;
    number = int(number / base);
  } while (number);

  return ret;
}

vector<string> allVectors(int n_vars) {
  int all = 2 << (n_vars - 1);
  //cout << all << endl;
  vector<string> res(all);
  for (int i = 0; i < all; ++i) {
    res[i] = conv(i, n_vars);
  }

  return res;
}


string code(string monom, int n_vars) {
  string res(n_vars, '0');
  //cout<<monom<<endl<<res<<endl;
  vector<string> vs = tail(split(monom, 'x'));

  for (int i = 0; i < vs.size(); ++i) {
    res[atoi(vs[i].c_str()) - 1] = '1';
  }

  return res;
}

bool le(string s1, string s2) {
  for (int i = 0; i < s2.length(); ++i) {
    if (s2[i] < s1[i])
      return false;
  }

  return true;
}

void makeVec(const string &poly, string &res, int n_vars, int beg = 0, int end = -1) {
  auto vectors = allVectors(n_vars);
  //string res(pow2(n_vars),'0');
  if (end < 0)
    end = vectors.size();
  vector<string> v = map(bind(code,_1,n_vars), split(poly, '+'));
  for (int i = beg; i < end; ++i) {
    res[i] = '0' + filter(bind(le,_1,vectors[i]), v).size() % 2;
  }
}

class PolyRange {
private:
  string poly;
  string &res;
  int n_vars, beg, end;

public:

  PolyRange(const string &p, string &r, int n, int b = 0, int e = -1) : \
    poly(p), res(r), n_vars(n), beg(b), end(e) {};

  void operator() () {
    makeVec(poly, res, n_vars, beg, end);
  }

  friend ostream& ::operator<<(ostream& out, const PolyRange& pr) {
    out << pr.res;

  	return out;
  }

};

void Solver(int i, int n_vars, myBlockingQueue<string> &q, const vector<string> &bins) {
  //sleep(1);
  M.lock();
  cout << "Hello, Solver " << i << "!\n";
  M.unlock();
  string s;

  while (!q.empty()) {
    s = q.pop();
    string res(pow2(n_vars),'0');
    makeVec(s,res,n_vars);

    M.lock();
    cout << "S" << i << " " << s << endl << res << endl;
    M.unlock();

    usleep(1);
  }
}

bool odd(int i) {
  return i % 2;
}

int main () {
  int ns = 3, nr = 4, n_vars = 3, i = 0;
  thread readers[nr];
  thread solvers[ns];
  myBlockingQueue<string> q(nr);
  vector<string> bins = allVectors(n_vars);

  for( i=0; i < nr; i++ ) {
    M.lock();
    cout << "main() : creating reader, " << i << endl;
    M.unlock();
    readers[i] = thread(Reader, i, n_vars, ref(q));
  }

  for( i=0; i < nr; i++ ) {
    readers[i].join();
  }

  usleep(100);

  for( i=0; i < ns; i++ ) {
    M.lock();
    cout << "main() : creating solver, " << i << endl;
    M.unlock();
    solvers[i] = thread(Solver, i, n_vars, ref(q), ref(bins));
  }

  for( i=0; i < ns; i++ ) {
    solvers[i].join();
  }

  cout<<"\nAll done!\n";
}
