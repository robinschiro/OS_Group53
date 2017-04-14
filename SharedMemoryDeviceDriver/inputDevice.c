/*
 * This character device driver allows a user program to write up to a certain number
 * of bytes to a buffer.
 * The implementations of most of the functions are based on code examples from the
 * following two sources:
 * http://www.tldp.org/LDP/lkmpg/2.6/html/x569.html
 * http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
 **/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/mutex.h>

/** Constants **/
#define DEVICE_NAME "SampleInputDevice"
#define BUFFER_SIZE 1024

/** Function Prototypes **/

// Core kernel functions
int init_module(void);
void cleanup_module(void);

// Functions required for character device
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

// Specify callback functions for the file operations structure.
static struct file_operations fops =
{
   .open = device_open,
   .release = device_release,
   .read = device_read,
   .write = device_write
};

/** Private Global variables **/

static int majorVersion;

/** Public Global variables **/
char buffer[BUFFER_SIZE + 1];
DEFINE_MUTEX(charDeviceMutex);
int remainingSpace = BUFFER_SIZE;
EXPORT_SYMBOL(buffer);
EXPORT_SYMBOL(charDeviceMutex);
EXPORT_SYMBOL(remainingSpace);

/** Function Definitions **/

int init_module(void)
{
   // Attempt to retrieve a valid major number for the device
   majorVersion = register_chrdev(0, DEVICE_NAME, &fops);

   // Handle error.
   if (majorVersion < 0)
   {
      printk(KERN_ALERT "Failed to register character device with version %d\n", majorVersion);
      return majorVersion;
   }

   // Otherwise, notify upon successful registration.
   printk(KERN_INFO "Successfully registered character device with major version %d\n", majorVersion);

   // Initialize buffer.
   strcpy(buffer, "");
   
   // Initialize the mutex.
   mutex_init(&charDeviceMutex);

   return 0;
}

void cleanup_module(void)
{
   // Destroy the mutex.
   mutex_destroy(&charDeviceMutex);
   
   // Deregister the device.
   unregister_chrdev(majorVersion, DEVICE_NAME);

   // Otherwise, notify upon successful deregistration.
   printk(KERN_INFO "Successfully deregistered character device with major version %d\n", majorVersion);
}

static int device_open(struct inode* inodep, struct file* filep)
{
   // Lock the device to prevent it from being read from while it's being written to.
   if (!mutex_trylock(&charDeviceMutex))
   {
      printk(KERN_INFO "This device is currently locked by another process.");
      return -EBUSY;
   }
   
   // Increment the counter that is keeping track of the number of processes
   // that have opened the device. This is needed to prevent someone from 
   // uninstalling the module if it is still in use.
   try_module_get(THIS_MODULE);

   printk(KERN_INFO "Input device opened.\n");
   return 0;
}

static int device_release(struct inode* inodep, struct file* filep)
{
   // Unlock the mutex.
   mutex_unlock(&charDeviceMutex);
   
   // Decrement the process usage counter.
   module_put(THIS_MODULE);

   printk(KERN_INFO "Input device closed.\n");
   return 0;
}

static ssize_t device_read(struct file* filep, char* output, size_t length, loff_t* offset)
{   
   printk(KERN_INFO "Error: Cannot read from an input device\n");

   return -1;
}

static ssize_t device_write(struct file* filep, const char* message, size_t length, loff_t* offset)
{
   // Copy incoming message to a temporary buffer.
   char messageBuffer[remainingSpace + 1];
   int numBytesToPush = (length < remainingSpace) ? (length) : (remainingSpace);
   strncpy(messageBuffer, message, numBytesToPush);

   // Append null-terminating character to properly format the message string.
   messageBuffer[length] = '\0';

   // Log message content and length.
   printk(KERN_INFO "Incoming Message Length: %d. Attempting to write message \"%s\" to character device.\n", length,
          messageBuffer);

   // Append the message to the internal buffer.
   strcat(buffer, messageBuffer);

   // Update number of remaining bytes.
   remainingSpace -= numBytesToPush;

   // Log buffer contents.
   printk(KERN_INFO "Buffer contents after write: %s\n", buffer);

   // This is critical. MUST return the length of the appended message.
   return length;
}

