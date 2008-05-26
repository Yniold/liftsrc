/*
* $RCSfile: backplanedriver.h,v $ last changed on $Date: 2008-05-26 15:01:41 $ by $Author: rudolf $
*
* $Log: backplanedriver.h,v $
* Revision 1.1  2008-05-26 15:01:41  rudolf
* added headerfile
*
*
*
*/

#ifndef __serialbus_h__
#define __serialbus_h__

#define MYIOBASE_CS1 0x12000000
#define MYIOBASE_CS3 0x14000000
#define ATA_OFFSET 0x00020000

#define CHIPSELECT_CONFIGRANGE 0x00220000
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
#define SERBUS_IOCSDEBUGON _IOW(SERBUS_IOC_MAGIC, 1, SERBUS_SETDEBUGON)
#define SERBUS_IOCSDEBUGOFF_IOW(SERBUS_IOC_MAGIC, 2, SERBUS_SETDEBUGOFF)
#define SERBUS_IOCTWRITEWORD_IOW(SERBUS_IOC_MAGIC, 3, SERBUS_WRITEWORD)
#define SERBUS_IOCHREADWORD_IOW(SERBUS_IOC_MAGIC, 4, SERBUS_READWORD)

#define SERBUS_IOC_MAXNR 4

// standardfunctions for CHARDEV

int Serbus_Open(struct inode *inode, struct file *filp);
int Serbus_Release(struct inode *inode, struct file *filp);
int Serbus_IOCtl(struct inode *inode, struct file *filp, unsigned int uiCommand, unsigned long ulParam);
ssize_t Serbus_Read(struct inode *inode, struct file *filp, char *buff, size_t count, loff_t *offp);
ssize_t Serbus_Write(struct inode *inode, struct file *filp, const char *buff, size_t count, loff_t *offp);

// driver functions
int SerBusInitialiseTimings(void);
unsigned short SerBusDoRead(unsigned short Address);
void SerBusDoWrite(unsigned short Address, unsigned short Data);

#endif
  
  