#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
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
            printk("ohshit is NULL \n");
            hello_exit() ;
        }
    }
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

