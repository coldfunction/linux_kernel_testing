#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <asm/fpu/internal.h>
#include <linux/smp.h>
#include <linux/kthread.h>
#include <linux/sched.h>

MODULE_LICENSE("Dual BSD/GPL");


#define THREAD_NUM 2

typedef struct __attribute__((__packed__)) c16x8_header {
    __u64 gfn;
    __u32 size;
    __u8 h[16];
} c16x8_header_t;

static struct task_struct *cmp_tsk[4];


static inline s64 time_in_us(void) {
      ktime_t val;
      val = ktime_get();
      return ktime_to_ns(val) / 1000;
}



static inline int memcmp_avx_32(uint8_t *a, uint8_t *b)
{
      unsigned long eflags;
      asm volatile("vmovdqa %0,%%ymm0" : : "m" (a[0]));
      asm volatile("vmovdqa %0,%%ymm1" : : "m" (b[0]));

      asm volatile("vxorpd %ymm0,%ymm1,%ymm2");
      asm volatile("vxorpd %ymm3,%ymm3,%ymm3");
      asm volatile("vptest %ymm2, %ymm3");
      asm volatile("pushf \n\t pop %0" : "=&r"(eflags));

	return !(eflags & X86_EFLAGS_CF);
}




struct page **mypage1;
struct page **mypage2;
static void hello_exit(void);
static int myid[4];

static int diff(struct page *page1, struct page *page2)
{
	char *backup = kmap_atomic(page1);
	char *page = kmap_atomic(page2);

	int i;
	uint8_t *buf;
	uint8_t *block;
	c16x8_header_t *header;

	buf = kmalloc(64*1024 + 8192, GFP_KERNEL);
	header = (c16x8_header_t *)buf;
	block = buf + sizeof(*header);

//	printk("ready to compress\n");
    kernel_fpu_begin();
    for (i = 0; i < 4096; i += 32) {
    	if (memcmp_avx_32(backup + i, page + i)) {
        	header->h[i / 256] |= (1 << ((i % 256) / 32));
            memcpy(block, page + i, 32);
            block += 32;
        }
    }
    kernel_fpu_end();


	if (block == buf + sizeof(*header)) {
    	memset(header->h, 0xff, 16 * sizeof(__u8));
        memcpy(block, page, 4096);
        block += 4096;
    }


    kunmap_atomic(backup);
    kunmap_atomic(page);

	if (block == buf + sizeof(*header))
		return 0;

	header->size = sizeof(header->h) + (block - (buf + sizeof(*header)));
	//int ret = block-buf;
	int ret = block-buf-sizeof(*header);
	kfree(buf);

	return ret;

}

static int compress_dirty_bytes(void)
{
	int len = 0;
	int i,j;

	s64 start = time_in_us();
    for(i = 0; i < 128; i++) {
        for(j = 0; j < 1024; j++) {
			len += diff(mypage1[i]+j, mypage2[i]+j);
		}
	}
	s64 end = time_in_us();
	printk("totoal diff len = %d, time = %ld, difftime = %d, rate = %d\n", len, time_in_us(), end-start, len/(end-start));
}

static int calc_dirty_bytes(void *arg)
{
	int *id = (int*) arg;
	printk("cocotion test id = %d, time = %ld\n", *id, time_in_us());
	compress_dirty_bytes();
	return 0;
}



static int hello_init(void)
{
    printk(KERN_INFO "Hello kernel\n");
    mypage1 = kmalloc(sizeof(struct page*)*128, GFP_KERNEL | __GFP_ZERO);
    mypage2 = kmalloc(sizeof(struct page*)*128, GFP_KERNEL | __GFP_ZERO);

    int i,j,k;
    for(i = 0; i < 128; i++) {
        mypage1[i] = alloc_pages(GFP_KERNEL | __GFP_ZERO, 10);
        mypage2[i] = alloc_pages(GFP_KERNEL | __GFP_ZERO, 10);
        if(mypage1[i] == NULL) {
            hello_exit() ;
        }
        if(mypage2[i] == NULL) {
            hello_exit() ;
        }
    }
    for(i = 0; i < 128; i++) {
        for(j = 0; j < 1024; j++) {
            char *page = kmap_atomic(mypage1[i]+j);
            get_random_bytes(page, 4096);
            kunmap_atomic(page);
        }
    }
    for(i = 0; i < 128; i++) {
        for(j = 0; j < 1024; j++) {
            char *page = kmap_atomic(mypage2[i]+j);
            get_random_bytes(page, 4096);
            kunmap_atomic(page);
        }
    }

    printk("random generating okokok\n");


	int ret;
	for(i = 0; i < THREAD_NUM; i++) {

		myid[i] = i;
		cmp_tsk[i] = kthread_create(calc_dirty_bytes, &myid[i], "cmp thread");
		if(IS_ERR(cmp_tsk[i])) {
			ret = PTR_ERR(cmp_tsk[i]);
			cmp_tsk[i] = NULL;
			goto out;
		}
		kthread_bind(cmp_tsk[i], 7-i);
	}


	for(i = 0; i < THREAD_NUM; i++) {
		printk("wakeup %d time = %ld\n", i, time_in_us());
		wake_up_process(cmp_tsk[i]);
	}
	printk("wait completing time = %ld\n", time_in_us());
	//	smp_call_function_single(7, calc_dirty_bytes, 0, true);
//	smp_call_function_single(7, calc_dirty_bytes, 1, true);


    return 0;

out:
	return ret;

}

static void hello_exit(void)
{
        printk(KERN_INFO "Goodbye\n");
        int i;
        for(i = 0; i < 128; i++) {
            if(mypage1[i]!=NULL)
                __free_pages(mypage1[i], 10);
                __free_pages(mypage2[i], 10);
        }
        kfree(mypage1);
}

module_init(hello_init);
module_exit(hello_exit);

