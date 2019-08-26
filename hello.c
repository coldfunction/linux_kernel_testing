#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <asm/fpu/internal.h>
MODULE_LICENSE("Dual BSD/GPL");


typedef struct __attribute__((__packed__)) c16x8_header {
    __u64 gfn;
    __u32 size;
    __u8 h[16];
} c16x8_header_t;


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

	printk("ready to compress\n");
    kernel_fpu_begin();
    for (i = 0; i < 4096; i += 32) {
    	if (memcmp_avx_32(backup + i, page + i)) {
        	header->h[i / 256] |= (1 << ((i % 256) / 32));
            memcpy(block, page + i, 32);
            block += 32;
			printk("compress okok\n");
        }
    }
    kernel_fpu_end();

	printk("after compress1\n");

	if (block == buf + sizeof(*header)) {
    	memset(header->h, 0xff, 16 * sizeof(__u8));
        memcpy(block, page, 4096);
        block += 4096;
    }

	printk("after compress2\n");

    kunmap_atomic(backup);
    kunmap_atomic(page);

	if (block == buf + sizeof(*header))
		return 0;

	header->size = sizeof(header->h) + (block - (buf + sizeof(*header)));
	int ret = block-buf;
	kfree(buf);

	return ret;

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

	int len = 0;
    for(i = 0; i < 128; i++) {
        for(j = 0; j < 1024; j++) {
			len += diff(mypage1[i]+j, mypage2[i]+j);
		}
	}

	printk("totoal diff len = %d\n", len);





	/*

    for(i = 0; i < 128; i++) {
        for(j = 0; j < 1024; j++) {
            char *backup = kmap_atomic(mypage[i]+j);
            if(j == 0 && i == 0) {
                for(k = 0; k < 1024; k++) {
                    int c = *(int*)(backup+4*k);
                    printk("%d %d, ", k, c);
                }
                printk("\n");
            }
            kunmap_atomic(backup);
        }
    }
*/



//    printk("page size = %d\n", sizeof(struct page));
//    page = kmap_atomic(mypage[0]);

 //   printk("after %d, %d, %d, %d\n", *(int*)(page), *(int*)(page+4), *(int*)(page+8), *(int*)(page+12));
  //  kunmap_atomic(page);

   // kunmap_atomic(page);
//    for(i = 0; i < 128; i++) {
 //       get_random_bytes(mypage[i], 4096*1024);
  //  }
   // printk("%d, %d, %d, %d\n", *(int*)(mypage[0]), *(int*)(mypage[0]+4), *(int*)(mypage[0]+8), *(int*)(mypage[0]+12));

    return 0;

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

