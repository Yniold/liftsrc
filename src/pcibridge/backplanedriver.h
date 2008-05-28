/*
* $RCSfile: backplanedriver.h,v $ last changed on $Date: 2008-05-28 10:55:32 $ by $Author: rudolf $
*
* $Log: backplanedriver.h,v $
* Revision 1.3  2008-05-28 10:55:32  rudolf
* fixed compiler warnings, cosmetics
*
* Revision 1.2  2008/05/27 15:13:29  rudolf
* integrated SYSFS and hotplug support
*
* Revision 1.1  2008/05/26 15:01:41  rudolf
* added headerfile
*
*
*
*/

#ifndef __SERIALBUS_H__
#define __SERIALBUS_H__

// needed for _IOW() macro
#include <linux/fs.h>

//ioctl() defines

#define SERBUS_SETDEBUGON 1
#define SERBUS_SETDEBUGOFF 2
#define SERBUS_WRITEWORD 3
#define SERBUS_READWORD 4

#define SERBUS_IOC_MAGIC 's'

#define SERBUS_IOCRESET _IO(SERBUS_IOC_MAGIC, 0)
/*
* S means "Set" through a ptr
* T means "Tell" directly with the argument value
* G means "Get": reply by setting through a pointer
* Q means "Query": response is on the return value
* X means "eXchange": G and S atomically
* H means "sHift": T and Q atomically
*/
#define SERBUS_IOCSDEBUGON   _IOW(SERBUS_IOC_MAGIC, SERBUS_SETDEBUGON,int)
#define SERBUS_IOCSDEBUGOFF  _IOW(SERBUS_IOC_MAGIC, SERBUS_SETDEBUGOFF,int)
#define SERBUS_IOCTWRITEWORD _IOW(SERBUS_IOC_MAGIC, SERBUS_WRITEWORD,int)
#define SERBUS_IOCHREADWORD  _IOW(SERBUS_IOC_MAGIC, SERBUS_READWORD,int)

#define SERBUS_IOC_MAXNR 4

// standardfunctions for CHARDEV

static int Serbus_Open(struct inode *inode, struct file *filp);
static int Serbus_Release(struct inode *inode, struct file *filp);
static int Serbus_IOCtl(struct inode *inode, struct file *filp, unsigned int uiCommand, unsigned long ulParam);
static ssize_t Serbus_Read(struct file *filp, char *buff, size_t count, loff_t *offp);
static ssize_t Serbus_Write(struct file *filp, const char *buff, size_t count, loff_t *offp);

#endif
