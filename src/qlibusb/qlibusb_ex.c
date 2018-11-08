#include <QtGlobal>
#ifdef Q_OS_WIN

#include <windows.h>
#include <setupapi.h>
#include "libusbi.h"
#define GUID_DEVINTERFACE_USB_HOST_CONTROLLER)
#define GUID_DEVINTERFACE_USB_DEVICE
#define GUID_DEVINTERFACE_USB_HUB
#define GUID_DEVINTERFACE_LIBUSB0_FILTER
#include "os/windows_winusb.h"


const char* getDevicePath(struct libusb_device* dev)
{
   return  _device_priv(dev)->path;
}
#endif
