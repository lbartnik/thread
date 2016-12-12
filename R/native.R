#' @export
native_call <- function (symbol_name, ..., package_name)
{
  args <- list(...)
  symbol <- getNativeSymbolInfo(symbol_name, package_name)
  
  .Call("C_thread_run_native", symbol$address, args)
}
