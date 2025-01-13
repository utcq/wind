@include "types.w"

@extern func memcpy(dest: ptr<char>, src: ptr<char>, size: int) : ptr<char>;
@extern func printf(str: string, ...) : int;
@extern func puts(str: string) : int;
@extern func scanf(str: string, ...) : int;
@extern func memset(dest: ptr<char>, c: int, size: int) : char;
@extern func malloc(size: int) : ptr<char>;
@extern func free(ptr: ptr<char>) : void;
@extern func realloc(ptr: ptr<char>, size: int) : ptr<char>;
@extern func strlen(str: string) : int;
@extern func strncat(dest: string, src: string, n: int) : string;


// Wind runtime functions useful for the user
@extern func __builtin_copy(dst: ptr<char>, src: ptr<char>, size: uint64): void; 
@extern func __builtin_memset(dst: ptr<char>, c: char, size: uint64): void;