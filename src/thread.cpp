// [[Rcpp::plugins(cpp11)]]

#include <Rcpp.h> 
using namespace Rcpp;

// [[Rcpp::export]]
Rcpp::RObject evaluate (Rcpp::Function fun, Rcpp::RObject data)
{
  return fun(data);
}

