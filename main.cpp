#include <unistd.h>
#include <vector>
using std::vector;
#include <thread>
using std::thread;
#include <mutex>
using std::mutex;
using std::unique_lock;
#include <condition_variable>
using std::condition_variable;
#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
#include <queue>
using std::queue;
#include <string>
using std::string;
using std::to_string;
#include <functional>
using std::ref;
#include <fstream>
using std::ifstream;

mutex M;

template <typename T>
class myBlockingQueue {
private:
  mutex mutex_;
  queue<T> queue_;
  condition_variable cond_;
  int readers_;
public:
  T pop() {
    /*unique_lock<mutex> mlock(mutex_);
    while (queue_.empty()) {
        cond_.wait(mlock);
    }*/
    this->mutex_.lock();
    T value = this->queue_.front();
    this->queue_.pop();
    this->mutex_.unlock();
    return value;
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

  void push(T value) {
    this->mutex_.lock();
    this->queue_.push(value);
    this->mutex_.unlock();
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
  cout << "Hello Reader! Thread ID, " << i << endl;
  M.unlock();
  string fname = "poly_3_3_" + std::to_string(i+1) + ".txt";
  ifstream poly(fname);
  string s;
  while (poly >> s) {
    M.lock();
    std::cout << "R" << i+1 << " " << s << std::endl;
    M.unlock();
    q.push(s);
  }
  q.done();
}

void Solver(int i, int n_vars, myBlockingQueue<string> &q) {
  sleep(1);
  M.lock();
  cout << "Hello Solver! Thread ID, " << i << endl;
  M.unlock();
  string s;
  while (!q.any()) {
    s = q.pop();
    M.lock();
    std::cout << "S" << i+1 << " " << s << std::endl;
    M.unlock();
  }
}

int main ()
{
  int ns = 4, nr = 3, n_vars = 3, i = 0;
  vector<thread> readers;
  vector<thread> solvers;
  myBlockingQueue<string> q(nr);

  for( i=0; i < nr; i++ ) {
    M.lock();
    cout << "main() : creating reader, " << i << endl;
    M.unlock();
    readers.push_back(thread(Reader, i, n_vars, ref(q)));
  }

  for( i=0; i < ns; i++ ) {
    M.lock();
    cout << "main() : creating solver, " << i << endl;
    M.unlock();
    solvers.push_back(thread(Solver, i, n_vars, ref(q)));
  }
  sleep(3);
  while (!q.empty()) {
    string s = q.pop();
    cout<<s<<endl;
  }
}
