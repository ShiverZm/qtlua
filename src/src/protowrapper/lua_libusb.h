#ifndef LUA_LIBUSB_H
#define LUA_LIBUSB_H
#include "../qtwrapper/lua_qt_wrapper.hpp"
#include "../../qlibusb/qlibusb.h"

typedef class_<QLibUsb> LQLibUsb;
typedef class_<QLibUsbInfo> LQLibUsbInfo;
typedef class_<QLibUsbEPInfo> LQLibUsbEPInfo;
typedef class_<QLibUsbInerfaceInfo> LQLibUsbInerfaceInfo;
LQLibUsb lqlibusb();
LQLibUsbInfo lqlibusbinfo();
LQLibUsbEPInfo lqlibusbepinfo();
LQLibUsbInerfaceInfo lqlibusbinterfaceinfo();

#endif // LUA_USBHID_H
