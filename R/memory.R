#' @export
is_memory_synchronized <- function ()
{
  .Call("C_is_memory_synchronized")
}

#' @export
memory_allocation_test  <- function (n, size, timeout)
{
  .Call("C_memory_allocation_test", as.integer(n), as.integer(size),
        as.integer(timeout))
}

#' @export
run_c_memory_exapmle <- function ()
{
  thread_runner <- function (data) {
    n <- 100
    size <- 10000
    timeout <- 100
    native_call("C_memory_allocation_test", as.integer(n), as.integer(size),
                as.integer(timeout))
  }
  
  th1 <- new_thread(thread_runner, list())
  th2 <- new_thread(thread_runner, list())
  
  thread_join(th1)
  thread_join(th2)
}
