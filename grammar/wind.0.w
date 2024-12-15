@extern func memcpy(dest: long, src: long, size: int) : long;
@extern func printf(str: long, ...) : int;

//@type ptr = long;
//@type string = ptr;

func main() : int {
	var localVar: [byte; 32];
	memcpy(localVar, "Hello", 5);
	localVar[1] = 'J';
	printf("Mem: %s\n" localVar);
	return 0;
}
