@include [
    "#types.wi"
]

@extern func puts(str: string) : void;
@extern func printf(fmt: string, ...) : void;
@extern func abort() : void;

@pub @pure[stchk] func __WD_canary_fail() : void {
  puts("*** [ canary fail ] ***");
  asm {
    mov rsi, r15;
  }
  printf("Canary fail encountered in function '%s'\n");
  abort();
}

@pub func __builtin_copy(dst: ptr<char>, src: ptr<char>, size: u64): void {
  var i: u64 = 0;
  loop [i < size] {
    dst[i] = src[i];
    i++;
  }
}

@pub func __builtin_memset(dst: ptr<char>, c: char, size: u64): void {
  var i: u64 = 0;
  loop [i < size] {
    dst[i] = c;
    i++;
  }
}