/*
 * This character device driver allows a user program to write up to a certain number
 * of bytes to a buffer and to read those bytes FIFO-style from the buffer.
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

/** Constants **/
#define DEVICE_NAME "SampleCharDevice"
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

/** Global variables **/

static int majorVersion;
static char buffer[BUFFER_SIZE + 1];
static int remainingSpace = BUFFER_SIZE;

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

   return 0;
}

void cleanup_module(void)
{
   // Deregister the device.
   unregister_chrdev(majorVersion, DEVICE_NAME);

   // Otherwise, notify upon successful deregistration.
   printk(KERN_INFO "Successfully deregistered character device with major version %d\n", majorVersion);
}

static int device_open(struct inode* inodep, struct file* filep)
{
   printk(KERN_INFO "Character device opened.\n");
   return 0;
}

static int device_release(struct inode* inodep, struct file* filep)
{
   printk(KERN_INFO "Character device closed.\n");
   return 0;
}

static ssize_t device_read(struct file* filep, char* output, size_t length, loff_t* offset)
{
   // Functions like 'cat' will continue reading until 0 is returned as the output size.
   // Therefore, return 0 if the buffer contents have already been sent to the user.
   if (*offset > 0)
   {
       return 0;
   }

   // Determine number of bytes to pop from buffer.
   int numBytesToPop = (length < BUFFER_SIZE - remainingSpace) ? (length) : (BUFFER_SIZE - remainingSpace);

   // Send the portion of the buffer contents to the user.
   int result = copy_to_user(output, buffer, numBytesToPop);

   // Overwrite the popped bytes with the remaining bytes in the buffer.
   char temp[BUFFER_SIZE - numBytesToPop];
   strcpy(temp, &buffer[numBytesToPop]);
   strcpy(buffer, temp);

   // Update the offset in order to indicate to the user program that the
   // reading of the buffer should end.
   *offset += numBytesToPop;

   // Update number of remaining bytes.
   remainingSpace += numBytesToPop;

   // Log the fact that the device was read from.
   printk(KERN_INFO "Buffer contents read from character device. Length requested: %d\n", length);

   // Return the size of the buffer contents.
   return numBytesToPop;
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

   // Log write success.
   printk(KERN_INFO "The message was successfully written to the character device.\n");
   printk(KERN_INFO "Buffer Contents: %s\n", buffer);

   // This is critical. MUST return the length of the appended message.
   return length;
}

