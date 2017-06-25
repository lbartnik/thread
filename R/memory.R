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
run_c_memory_exapmle <- function (silent = FALSE)
{
  thread_runner <- function (silent) {
    n <- 10
    size <- 10000
    timeout <- 1000
    native_call("C_memory_allocation_test", as.integer(n), as.integer(size),
                as.integer(timeout))
  }
  
  memory_monitor <- function (silent) {
    for (i in seq(12)) {
      print(memory.profile())
      thread_sleep(1000)
    }
  }
  
  th1 <- new_thread(thread_runner, silent)
  th2 <- new_thread(thread_runner, silent)
  th3 <- new_thread(memory_monitor, silent)
  
  thread_join(th1)
  thread_join(th2)
  thread_join(th3)
}
