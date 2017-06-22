#include "aufgabe.h"


static int aufgabe_major;
static int momentane_zeile_zeichen_anzahl;
static u64 zeit_stempel;
static int zeit_seit_letzte_gelessene_zeile;
static struct cdev my_cdev;

static struct mutex aufgabe_mutex;

static int isOpen;

int aufgabe_open(struct inode *inode, struct file *filp) {
	
	if(isOpen == 1){
	  PDEBUG("OPEN FAILED\n");
	  return -EBUSY;
	}
  
	isOpen = 1;
	PDEBUG("OPEN\n");
	
	return 0;
}

int aufgabe_release(struct inode *inode, struct file *filp) {
	// Nothing to do
  
	isOpen = 0;
	PDEBUG("RELEASE\n");
	return 0;
}

ssize_t aufgabe_read(struct file *filp
, char *buffer,//Buffer denn man fuelt
size_t buffer_length,//Die lange des buffers
loff_t *f_offset//Unser offset im file
){
	int ubergeben = 0;
	char ausgabe_buffer[BUFFER_SIZE] = { 0 };
	
	PDEBUG("READ WIRD BENUTZ");
	
	mutex_lock(&aufgabe_mutex);
	
	if(*f_offset != 0){ //zweiter read befehl
		mutex_unlock(&aufgabe_mutex);
		return 0;
	}
	
	sprintf(ausgabe_buffer,"%i  %i\n",momentane_zeile_zeichen_anzahl,zeit_seit_letzte_gelessene_zeile);
	
	ubergeben = MINIMUM(strlen(ausgabe_buffer),buffer_length);
	
	if(copy_to_user(buffer,ausgabe_buffer,ubergeben)){
		PDEBUG("Daten wurden nicht gesendet\n");
	}
	else{
		PDEBUG("Daten wurden gesendet\n");
	}
	
	PDEBUG("Data mit der lange %d wird gelessen\n", ubergeben);
	
	*f_offset = ubergeben;
	
	mutex_unlock(&aufgabe_mutex);
	
	PDEBUG("READ WIRD GESCHLOSSEN\n");
	return ubergeben;
	
}

ssize_t aufgabe_write(struct file *filp,const char *buffer,size_t buffer_length,loff_t *f_offset){

	u64 temp = 0;
	PDEBUG("WRITE WIRD BENUTZ\n");
	
	mutex_lock(&aufgabe_mutex);
	
	momentane_zeile_zeichen_anzahl = buffer_length - 1;
	PDEBUG("Data mit der lange %d und wert %s wird gelessen\n", buffer_length,buffer);
	
	temp = (u64)get_jiffies_64();
	zeit_seit_letzte_gelessene_zeile = temp - zeit_stempel;
	zeit_seit_letzte_gelessene_zeile = zeit_seit_letzte_gelessene_zeile * 1000 / HZ;
	zeit_stempel = temp;
	PDEBUG("Schreiben ist fertig\n");
	
	mutex_unlock(&aufgabe_mutex);
	
	PDEBUG("WRITE WIRD GESCHLOSSEN\n");
	
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
	
	PDEBUG("Modul initialisation wurde gestartet\n");
	
	result = alloc_chrdev_region(&dev,MINOR_START,DEVICE_COUNT,"aufgabe");
	if (result < 0) {
		printk(KERN_WARNING "Cannot register Major number for timer!");
		return result;
	}
	
	aufgabe_major = MAJOR(dev);
	
	momentane_zeile_zeichen_anzahl = REF_SIZE;
	zeit_seit_letzte_gelessene_zeile = REF_TIME;
	
	zeit_stempel=(u64)get_jiffies_64();
	
	
	cdev_init(&my_cdev, &fops);
	
	result = cdev_add(&my_cdev, MKDEV(aufgabe_major, MINOR_START), 1);
	  if (result < 0) {
	    printk(KERN_WARNING "Cannot register forward timer!\n");
	    return result;
	  }
	
	PDEBUG("Modul wurde mit der major numer %i gestartert\n",aufgabe_major);
	
	isOpen = 0;
	mutex_init(&aufgabe_mutex);
	
	return 0;
}

static void aufgabe_exit(void){
	dev_t devno = MKDEV(aufgabe_major, MINOR_START);
	
	cdev_del(&my_cdev);
	
	mutex_destroy(&aufgabe_mutex);
	
	unregister_chrdev_region(devno,DEVICE_COUNT);
	PDEBUG("Modul wurde aufgereumt\n");
}


module_init(aufgabe_init);
module_exit(aufgabe_exit);
