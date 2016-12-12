context("synchronized memory")


test_that("memory access is synchronized", {
  skip_if_not(is_memory_synchronized())
  
  run_c_memory_exapmle()
})
