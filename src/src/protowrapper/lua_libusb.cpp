#include "lua_libusb.h"
#include "luabind/tag_function.hpp"
#include "../qtwrapper/qluaslot.h"
#include "luabind/out_value_policy.hpp"

LQLibUsbInfo lqlibusbinfo()
{
    return
    class_<QLibUsbInfo>("QLibUsbInfo")
    .def(constructor<>())
    .def_readwrite("address", &QLibUsbInfo::address)
    .def_readwrite("port", &QLibUsbInfo::port)
    .def_readwrite("ports", &QLibUsbInfo::ports)
    .def_readwrite("speed", &QLibUsbInfo::speed)
    .def_readwrite("path", &QLibUsbInfo::path)
    .def_readonly("deviceDescriptor", &QLibUsbInfo::deviceDescriptor)
    .def_readonly("descriptor", &QLibUsbInfo::deviceDescriptor)
    .property("infoString", &QLibUsbInfo::toString)
    .property("vid", &QLibUsbInfo::VID)
    .property("pid", &QLibUsbInfo::PID)
    .property("bcdUSB", &QLibUsbInfo::bcdUSB)
    .property("bcdDevice", &QLibUsbInfo::bcdDevice)
    .property("deviceClass", &QLibUsbInfo::deviceClass)
    .property("deviceSubClass", &QLibUsbInfo::deviceSubClass)
    .property("deviceProtocol", &QLibUsbInfo::deviceProtocol)
    .property("manufacture", &QLibUsbInfo::manufacture)
    .property("product", &QLibUsbInfo::product)
    .property("serialNumber", &QLibUsbInfo::serialNumber)
    .property("configCount", &QLibUsbInfo::configCount)
    .def("toString", &QLibUsbInfo::toString)
    .def("__tostring", &QLibUsbInfo::toString)
    ;
}


LQLibUsbEPInfo lqlibusbepinfo()
{
    return
    class_<QLibUsbEPInfo>("QLibUsbEPInfo")
    //.def(constructor<>())
    .def(constructor<const QLibUsbEPInfo&>())
    .def_readonly("bEndpointAddress", &QLibUsbEPInfo::bEndpointAddress)
    .def_readonly("bmAttributes", &QLibUsbEPInfo::bmAttributes)
    .def_readonly("wMaxPacketSize", &QLibUsbEPInfo::wMaxPacketSize)
    .def_readonly("bInterval", &QLibUsbEPInfo::bInterval)
    .def_readonly("addr", &QLibUsbEPInfo::bEndpointAddress)
    .def_readonly("attr", &QLibUsbEPInfo::bmAttributes)
    .def_readonly("size", &QLibUsbEPInfo::wMaxPacketSize)
    .def_readonly("interval", &QLibUsbEPInfo::bInterval)
    .property("isIn", &QLibUsbEPInfo::isIn)
    .property("infoString", &QLibUsbEPInfo::toString)
    .def("toString", &QLibUsbEPInfo::toString)
    .def("__tostring", &QLibUsbEPInfo::toString)
    ;
}

LQLibUsbInerfaceInfo lqlibusbinterfaceinfo()
{
    return
    class_<QLibUsbInerfaceInfo>("QLibUsbInerfaceInfo")
    //.def(constructor<>())
    .def_readonly("bInterfaceNumber", &QLibUsbInerfaceInfo::bInterfaceNumber)
    .def_readonly("bAlternateSetting", &QLibUsbInerfaceInfo::bAlternateSetting)
    .def_readonly("bInterfaceClass", &QLibUsbInerfaceInfo::bInterfaceClass)
    .def_readonly("bInterfaceSubClass", &QLibUsbInerfaceInfo::bInterfaceSubClass)
    .def_readonly("bInterfaceProtocol", &QLibUsbInerfaceInfo::bInterfaceProtocol)
    .def_readonly("iInterface", &QLibUsbInerfaceInfo::iInterface)
    .def_readonly("endpoints", &QLibUsbInerfaceInfo::endpoints)
    .property("infoString", &QLibUsbInerfaceInfo::toString)
    .def("toString", &QLibUsbInerfaceInfo::toString)
    .def("__tostring", &QLibUsbInerfaceInfo::toString)
    ;
}


SIGNAL_PROPERYT(lqlibusb,onConnect, QLibUsb, "(const QLibUsbInfo &)")
SIGNAL_PROPERYT(lqlibusb,onDisconnect, QLibUsb, "(const QLibUsbInfo &)")
SIGNAL_PROPERYT(lqlibusb,epDataReady, QLibUsb, "(int, const QByteArray&)")
LQLibUsb lqlibusb()
{
    return
    class_<QLibUsb>("QLibUsb")
    .def(constructor<>())
    .def(constructor<QObject*>())
    .def("open", (bool (QLibUsb::*)(const QLibUsbInfo&, int))&QLibUsb::open)
    .def("open", (bool (QLibUsb::*)(const QLibUsbInfo&))&QLibUsb::open)
    .def("close", &QLibUsb::close)
    .def("write", (int (QLibUsb::*)(int, const QByteArray&, bool, unsigned int))&QLibUsb::write)
    .def("write", (int (QLibUsb::*)(int, const QByteArray&, bool))&QLibUsb::write)
    .def("write", (int (QLibUsb::*)(int, const QByteArray&))&QLibUsb::write)
    .def("indexStringAscii", &QLibUsb::indexStringAscii)
    .def("indexString", (QByteArray (QLibUsb::*)(int, uint16_t))&QLibUsb::indexString)
    .def("indexString", (QByteArray (QLibUsb::*)(int))&QLibUsb::indexString)
    .property("interfaces", &QLibUsb::interfaces)
    .property("readEndpoints", &QLibUsb::readEndpoints)
    .property("writeEndpoints", &QLibUsb::writeEndpoints)
    .property("deviceInfo", &QLibUsb::deviceInfo)
    .property("interfaceInfo", &QLibUsb::interfaceInfo)
    .property("lastErrorString", &QLibUsb::lastErrorString)
    .property("lastError", &QLibUsb::lastError)
    .sig_prop(lqlibusb,onConnect)
    .sig_prop(lqlibusb,onDisconnect)
    .sig_prop(lqlibusb,epDataReady)
    .scope[
        def("enumDevices", (QList<QLibUsbInfo>(*)(int,int))&QLibUsb::enumDevices),
        def("enumDevices", (QList<QLibUsbInfo>(*)(int))&QLibUsb::enumDevices),
        def("enumDevices", (QList<QLibUsbInfo>(*)())&QLibUsb::enumDevices),
        def("monitor", (bool(*)(int,int))&QLibUsb::monitor),
        def("monitor", (bool(*)(int))&QLibUsb::monitor),
        def("monitor", (bool(*)())&QLibUsb::monitor),
        def("errorString", &QLibUsb::errorString)
    ]
    ;
}
