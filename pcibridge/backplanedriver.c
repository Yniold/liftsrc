/*
 * $RCSfile: backplanedriver.c,v $ last changed on $Date: 2008-05-28 11:15:20 $ by $Author: rudolf $
 *
 * $Log: backplanedriver.c,v $
 * Revision 1.8  2008-05-28 11:15:20  rudolf
 * increased udelay due to slow serbus reading old value instead of current
 *
 * Revision 1.7  2008/05/28 10:55:32  rudolf
 * fixed compiler warnings, cosmetics
 *
 * Revision 1.6  2008/05/27 19:50:58  rudolf
 * added read and write IOCTLs
 *
 * Revision 1.5  2008/05/27 15:13:29  rudolf
 * integrated SYSFS and hotplug support
 *
 * Revision 1.4  2008/05/13 16:32:29  rudolf
 * added offsets for serbus registers
 *
 * Revision 1.3  2008/05/08 16:55:22  rudolf
 * more work on driver
 *
 * Revision 1.2  2008/05/07 16:42:17  rudolf
 * added kernelthread to write in 1s interval to PCI address
 *
 * Revision 1.1  2008/05/07 08:26:15  rudolf
 * checked in skeleton PCI driver for cPCI2Serbus brigde, work in progress
 *
 *
 *
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

#include "backplanedriver.h"

#define PCI_VENDOR_ID_ALTERA     (0x1172)
#define PCI_DEVICE_ID_BACKPLANE  (0x4711)
#define MAXBAR                   (6)

/* we do not decode the lane enables, so all adress offsets are multiples of ALIGNMENT */
#define ALIGNMENT                (4)

/* register for serial address on SERBUS, */
/* bit0 determines if it is a SERBUS read or write */
#define OFF_SER_ADDRESSREG       (0)

/* register for serial data on SERBUS, */
#define OFF_SER_DATAREG          (2)

/* register with serbus signature */
#define OFF_SER_STATUS           (4)
#define SERBUS_SIG               (0x4711)

// device major number
//
unsigned int serbus_major = 0;
int showDebug = 1;

static int thread_id=0;
static wait_queue_head_t wq;
static DECLARE_COMPLETION(on_exit);

static struct class *serbus_class;

/* structure for the mem ranges */
typedef struct sIORanges
{
   unsigned long ulSRAMStart;
   unsigned long ulSRAMLen;
   unsigned long ulPort80Start;
   unsigned long ulPort80Len;
   unsigned long ulSerbusPort;
   unsigned long ulSerbusLen;
}
sIORanges;

static struct sIORanges sIORange =
{
   0UL,0UL,0UL,0UL,0UL,0UL
};

int iLoop;

static struct pci_device_id ids[] =
{
     { PCI_DEVICE(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_BACKPLANE), },
     { 0, }
};
MODULE_DEVICE_TABLE(pci, ids);

unsigned short SerBusDoRead(unsigned short Address)
{
   unsigned short usReadData;

   if(showDebug)
     {

	printk("BPD: Reading from Address 0x%04X\n",Address);
	printk("BPD: Addressregister: @Offset 0x00 wrote:         0x%04X\n",Address | (unsigned short)(0x0001));
	printk("BPD: Dataregister:    @Offset 0x02 wrote (dummy): 0x%04X\n",0x0000);
     };

   /* initiate the read access
    * read access is characterized by setting the LSB to 1 */
   outw((Address | (unsigned short)(0x0001)),sIORange.ulSerbusPort+(OFF_SER_ADDRESSREG*ALIGNMENT));

   /* dummy access to trigger the FPGA statemachine */
   outw(0x0000,sIORange.ulSerbusPort+(OFF_SER_DATAREG*ALIGNMENT));

   /* serbus is slow so wait an usec (has to be investigated if it's
    * really neccessary to wait here) */
   udelay(5);

   usReadData = inw(sIORange.ulSerbusPort+(OFF_SER_DATAREG*ALIGNMENT));

   return(usReadData);
}

// open() device
int Serbus_Open(struct inode *inode, struct file *filp)
{
   if(showDebug)
     {
	printk("BPD: Serbus_Open() called...\n");
     };

   return 0;   // success
};

/* SERBUS WRITE SUBROUTINE
 * actual hardware access is done here */
void SerBusDoWrite(unsigned short Address, unsigned short Data)
{
   if(showDebug)
     {
	printk("BPD: Writing to Address 0x%04X\n",Address);
	printk("BPD: Addressregister: @Offset 0x00 wrote:         0x%04X\n",Address & 0xFFFE);
	printk("BPD: Dataregister:    @Offset 0x02 wrote:         0x%04X\n",Data);
     };

   /* initiate the read access
    * by clearing the MSB of  the address */

   outw((Address & 0xFFFE),sIORange.ulSerbusPort+(OFF_SER_ADDRESSREG*ALIGNMENT));
   outw((Data),sIORange.ulSerbusPort+(OFF_SER_DATAREG*ALIGNMENT));

   /* serbus is slow so wait an usec (has to be investigated if it's
    * really neccessary to wait here) */
   udelay(1);
};

// close() device
int Serbus_Release(struct inode *inode, struct file *filp)
{
   if(showDebug)
     printk("BPD: Serbus_Release() called...\n");

   return 0;
};

// read() device
static ssize_t Serbus_Read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
   if(showDebug)
     printk("BPD: Serbus_Read() called...\n");
   return (-EINVAL);
};

// write() device
static ssize_t Serbus_Write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
   if(showDebug)
     printk("BPD: Serbus_Write() called...\n");

   return (-EINVAL);
};

// ioctl() device
static int Serbus_IOCtl(struct inode *inode, struct file *filp, unsigned int uiCommand, unsigned long ulParam)
{
   static unsigned int uiSerbusAddress = 0;
   static unsigned int uiSerbusData = 0;

   // do some sanity checks
   int err=0;
   int size= _IOC_SIZE(uiCommand);

   if(showDebug)
     printk("BPD: Serbus_IOCtl() called, uiCommand: 0x%08X ulParam: 0x%08lX\n", uiCommand, ulParam);

   if (_IOC_TYPE(uiCommand) != SERBUS_IOC_MAGIC) // check for our magic byte
     {
	printk("BPD: IOCTL: magic byte mismatch\n");
	return -EINVAL;
     }

   if (_IOC_NR(uiCommand) > SERBUS_IOC_MAXNR)// check for greatest IOCTL code
     {
	printk("BPD: IOCTL: max number exceeded\n");
	return -EINVAL;
     }
   if (_IOC_DIR(uiCommand) & _IOC_READ)// check userspace buffers
     err = !access_ok(VERIFY_WRITE, (void *)ulParam, size);
   else
     // check kernelspace buffers
     if (_IOC_DIR(uiCommand) & _IOC_WRITE)
       err =  !access_ok(VERIFY_READ, (void *)ulParam, size);

   // any trouble so far ?
   if (err)
     {
	printk("BPD: IOCTL: trouble with userspace buffers\n");
	return err;
     };
   // command processing
   //
   switch(uiCommand)
     {
      case  SERBUS_IOCSDEBUGON:
	printk("BPD: ioctl(SERBUS_IOCSDEBUGON) called.\n");
	showDebug = 1;
	break;

      case SERBUS_IOCSDEBUGOFF:
	printk("BPD: ioctl(SERBUS_IOCSDEBUGOFF) called.\n");
	showDebug = 0;
	break;

      case SERBUS_IOCTWRITEWORD:
	if(showDebug)
	  printk("BPD: ioctl(SERBUS_IOCTWRITEWORD) called.\n");
	uiSerbusData    = (unsigned int)(ulParam >> 16);
	uiSerbusAddress = (unsigned int)(ulParam & (unsigned long)(0x0000FFFF));

	SerBusDoWrite(uiSerbusAddress, uiSerbusData);

	// debug data
	if(showDebug)
	  {
	     printk("BPD: Address: 0x%08X\n",uiSerbusAddress);
	     printk("BPD: Data:    0x%08X\n",uiSerbusData);
	  };
	break;

      case SERBUS_IOCHREADWORD:
	if(showDebug)
	  printk("BPD: ioctl(SERBUS_IOCHREADWORD) called.\n");
	uiSerbusAddress = (unsigned int)(ulParam & (unsigned long)(0x0000FFFF));

	// debug data
	if(showDebug)
	  printk("BDP: Address: 0x%08X\n",uiSerbusAddress);

	uiSerbusData = (unsigned int) SerBusDoRead((unsigned short) uiSerbusAddress);

	if(showDebug)
	  printk("BPD: Data:    0x%08X\n",uiSerbusData);

	return (unsigned long) uiSerbusData;
	break;

      default:
	return -ENOTTY;
     }
   return (0);
};

static struct file_operations serbus_fops =
{

   .owner   = THIS_MODULE,
     .write   = Serbus_Write,
     .read    = Serbus_Read,
     .ioctl   = Serbus_IOCtl,
     .open    = Serbus_Open,
     .release = Serbus_Release,
}
;

/* Kernel Thread for writing testdata periodically to PCI bus */
static int thread_code( void *data )
{
   unsigned long timeout;

   sIORanges* sStruct = (sIORanges*)data;

   daemonize("BPDDebugThread");
   allow_signal( SIGTERM );
   while(1)
     {
	timeout=HZ; // wait 1 second
	timeout=wait_event_interruptible_timeout(wq, (timeout==0), timeout);


	if( timeout==-ERESTARTSYS )
	  {
	     printk("BPD: Kernelthread got SIGRESTARTSYS, terminating.\n");
	     break;
	  }
     }
   thread_id = 0;
   complete_and_exit( &on_exit, 0 );
}

static unsigned char backplane_get_revision(struct pci_dev *dev)
{
   u8 revision;

   pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
   return revision;
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
   pci_enable_device(dev);

   /* check hardware revision */
   if (backplane_get_revision(dev) != 0x01)
     return -ENODEV;

   printk(KERN_NOTICE "$Id: backplanedriver.c,v 1.8 2008-05-28 11:15:20 rudolf Exp $ initialising\n");
   printk(KERN_NOTICE "BPD: cPCI2Serbus Bridge HW Revision %d found.\n",backplane_get_revision(dev));

   /* iterate through all BARs an print informations */
   for(iLoop=0;iLoop < MAXBAR;iLoop++)
     {
	if (pci_resource_flags(dev,iLoop) & IORESOURCE_IO)
	  {
	     printk(KERN_NOTICE "BPD: BAR%d is located in IO-space, baseaddress @0x%08x, length=0x%08x\n",iLoop,pci_resource_start(dev,iLoop),pci_resource_len(dev,iLoop));
	  }
	if (pci_resource_flags(dev,iLoop) & IORESOURCE_MEM)
	  {
	     printk(KERN_NOTICE "BPD: BAR%d is located in memspace, baseaddress @0x%08x, length=0x%08x\n",iLoop,pci_resource_start(dev,iLoop),pci_resource_len(dev,iLoop));
	  }
     };

   /* request MEM region for BAR0 (SRAM)*/
   sIORange.ulSRAMStart = pci_resource_start(dev,0);
   sIORange.ulSRAMLen   = pci_resource_len(dev,0);

   /* request IO region for BAR2 (serbus)*/
   sIORange.ulSerbusPort = pci_resource_start(dev,2);
   sIORange.ulSerbusLen  = pci_resource_len(dev,2);

   /* check if we have successfully requested the IO region for BAR0 */
   if(request_mem_region(sIORange.ulSRAMStart,sIORange.ulSRAMLen,dev->dev.kobj.name) == NULL)
     {
	release_mem_region(sIORange.ulSRAMStart,sIORange.ulSRAMLen);
	printk(KERN_ERR "BPD: can't request region for BAR0.\n");
	return -EIO;
     };

   /* check if we have successfully requested the IO region for BAR2 */
   if(request_region(sIORange.ulSerbusPort,sIORange.ulSerbusLen,dev->dev.kobj.name) == NULL)
     {
	release_region(sIORange.ulSerbusPort,sIORange.ulSerbusLen);
	printk(KERN_ERR "BPD: can't request region for BAR2.\n");
	return -EIO;
     };

   /* create kernelthread, pass pointer to structure to make mem areas */
   /* available to thread as well */
   init_waitqueue_head(&wq);
   thread_id = kernel_thread(thread_code, &sIORange, CLONE_KERNEL);

   if(thread_id == 0)
     return -EIO;
   return 0;
}

static void remove(struct pci_dev *dev)
{
   /* kill thread and wait for termination*/
   if(thread_id)
     kill_proc(thread_id, SIGTERM, 1);
   wait_for_completion(&on_exit);

   /* check for requested IO space and release it*/
   if(sIORange.ulSRAMStart)
     release_mem_region(sIORange.ulSRAMStart,sIORange.ulSRAMLen);

   /* check for requested IO space and release it*/
   if(sIORange.ulSerbusPort)
     release_region(sIORange.ulSerbusPort,sIORange.ulSerbusLen);
}

static struct pci_driver backplane_driver =
{
   .name = "backplanedriver",
     .id_table = ids,
     .probe = probe,
     .remove = remove,
};

static int __init pci_backplane_init(void)
{
   int iRetVal;
   /* returns number of found PCI boards */
   iRetVal = pci_register_driver(&backplane_driver);

   /* check if one board found */
   if(iRetVal == 0)
     {
	/* try to register a character device */
	iRetVal = register_chrdev(serbus_major,"backplanedriver",&serbus_fops);

	if(iRetVal)
	  {
	     /* save our new assigned major number we got from the kernel */
	     serbus_major = iRetVal;
	     printk(KERN_NOTICE "BPD: successfully registered chardev. Got major number 0x%02X from kernel.\n",serbus_major);

	     /* create a simple class to be able to use udev for creating the devnodes automagically */
	     serbus_class = class_create(THIS_MODULE, "serbus");
	     if(IS_ERR(serbus_class))
	       {
		  printk(KERN_ERR "BPD: Error creating serbus class.\n");
	       }
	     else
	       {
		  printk(KERN_NOTICE "BPD: created serbus class in SYSFS.\n");
		  class_device_create(serbus_class, NULL , MKDEV(serbus_major,0), NULL, "serbus");
	       };
	  }
	else
	  {
	     printk(KERN_NOTICE "BPD: register_chrdev() failed with %d\n",iRetVal);
	  }
     }
   return iRetVal;
}

static void __exit pci_backplane_exit(void)
{
   printk(KERN_NOTICE "BPD: cleaning up...\n");
   class_device_destroy(serbus_class,MKDEV(serbus_major,0));
   class_destroy(serbus_class);
   unregister_chrdev(serbus_major,"serbus");
   pci_unregister_driver(&backplane_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Markus Rudolf");
MODULE_DESCRIPTION("MPI for Chemistry SERBUS Access Driver for the cPCI Interface Board");
module_init(pci_backplane_init);
module_exit(pci_backplane_exit);
