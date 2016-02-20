#include <unistd.h>
#include <vector>
using std::vector;
#include <thread>
using std::thread;
#include <mutex>
using std::mutex;
#include <iostream>
using std::cout;
using std::endl;
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
class BlockingQueue {
private:
  mutex mutex_;
  queue<T> queue_;
public:
  T pop() {
    this->mutex_.lock();
    T value = this->queue_.front();
    this->queue_.pop();
    this->mutex_.unlock();
    return value;
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

void Reader(int i)
{
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
  }
}

void Solver(int i)
{
  M.lock();
  cout << "Hello Solver! Thread ID, " << i << endl;
  M.unlock();
}

int main ()
{
  int rc, ns = 4, nr = 3;
  vector<thread> readers;
  vector<thread> solvers;
  int i;
  for( i=0; i < nr; i++ ) {
    M.lock();
    cout << "main() : creating reader, " << i << endl;
    M.unlock();
    readers.push_back(thread(Reader, i));
  }

  for( i=0; i < ns; i++ ) {
    M.lock();
    cout << "main() : creating solver, " << i << endl;
    M.unlock();
    solvers.push_back(thread(Solver, i));
  }
  sleep(1);
}
