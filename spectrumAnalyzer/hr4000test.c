#include <stdio.h>
#include <usb.h>

#define OOI_VENDOR_ID 0x2457
#define OOI_HR4000_ID 0x1012

#define INPUT_BUFFER_SIZE 7681

#define USB_BULK_TIMEOUT 50

int main()
{

    struct usb_bus *bus = 0;
    struct usb_device *device = 0;
    struct usb_dev_handle *hdev = 0;
    int interface = 0;
    int bytes_read = 0;
    int i = 0, pixel_value = 0;
    unsigned char input_buffer[INPUT_BUFFER_SIZE];
    unsigned char spectrum_request[] = { 0x09 };

    usb_init();

    printf("Finding devices.\n");
    usb_find_busses();
    usb_find_devices();

    for(bus = usb_busses; bus; bus = bus->next)
        {
            for(device = bus->devices; device; device = device->next)
                {
                    if(0 == device->descriptor.idVendor
                        && 0 == device->descriptor.idProduct)
                        continue;

                    printf("Checking device: vendor 0x%4x, product 0x%4x\n",
                        device->descriptor.idVendor,
                        device->descriptor.idProduct);

                    if(OOI_VENDOR_ID == device->descriptor.idVendor
                        && OOI_HR4000_ID == device->descriptor.idProduct)
                        break;
                }
            if(0 != device)
                break;
        }

    if(0 == device)
        {
            printf("HR4000 not found.\n");
            return;
        }

    if(!(hdev = usb_open(device)))
        {
            perror("usb_open() failed");
        }
    interface = device->config->interface->altsetting->bInterfaceNumber;
    if(usb_claim_interface(hdev, interface) != 0)
        {
            perror("usb_claim_interface failed");
            return;
        }

    usb_clear_halt(hdev, 0x01);
    usb_clear_halt(hdev, 0x82);
    usb_clear_halt(hdev, 0x86);
    usb_clear_halt(hdev, 0x81);

    usb_bulk_write(hdev, 0x01, spectrum_request, 1, USB_BULK_TIMEOUT);

    bytes_read = usb_bulk_read(hdev, 0x82, input_buffer,
        INPUT_BUFFER_SIZE, USB_BULK_TIMEOUT);

    printf("Read %d bytes.\n", bytes_read);
    if(bytes_read < INPUT_BUFFER_SIZE)
        {
            printf("Read was short, giving up.\n");
            return;
        }

    if(input_buffer[bytes_read - 1] != 0x69)
        {
            printf("Did not get sync byte.  Giving up.\n");
            return;
        }

    for(i = 0; i < 3648; i++)
        {
            pixel_value =
                (input_buffer[i*2] | ((input_buffer[i*2+1]) << 8)) ^
0x00002000;
            printf("Pixel %d value is %d\n", i, pixel_value);
        }
}

