#ifndef __MEM_LEAK_TEST_H__
#define __MEM_LEAK_TEST_H__



#define malloc(sz)		        _vmalloc_dbg((sz), __FUNCTION__, __LINE__)
#define calloc(n, sz)		    _vcalloc_dbg((n), (sz), __FUNCTION__, __LINE__)
#define free(pbuf)		        _vfree_dbg(pbuf)
#define kfree(pbuf)		        _vfree_dbg(pbuf)
#define vfree(pbuf)		        _vfree_dbg(pbuf)
#define realloc(pv, sz)         _vrealloc_dbg(pv, (sz), __FUNCTION__, __LINE__)
#define kmalloc(sz, flags)      _vmalloc_dbg((sz), __FUNCTION__, __LINE__)
#define vmalloc(sz)		        _vmalloc_dbg((sz), __FUNCTION__, __LINE__)
#define kzalloc(sz, flags)	    _vcalloc_dbg(1, (sz), __FUNCTION__, __LINE__)
#define zalloc(sz)	            _vcalloc_dbg(1, (sz), __FUNCTION__, __LINE__)

extern void malloc_debug_start(void);
extern void malloc_debug_stop(void);
extern void malloc_debug_show(void);

#endif

