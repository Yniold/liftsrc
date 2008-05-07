/*
 * $RCSfile: backplanedriver.c,v $ last changed on $Date: 2008-05-07 16:42:17 $ by $Author: rudolf $
 *
 * $Log: backplanedriver.c,v $
 * Revision 1.2  2008-05-07 16:42:17  rudolf
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

static int thread_id=0;
static wait_queue_head_t wq;
static DECLARE_COMPLETION(on_exit);

int iLoop;
unsigned long ulMyIOPort = 0L, ulMyIOLen = 0L, ulMyMemStart = 0L, ulMyMemLen = 0L;

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

   daemonize("MyKThread");
   allow_signal( SIGTERM );
   while(1)
     {
	timeout=HZ; // wait 1 second
	timeout=wait_event_interruptible_timeout(wq, (timeout==0), timeout);

	outb(*((unsigned long*)data),0xA5);

	printk("thread_code: woke up ...\n");
	if( timeout==-ERESTARTSYS )
	  {
	     printk("got signal, break\n");
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

   printk(KERN_NOTICE "$Id: backplanedriver.c,v 1.2 2008-05-07 16:42:17 rudolf Exp $ initialising\n");
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

   /* request IO region for BAR2 (serbus)*/
   ulMyIOPort = pci_resource_start(dev,2);
   ulMyIOLen  = pci_resource_len(dev,2);
   
   if(request_region(ulMyIOPort,ulMyIOLen,dev->dev.kobj.name) == NULL)
     {
	release_region(ulMyIOPort,ulMyIOLen);
	return -EIO;
     };
   
   /* do a test write on BAR2 */
   
   outb(ulMyIOPort,0xA5);
   
   /* create kernelthread */
   init_waitqueue_head(&wq);
   thread_id = kernel_thread(thread_code, &ulMyIOPort, CLONE_KERNEL);

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
   if(ulMyIOPort)
     release_region(ulMyIOPort,ulMyIOLen);
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
