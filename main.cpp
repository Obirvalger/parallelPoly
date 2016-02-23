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

void Solver(int i, int n_vars, myBlockingQueue<string> &q, const vector<string> &bins) {
  //sleep(1);
  M.lock();
  cout << "Hello, Solver " << i << "!\n";
  M.unlock();
  string s;
  vector<string> v;
  auto codeN = [n_vars] (string s) {return code(s, n_vars);};

  while (!q.empty()) {
    s = q.pop();
    v = split(s, '+');

    M.lock();
    cout << "S" << i << " " << s << endl << v << map(codeN, v);
    M.unlock();

    usleep(1);
  }
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

bool odd(int i) {
  return i % 2;
}

int main () {
  /*auto lambda_echo = [](int i ) { std::cout << i << std::endl; };
  auto m2 = [](int i) { return i*2;};
  std::vector<int> col{20,24,37,42,23,45,37};
  for_each(tailIt(col),lambda_echo);
  cout<<col;
  vector<string> v = {"1","2","3","4"};
  cout << code("x1x2x4", 5) << endl;
  auto f = bind(less<int>(), _1, 30);
  cout << filter(f, col);
  vector<string> vs = {"000","101","100","110","111"};
  cout << filter(bind(le,"000",_1),vs);*/
  //cout << allVectors(4);
  //cout<<atoi("32");

  int ns = 1, nr = 1, n_vars = 3, i = 0;
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
