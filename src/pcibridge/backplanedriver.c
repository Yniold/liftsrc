/*
 * $RCSfile: backplanedriver.c,v $ last changed on $Date: 2008-05-07 08:26:15 $ by $Author: rudolf $
 *
 * $Log: backplanedriver.c,v $
 * Revision 1.1  2008-05-07 08:26:15  rudolf
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

int iLoop;
unsigned long ulMyIOPort = 0L, ulMyIOLen = 0L, ulMyMemStart = 0L, ulMyMemLen = 0L;

static struct pci_device_id ids[] =
{
     { PCI_DEVICE(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_BACKPLANE), },
     { 0, }
};
MODULE_DEVICE_TABLE(pci, ids);

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

   printk(KERN_NOTICE "Backplanedriver $Id: backplanedriver.c,v 1.1 2008-05-07 08:26:15 rudolf Exp $ initialising\n");
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
	     printk(KERN_NOTICE "BPD: BAR%d is located in memspace, baseaddress @%0x08x, length=0x%08x\n",iLoop,pci_resource_start(dev,iLoop),pci_resource_len(dev,iLoop));
	  }
     };
   return 0;
}

static void remove(struct pci_dev *dev)
{
	/* clean up any allocated resources and stuff here.
	 * like call release_region();
	 */
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
