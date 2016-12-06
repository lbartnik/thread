#' @useDynLib thread
NULL

#' @export
try_evaluate <- function (fun, data)
{
  evaluate(fun, data)
}
