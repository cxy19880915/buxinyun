
void cpp_run_init()
{
    int i ;
    extern unsigned int _ctors_begin, _ctors_count ;
    unsigned int *dat = (u32 *)&_ctors_begin ;
    unsigned int count = (u32)&_ctors_count ;
    void (*fun)() ;
    for (i = 0; i < (count) / 4 ; i++) {
        fun = (void (*)())dat[i];
        fun() ;

    }

}