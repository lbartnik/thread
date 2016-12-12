context("threads & errors")

test_that("error can be thrown in a thread", {
  thread_runner <- function (data) {
    stop("error")
  }

  thread <- new_thread(thread_runner, list())
  thread_join(thread)

  expect_equal(thread_result(thread), 0)
})


