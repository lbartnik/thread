/*
 * How to intercept calls to allocVector?
 * 
 * http://stackoverflow.com/questions/12947388/is-there-a-way-to-modify-the-code-of-a-function-in-a-linux-c-program-at-runtime
 * 
 * Don't overwrite the procedure, overwrite the symbol reference in
 * the symbol table instead. That does require dynamic linkage.
 * Alternatively you can overwrite the call(s) to the function with
 * a call to the other function, but things like NX bits may come to
 * stand in your way. Self-modifying code is generally frowned upon.
 * 
 * http://bottomupcs.sourceforge.net/csbu/x4012.htm
 * https://refspecs.linuxbase.org/elf/gabi4+/ch4.symtab.html
 * 
 * http://stackoverflow.com/questions/11254891/can-a-running-c-program-access-its-own-symbol-table
 * 
 * http://stackoverflow.com/questions/12255555/accessing-elf-symbol-table-in-c
 */


#include <cstdio>
#include <cstring>
#include <Rinternals.h>

#include <elf.h>

Elf64_Sym  gbl_sym_table[1] __attribute__((weak));
