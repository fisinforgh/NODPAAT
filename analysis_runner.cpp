
#include <condition_variable>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// Mutex for logging
std::mutex io_mutex;

// Thread pool implementation
class ThreadPool {
public:
  ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
      workers.emplace_back([this] {
        while (true) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(
                lock, [this] { return this->stop || !this->tasks.empty(); });
            if (this->stop && this->tasks.empty())
              return;
            task = std::move(this->tasks.front());
            this->tasks.pop();
          }
          task();
        }
      });
    }
  }

  template <class F> void enqueue(F &&f) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      if (stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");
      tasks.emplace(std::forward<F>(f));
    }
    condition.notify_one();
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
      worker.join();
  }

private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;

  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop = false;
};

// Function to run one analysis
void run_analysis(int nEveOffSet, int lat, int lon, int alpha) {
  std::ostringstream name;
  name << "LAT" << lat << "LON" << lon;

  std::ostringstream cmd;
  cmd << "./chi2LRSO3vsSnRunApp -E" << nEveOffSet << " -N" << name.str()
      << " -I" << alpha;

  {
    std::lock_guard<std::mutex> lock(io_mutex);
    std::cout << "== START " << cmd.str() << " ==" << std::endl;
  }

  int ret = system(cmd.str().c_str());

  {
    std::lock_guard<std::mutex> lock(io_mutex);
    std::cout << "============================================== END "
              << name.str() << " == (ret=" << ret << ")" << std::endl;
    for (int k = 0; k < 20; ++k) {
      std::cout << "============================================== END "
                << name.str() << " ==" << std::endl;
    }
    std::cout << std::endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << " <nEveOffSet> <grid precision i.e 10|5|2> <alpha>"
              << std::endl;
    return 1;
  }

  int nEveOffSet = std::stoi(argv[1]);
  int gridPrecision = std::stoi(argv[2]);
  int alpha = std::stoi(argv[3]);

  int latMin = -90, latMax = 90;
  int lonMin = -180, lonMax = 180;

  const size_t NUM_THREADS = std::thread::hardware_concurrency(); // auto detect
  ThreadPool pool(NUM_THREADS > 0 ? NUM_THREADS : 8);

  for (int lon = lonMin; lon <= lonMax; lon += gridPrecision) {
    for (int lat = latMin; lat <= latMax; lat += gridPrecision) {
      pool.enqueue([=] { run_analysis(nEveOffSet, lat, lon, alpha); });
    }
  }

  // ThreadPool destructor waits for all tasks
  return 0;
}
