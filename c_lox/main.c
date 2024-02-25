#include <stdio.h>
#include <signal.h>
#include <lox/all.h>

void __attribute__((noreturn))
vm_delete_on_sigint(int _) {
  if (vm.objects) putchar(10);
  vm_delete();
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  // Temporary cleanup procedure for testing
  signal(SIGINT, vm_delete_on_sigint);
  vm_init();
  if (argc == 1) repl();
  else if (argc == 2)
    run_file(argv[1]);
  else {
    fputs("Usage: clox [path]\n", stderr);
    exit(64);
  }
  vm_delete();
}
