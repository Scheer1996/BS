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

#define MINOR_START 0
#define DEVICE_COUNT 2
#define BUFFER_SIZE 128
#define MINIMUM(a,b)  ((a) < (b) ? (a) : (b))


static int aufgabe_major;
static int letzte_zeile_zeichen_anzahl;
static int momentane_zeile_zeichen_anzahl;
static u64 zeit_stempel;
static int zeit_seit_letzte_gelessene_zeile;

int open(struct inode *inode, struct file *filp) {
	
	PDEBUG("OPEN\n");

	return 0;
}

int release(struct inode *inode, struct file *filp) {
	// Nothing to do
	PDEBUG("RELEASE\n");
	return 0;
}


ssize_t read(struct file *filp
, char *buffer,//Buffer denn man fuelt
size_t buffer_length,//Die lange des buffers
loff_t *f_offset//Unser offset im file
){
	int ubergeben = 0;
	
	if(*f_offset != 0){ //zweiter read befehl
		return 0;
	}
	PDEBUG("Data mit der lange %d wird gelessen\n", buffer_length);
	
	char ausgabe_buffer[BUFFER_SIZE]
	
	sprintf(ausgabe_buffer,"%i  %u\n",letzte_zeile_zeichen_anzahl,zeit_seit_letzte_gelessene_zeile);
	
	ubergeben = MINIMUM(strlen(ausgabe_buffer),buffer_length);
	
	return ubergeben;
	
}

ssize_t write(struct file *filp, char *buffer,size_t buffer_length,loff_t *f_offset){

		letzte_zeile_zeichen_anzahl = momentane_zeile_zeichen_anzahl;
		momentane_zeile_zeichen_anzahl = buffer_length;
		
		
		u64 temp = get_jiffies_64();
		zeit_seit_letzte_gelessene_zeile = time_after(zeit_stempel,temp);
		zeit_seit_letzte_gelessene_zeile = zeit_seit_letzte_gelessene_zeile * 1000 / Hz;
		zeit_stempel = temp;
	
}

struct file_operations fops = {
       .read = read,
       .write = write,
       .open = open,
       .release = release
    };

static int aufgabe_init(void){
	
	int result;
	dev_t dev = 0;
	
	result = alloc_chrdev_region(&dev,MINOR_START,DEVICE_COUNT,"aufgabe");
	if (result < 0) {
		printk(KERN_WARNING "Cannot register Major number for timer!");
		return result;
	}
	
	aufgabe_major = MAJOR(dev);
	letzte_zeile_zeichen_anzahl = 0;
	zeit_seit_letzte_gelessene_zeile = 0;
	
	return 0;
}

static void aufgabe_exit(void){
	dev_t devno = MKDEV(aufgabe_major, MINOR_START);
	
	unregister_chrdev_region(devno,DEVICE_COUNT);
}


module_init(aufgabe_init);
module_exit(aufgabe_exit);