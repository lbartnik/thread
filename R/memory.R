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
run_c_memory_exapmle <- function (silent = FALSE, n = 10)
{
  thread_runner <- function (n = 10) {
    size <- 10000
    timeout <- 1000
    native_call("C_memory_allocation_test", as.integer(n), as.integer(size),
                as.integer(timeout))
  }
  
  memory_monitor <- function (args) {
    for (i in seq(args$n+2)) {
      print(memory.profile())
      thread_sleep(1000)
    }
  }
 
  th1 <- new_thread(thread_runner, n)
  th2 <- new_thread(thread_runner, n)
  if (!isTRUE(silent)) {
    th3 <- new_thread(memory_monitor, n)
  }

  thread_join(th1)
  thread_join(th2)
  if (!isTRUE(silent)) {
    thread_join(th3)
  }
}
