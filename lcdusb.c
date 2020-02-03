#include <usb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#define USB_TIMEOUT 500

unsigned short m_VendorID  = 0x03eb;
unsigned short m_ProductID = 0x1111;

#define m_hReadPipe  USB_ENDPOINT_IN + 1
#define m_hWritePipe USB_ENDPOINT_OUT + 1

typedef usb_dev_handle HANDLE;

HANDLE *m_hDevUsb;
int m_bConnected = 0;

static struct usb_device * USBFindDevice()
{
        struct usb_bus *bus;
        struct usb_device *dev;

        for (bus = usb_get_busses(); bus != NULL; bus = bus->next) 
        {
                for (dev = bus->devices; dev != NULL; dev = dev->next) 
                {
                        if ((dev->descriptor.idVendor == m_VendorID) &&
                             (dev->descriptor.idProduct == m_ProductID)) 
                        {
                                fprintf(stderr, "Found LCD on bus %s device %s\n", bus->dirname, dev->filename);
                                return dev;
                        }
                }
        }

        return NULL;
}

int USBConnect(void)
{
        struct usb_device *dev;

        usb_init();
        usb_find_busses();
        usb_find_devices();

        if ( (dev = USBFindDevice()) == NULL) {
                fprintf(stderr, "Device not found\n");
                return 0;
        }

        m_hDevUsb = usb_open(dev);
        /*
        if (usb_claim_interface(m_hDevUsb, 0) < 0)
                if ((usb_detach_kernel_driver_np(m_hDevUsb, 0) < 0) ||
                     (usb_claim_interface(m_hDevUsb, 0) < 0))
                        return 0;
	*/

        fprintf(stderr, "Succesfully claimed device\n");
        m_bConnected = 1;

        return 1;
}


void USBDisconnect(void)
{
        usb_release_interface(m_hDevUsb, 0);
}

long USBWriteReport(char *pReport, unsigned long dwReportSize)
{
        long bytesWritten = 0;

        if ( ! m_hDevUsb || ! m_bConnected) 
                return 0;

        bytesWritten = usb_bulk_write(m_hDevUsb, m_hWritePipe, pReport, dwReportSize, USB_TIMEOUT);

        //fprintf(stderr, "Wrote %li bytes\n", bytesWritten);

        if ( bytesWritten < 0 )
                fprintf(stderr, "Error writting to 0x%02x endpoint error was: %s\n", m_hWritePipe, strerror(errno));

        return bytesWritten;
}

long USBReadReport(char *pReport, unsigned long dwReportSize)
{
        long bytesRead = 0;

        if (! m_hDevUsb || ! m_bConnected)
                return 0;

        bytesRead = usb_bulk_read(m_hDevUsb, m_hReadPipe, pReport, dwReportSize, USB_TIMEOUT);

        fprintf(stderr, "Read %li bytes\n", bytesRead);

        if ( bytesRead < 0 )
                fprintf(stderr, "Error reading from 0x%02x endpoint error was: %s\n", m_hReadPipe, strerror(errno));

        return bytesRead;
}
