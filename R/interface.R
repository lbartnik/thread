#' @useDynLib thread
NULL

#' @importFrom Rcpp cppFunction Rcpp.plugin.maker
#' @export
try_evaluate <- function (fun, data)
{
  assign("foo", function()1L, envir = globalenv())
  init()

  store(fun, data)
  gc()
  trigger_evaluation()
  gc()
  res <- evaluation_result()

  join_bg()
  
  #res
  .Call("get_static_result")
}
