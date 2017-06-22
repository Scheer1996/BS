#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#define MINOR_START 0
#define DEVICE_COUNT 1
#define BUFFER_SIZE 128

#ifndef REF_TIME
#define REF_TIME -1
#endif

#ifndef REF_SIZE
#define REF_SIZE -1
#endif

#define PDEBUG(fmt, args...) printk(KERN_DEBUG "Aufgabe: "fmt, ## args)
#define MINIMUM(a,b)  ((a) < (b) ? (a) : (b))

