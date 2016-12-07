// [[Rcpp::plugins(cpp11)]]

#include <Rcpp.h> 

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>


static Rcpp::Function static_fun("summary", "base");
static Rcpp::RObject  static_data;
static Rcpp::RObject  static_result;

static std::mutex              static_mutex;
static std::condition_variable static_cv;
static std::thread             static_thread;


void run_bg_thread ();


// [[Rcpp::export]]
void init ()
{
  //static_mutex.lock();
  static_thread = std::thread(run_bg_thread);
}


// [[Rcpp::export]]
void join_bg ()
{
  static_thread.join();
}


// [[Rcpp::export]]
void store (Rcpp::Function fun, Rcpp::RObject data)
{
  static_fun  = fun;
  static_data = data;
}


// [[Rcpp::export]]
void trigger_evaluation ()
{
  std::cout << "main: going to trigger" << std::endl;
  
  std::unique_lock<std::mutex> lk(static_mutex);
  static_cv.notify_all();
  
  std::cout << "main: going to yield" << std::endl;
  static_cv.wait(lk);
  
  std::cout << "main: locked again" << std::endl;
  lk.unlock();
}


// [[Rcpp::export]]
Rcpp::RObject evaluation_result ()
{
  return static_result;
}


// ---------------------------------------------------------------------

extern "C" int callFoo();

void run_bg_thread ()
{
  std::cout << "in bg thread: started" << std::endl;
  
  std::unique_lock<std::mutex> lk(static_mutex);
  
  std::cout << "in bg thread: going to wait" << std::endl;
  static_cv.wait(lk);
  
  std::cout << "in bg thread: evaluating" << std::endl;
  //static_result = static_fun(static_data);
  callFoo();
  
  std::cout << "in bg thread: exiting" << std::endl;
  lk.unlock();
  
  static_cv.notify_all();
}
