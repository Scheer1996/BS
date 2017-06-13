#include "aufgabe.h"


static int aufgabe_major;
static int letzte_zeile_zeichen_anzahl;
static int momentane_zeile_zeichen_anzahl;
static u64 zeit_stempel;
static int zeit_seit_letzte_gelessene_zeile;
static struct cdev my_cdev;

int aufgabe_open(struct inode *inode, struct file *filp) {
	
	PDEBUG("Aufgabe: OPEN\n");

	return 0;
}

int aufgabe_release(struct inode *inode, struct file *filp) {
	// Nothing to do
	PDEBUG("Aufgabe: RELEASE\n");
	return 0;
}

ssize_t aufgabe_read(struct file *filp
, char *buffer,//Buffer denn man fuelt
size_t buffer_length,//Die lange des buffers
loff_t *f_offset//Unser offset im file
){
	int ubergeben = 0;
	char ausgabe_buffer[BUFFER_SIZE] = { 0 };
	
	
	if(*f_offset != 0){ //zweiter read befehl
		return 0;
	}
	
	sprintf(ausgabe_buffer,"%i  %u\n",letzte_zeile_zeichen_anzahl,zeit_seit_letzte_gelessene_zeile);
	
	ubergeben = MINIMUM(strlen(ausgabe_buffer),buffer_length);
	
	if(copy_to_user(buffer,ausgabe_buffer,ubergeben)){
		PDEBUG("Aufgabe: Daten wurden nicht gesendet");
	}
	else{
		PDEBUG("Aufgabe: Daten wurden gesendet");
	}
	
	PDEBUG("Aufgabe: Data mit der lange %d wird gelessen\n", ubergeben);
	
	*f_offset = ubergeben;
	return ubergeben;
	
}

ssize_t aufgabe_write(struct file *filp,const char *buffer,size_t buffer_length,loff_t *f_offset){

	u64 temp = 0;

	letzte_zeile_zeichen_anzahl = momentane_zeile_zeichen_anzahl;
	momentane_zeile_zeichen_anzahl = buffer_length;
	PDEBUG("Aufgabe: Data mit der lange %d und wert %s wird gelessen\n", buffer_length,buffer);
	
	temp = (u64)get_jiffies_64();
	zeit_seit_letzte_gelessene_zeile = temp - zeit_stempel;
	zeit_seit_letzte_gelessene_zeile = zeit_seit_letzte_gelessene_zeile * 1000 / HZ;
	zeit_stempel = temp;
	PDEBUG("Aufgabe: Schreiben ist fertig\n");
	return buffer_length;
}

struct file_operations fops = {
       .read = aufgabe_read,
       .write = aufgabe_write,
       .open = aufgabe_open,
       .release = aufgabe_release
    };

static int aufgabe_init(void){
	int result;
	dev_t dev = 0;
	
	PDEBUG("Aufgabe: Modul initialisation wurde gestartet\n");
	
	result = alloc_chrdev_region(&dev,MINOR_START,DEVICE_COUNT,"aufgabe");
	if (result < 0) {
		printk(KERN_WARNING "Cannot register Major number for timer!");
		return result;
	}
	
	aufgabe_major = MAJOR(dev);
	letzte_zeile_zeichen_anzahl = 0;
	zeit_seit_letzte_gelessene_zeile = 0;
	
	
	cdev_init(&my_cdev, &fops);
	
	result = cdev_add(&my_cdev, MKDEV(aufgabe_major, MINOR_START), 1);
	  if (result < 0) {
	    printk(KERN_WARNING "Cannot register forward timer!");
	    return result;
	  }
	
	PDEBUG("Aufgabe: Modul wurde mit der major numer %i gestartert\n",aufgabe_major);
	
	return 0;
}

static void aufgabe_exit(void){
	dev_t devno = MKDEV(aufgabe_major, MINOR_START);
	
	cdev_del(&my_cdev);	
	
	unregister_chrdev_region(devno,DEVICE_COUNT);
	PDEBUG("Aufgabe: Modul wurde aufgereumt\n");
}


module_init(aufgabe_init);
module_exit(aufgabe_exit);
