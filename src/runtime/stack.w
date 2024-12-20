@extern func puts(str: long) : void;
@extern func printf(fmt: long, ...) : void;
@extern func abort() : void;

@pub @pure[stchk] func __WDcanary_fail(ptr: long, name: long) : void {
  puts("[STACK CHECK] *** Integrity check failed ***");
  printf("** BACKTRACE: %s (%p) **\n", name, ptr);
  abort();
}