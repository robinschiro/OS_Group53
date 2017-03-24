#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#define  DEVICE_NAME "Sample"
#define  CLASS_NAME  "CharDevice"

/** Function Prototypes **/
int init_module(void);
void cleanup_module(void);

int init_module(void)
{
   printk(KERN_INFO "Installing");
   return 0;
}

void cleanup_module(void)
{
   printk(KERN_INFO "Removing");
}
