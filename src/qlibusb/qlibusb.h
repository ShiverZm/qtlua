#ifndef QLIBUSB_H
#define QLIBUSB_H

#include <QIODevice>
#include <QByteArray>
#include "libusb.h"
#include <QList>
#include <QMap>

struct QLibUsbInfo
{
    QLibUsbInfo(){}
    ~QLibUsbInfo(){}
    QLibUsbInfo(const QLibUsbInfo& info){
        this->bus = info.bus;
        this->port = info.port;
        this->address = info.address;
        this->speed = info.speed;
        this->deviceDescriptor = info.deviceDescriptor;
        this->ports = info.ports;
        this->path = info.path;
    }
    QLibUsbInfo& operator= (const QLibUsbInfo& info){
        this->bus = info.bus;
        this->port = info.port;
        this->address = info.address;
        this->speed = info.speed;
        this->deviceDescriptor = info.deviceDescriptor;
        this->ports = info.ports;
        this->path = info.path;
        return *this;
    }
    bool operator==(const QLibUsbInfo& info){
        return this->bus == info.bus
            && this->port == info.port
            && this->address == info.address
            && this->deviceDescriptor == info.deviceDescriptor;
    }
    QString toString()const{
        return path + QString("\nVID: %1, PID: %2, BUS:%3, Port:%4, Addr:%5, Speed:%6\n  Device Descriptor:\n  %7")
                .arg(VID(),4,16).arg(PID(),4,16).arg(bus).arg(port).arg(address).arg(speed).arg(deviceDescriptor.toHex().data());
    }
    int bus;
    int port;
    int address;
    int speed;
    int VID()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->idVendor; }
    int PID()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->idProduct; }
    int bcdDevice()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->bcdDevice;  }
    int deviceClass()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->bDeviceClass;  }
    int deviceSubClass()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->bDeviceSubClass;  }
    int deviceProtocol()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->bDeviceProtocol;  }
    int bcdUSB()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->bcdUSB;}
    int manufacture()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->iManufacturer;}
    int product()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->iProduct;}
    int serialNumber()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->iSerialNumber;}
    int configCount()const{ return ((struct libusb_device_descriptor*)deviceDescriptor.constData())->bNumConfigurations;}
    QString path;
    QByteArray deviceDescriptor;
    QList<int> ports;
};

struct QLibUsbEPInfo
{
    /** The address of the endpoint described by this descriptor. Bits 0:3 are
     * the endpoint number. Bits 4:6 are reserved. Bit 7 indicates direction,
     * see \ref libusb_endpoint_direction.
     */
    uint8_t  bEndpointAddress;
    /** Attributes which apply to the endpoint when it is configured using
     * the bConfigurationValue. Bits 0:1 determine the transfer type and
     * correspond to \ref libusb_transfer_type. Bits 2:3 are only used for
     * isochronous endpoints and correspond to \ref libusb_iso_sync_type.
     * Bits 4:5 are also only used for isochronous endpoints and correspond to
     * \ref libusb_iso_usage_type. Bits 6:7 are reserved.
     */
    uint8_t  bmAttributes;
    /** Maximum packet size this endpoint is capable of sending/receiving. */
    uint16_t wMaxPacketSize;
    /** Interval for polling endpoint for data transfers. */
    uint8_t  bInterval;
    bool isIn(void)const{
        return bEndpointAddress & 0x80;
    }
    QString toString()const{
        return QString("EndPoint: Addr=%1, Attr=%2, MaxPacket=%3, Interval=%4")
                .arg(bEndpointAddress).arg(bmAttributes).arg(wMaxPacketSize).arg(bInterval);
    }
    QLibUsbEPInfo(const struct libusb_endpoint_descriptor* ep){
        bEndpointAddress = ep->bEndpointAddress;
        bmAttributes = ep->bmAttributes;
        wMaxPacketSize = ep->wMaxPacketSize;
        bInterval = ep->bInterval;
    }
    QLibUsbEPInfo(const QLibUsbEPInfo& other){
        memcpy(this, &other, sizeof(other));
    }
    QLibUsbEPInfo& operator= (const QLibUsbEPInfo& other){
        memcpy(this, &other, sizeof(other));
        return *this;
    }
};
class QLibUsb;
struct QLibUsbInerfaceInfo
{
    /** Number of this interface */
    uint8_t  bInterfaceNumber;
    /** Value used to select this alternate setting for this interface */
    uint8_t  bAlternateSetting;
    /** Number of endpoints used by this interface (excluding the control
     * endpoint). */
    uint8_t  bNumEndpoints;
    /** USB-IF class code for this interface. See \ref libusb_class_code. */
    uint8_t  bInterfaceClass;
    /** USB-IF subclass code for this interface, qualified by the
     * bInterfaceClass value */
    uint8_t  bInterfaceSubClass;
    /** USB-IF protocol code for this interface, qualified by the
     * bInterfaceClass and bInterfaceSubClass values */
    uint8_t  bInterfaceProtocol;
    /** Index of string descriptor describing this interface */
    uint8_t  iInterface;
    QList<QLibUsbEPInfo> endpoints;

    QString toString()const{
        QString r = QString("Interface: Num=%1, AltSetting=%2, NumEndPoint=%3 Class=%4 SubClass=%5 Protocol=%6 IndexString=%7")
                .arg(bInterfaceNumber).arg(bAlternateSetting).arg(bNumEndpoints).arg(bInterfaceClass).arg(bInterfaceSubClass).arg(bInterfaceProtocol).arg(iInterface);
        foreach(const QLibUsbEPInfo& ep, endpoints){
            r = r + "\r\n" + ep.toString();
        }
        return r;
    }

    QLibUsbInerfaceInfo(const struct libusb_interface_descriptor* interface){
        bInterfaceNumber = interface->bInterfaceNumber;
        bAlternateSetting = interface->bAlternateSetting;
        bNumEndpoints = interface->bNumEndpoints;
        bInterfaceClass = interface->bInterfaceClass;
        bInterfaceSubClass = interface->bInterfaceSubClass;
        bInterfaceProtocol = interface->bInterfaceProtocol;
        iInterface = interface->iInterface;
        for(uint8_t i=0;i<bNumEndpoints;i++){
            const struct libusb_endpoint_descriptor* ep = interface->endpoint + i;
            endpoints.append(QLibUsbEPInfo(ep));
        }
    }
    QLibUsbInerfaceInfo(const QLibUsbInerfaceInfo& other){
        bInterfaceNumber = other.bInterfaceNumber;
        bAlternateSetting = other.bAlternateSetting;
        bNumEndpoints = other.bNumEndpoints;
        bInterfaceClass = other.bInterfaceClass;
        bInterfaceSubClass = other.bInterfaceSubClass;
        bInterfaceProtocol = other.bInterfaceProtocol;
        iInterface = other.iInterface;
        endpoints = other.endpoints;
    }
    QLibUsbInerfaceInfo& operator= (const QLibUsbInerfaceInfo& other){
        bInterfaceNumber = other.bInterfaceNumber;
        bAlternateSetting = other.bAlternateSetting;
        bNumEndpoints = other.bNumEndpoints;
        bInterfaceClass = other.bInterfaceClass;
        bInterfaceSubClass = other.bInterfaceSubClass;
        bInterfaceProtocol = other.bInterfaceProtocol;
        iInterface = other.iInterface;
        endpoints = other.endpoints;
        return *this;
    }
};

class QLibUsbEP
{
public:
    QLibUsbEP(QLibUsb& parent, const QLibUsbEPInfo& epInfo, int buffer_size = 4096);
    ~QLibUsbEP();
    bool init();
    QLibUsbEPInfo info()const{ return m_info; }
    static void LIBUSB_CALL completeCallback(libusb_transfer *transfer);
private:
    QLibUsb& m_parent;
    QLibUsbEPInfo m_info;
    struct libusb_transfer* m_xfer;
    uint8_t* m_buf;
    uint32_t m_bufSize;
};

class QLibUsb : public QObject
{
    Q_OBJECT
public:
    enum QueryMode {
        Polling,
        EventDriven
    };

    friend class QLibUsbEP;
    QLibUsb(QObject* parent = 0);
    virtual ~QLibUsb();

    bool open(const QLibUsbInfo& info, int bufSize);
    bool open(const QLibUsbInfo& info){ return open(info, 4096); }
    void close();
    int write(int epAddr, const QByteArray& data){ return write(epAddr, data, true, 10*1000);}
    int write(int epAddr, const QByteArray& data, bool needZeroPacket){ return write(epAddr, data, needZeroPacket, 10*1000);}
    int write(int epAddr, const QByteArray& data, bool needZeroPacket, unsigned int timeoutMS);
    QString indexStringAscii(int index);
    QByteArray indexString(int index) { return indexString(index,0); }
    QByteArray indexString(int index, uint16_t langId);
    QList<QLibUsbInerfaceInfo> interfaces() const { return m_interfaces; }
    QList<QLibUsbEPInfo> readEndpoints()const;
    QList<QLibUsbEPInfo> writeEndpoints()const { return m_writeEps.values(); }
    QLibUsbInfo deviceInfo()const{ return m_info; }
    QString interfaceInfo() const {
        QString r;
        foreach(const QLibUsbInerfaceInfo& it, m_interfaces){
            r = r + "\r\n" + it.toString();
        }
        return r;
    }
    static QList<QLibUsbInfo> enumDevices(int vid, int pid);
    static QList<QLibUsbInfo> enumDevices(int vid){ return enumDevices(vid,0); }
    static QList<QLibUsbInfo> enumDevices(){ return enumDevices(0,0); }
    static bool monitor(int vid, int pid);
    static bool monitor(int vid){ return monitor(vid,0); }
    static bool monitor(){ return monitor(0,0); }
    void hotplugCallback(const QLibUsbInfo& info, bool isArrival);

    static QString errorString(int err) { return QString(libusb_error_name(err)); }
    QString lastErrorString()const { return QString(libusb_error_name(m_lastError)); }
    int lastError() const { return m_lastError; }

    libusb_device_handle* handle(){return m_handle; }
    static int LIBUSB_CALL hotplugCallback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);
signals:
    void onConnect(const QLibUsbInfo& info);
    void onDisconnect(const QLibUsbInfo& info);
    void epDataReady(int ep, const QByteArray& str);
private:
    QLibUsbInfo m_info;
    libusb_device_handle* m_handle;
    QList<QLibUsbInerfaceInfo> m_interfaces;
    QList<QLibUsbEP*> m_readEps;
    QMap<int, QLibUsbEPInfo> m_writeEps;
    int m_lastError;
    static QMap<QLibUsb*, bool> m_devList;
};

#endif // QUSBHID_H
