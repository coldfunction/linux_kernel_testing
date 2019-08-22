#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/mm.h>
#include <linux/highmem.h>
MODULE_LICENSE("Dual BSD/GPL");


struct page **mypage;
static void hello_exit(void);

static int hello_init(void)
{
    printk(KERN_INFO "Hello kernel\n");
    mypage = kmalloc(sizeof(struct page*)*128, GFP_KERNEL | __GFP_ZERO);

    int i;
    for(i = 0; i < 128; i++) {
        mypage[i] = alloc_pages(GFP_KERNEL | __GFP_ZERO, 10);
        if(mypage[i] == NULL) {
            hello_exit() ;
        }
    }

    char *page = kmap_atomic(mypage[0]);

//    printk("%d, %d, %d, %d\n", *(int*)(mypage[0]), *(int*)(mypage[0]+4), *(int*)(mypage[0]+8), *(int*)(mypage[0]+12));
    printk("%d, %d, %d, %d\n", *(int*)(page), *(int*)(page+4), *(int*)(page+8), *(int*)(page+12));

    kunmap_atomic(page);

    int j, k;



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
















    for(i = 0; i < 128; i++) {
        //printk("before kmap\n");
        for(j = 0; j < 1024; j++) {
            char *backup = kmap_atomic(mypage[i]+j);
            //printk("%p, %p, %p\n", mypage[i], mypage[i]+1, mypage[i]+2);
            //printk("random before %d\n", i);
            get_random_bytes(backup, 4096);
            //printk("random after %d\n", i);
            kunmap_atomic(backup);

        }
    }

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
            if(mypage[i]!=NULL)
                __free_pages(mypage[i], 10);
        }
        kfree(mypage);
}

module_init(hello_init);
module_exit(hello_exit);

