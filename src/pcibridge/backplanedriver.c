/*
 * $RCSfile: backplanedriver.c,v $ last changed on $Date: 2008-05-13 16:32:29 $ by $Author: rudolf $
 *
 * $Log: backplanedriver.c,v $
 * Revision 1.4  2008-05-13 16:32:29  rudolf
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

static int thread_id=0;
static wait_queue_head_t wq;
static DECLARE_COMPLETION(on_exit);

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

/* Kernel Thread for writing testdata periodically to PCI bus */
static int thread_code( void *data )
{
   unsigned long timeout;
   unsigned char ucBuffer[4];
   volatile unsigned short usDummy1;
   volatile unsigned short usDummy2;

   volatile unsigned long  ulDummy;
   void* pAddress;
   sIORanges* sStruct = (sIORanges*)data;

   daemonize("DebugThread");
   allow_signal( SIGTERM );
   while(1)
     {
	timeout=HZ; // wait 1 second
	timeout=wait_event_interruptible_timeout(wq, (timeout==0), timeout);

	/* test write words on consecutive addresses to test CBEN[0..3] on bridge */
	outw(0x0123,sStruct->ulSerbusPort+(OFF_SER_ADDRESSREG*ALIGNMENT));
	outw(0x5678,sStruct->ulSerbusPort+(OFF_SER_DATAREG*ALIGNMENT));

	/* test read words on consecutive addresses to test CBEN[0..3] on bridge */
	usDummy1 = inw(sStruct->ulSerbusPort+(OFF_SER_ADDRESSREG*ALIGNMENT));
	usDummy2 = inw(sStruct->ulSerbusPort+(OFF_SER_DATAREG*ALIGNMENT));

	printk("Read %04x from PCI bus @0x0\n",usDummy1);
	printk("Read %04x from PCI bus @0x2\n",usDummy2);

	if( timeout==-ERESTARTSYS )
	  {
	     printk("got signal, break.\n");
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

   printk(KERN_NOTICE "$Id: backplanedriver.c,v 1.4 2008-05-13 16:32:29 rudolf Exp $ initialising\n");
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
   printk(KERN_NOTICE "BPD: init()\n");
   return pci_register_driver(&backplane_driver);
}

static void __exit pci_backplane_exit(void)
{
   printk(KERN_NOTICE "BPD: exit()\n");
   pci_unregister_driver(&backplane_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Markus Rudolf");
module_init(pci_backplane_init);
module_exit(pci_backplane_exit);
