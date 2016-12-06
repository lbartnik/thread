// [[Rcpp::plugins(cpp11)]]

#include <Rcpp.h> 
using namespace Rcpp;

// [[Rcpp::export]]
Rcpp::RObject thread_evaluate (Rcpp::Function _function, Rcpp::RObject _data)
{
  return _function(_data);
}

