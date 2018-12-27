
typedef unsigned long size_t;

extern "C" {
	int puts(const char *);
	void *malloc(size_t);
	void free(void *);
}

extern "C" {

__attribute__((noreturn))
void __cxa_pure_virtual(void) {
	puts("Pure virutal function called!");
	while (1);
}

__attribute__((noreturn))
void __cxa_deleted_virtual(void) {
	puts("Deleted virtual function called!");
	while (1);
}

}

__attribute__((__weak__, __visibility__("default")))
void *
operator new(size_t size)
{
	if (size == 0)
		size = 1;
	void *p = malloc(size);
	if (p == (void *)0) {
		puts("malloc failed");
		while (1);
	}
	return p;
}

__attribute__((__weak__, __visibility__("default")))
void *
operator new[](size_t size)
{
	return ::operator new(size);
}

__attribute__((__weak__, __visibility__("default")))
void
operator delete(void *ptr)
{
	if (ptr)
		free(ptr);
}

__attribute__((__weak__, __visibility__("default")))
void
operator delete[](void *ptr)
{
	::operator delete(ptr);
}

