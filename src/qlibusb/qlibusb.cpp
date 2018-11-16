#include "qlibusb.h"
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

//#define ERR(args...)  sprintf(stderr, ##args)
//#define WARNING(args...)  sprintf(stdout, ##args)
//#define DBG(args...)  sprintf(stdout, ##args)

#define QDBG(tag, args...) do{\
    char temp[4096] = tag;\
    sprintf(temp + 7, ##args);\
    qDebug()<<temp;\
}while(0)
#define ERR(args...)      QDBG("ERROR   ", ##args)
#define WARNING(args...)  QDBG("WARNING ", ##args)
#define DBG(args...)      QDBG("DEBUG   ", ##args)

class QLibUsbMonitor : protected QThread
{
public:
    static void SetSupportHotplug(bool hotplug){
        geteMonitor()->setSupportHotplug(hotplug);
    }
private:
static QLibUsbMonitor* theMonitor;
static QLibUsbMonitor* geteMonitor(){
    if(!theMonitor) theMonitor = new QLibUsbMonitor();
    return theMonitor;
}
private:
    QLibUsbMonitor(QObject *parent = 0):QThread(parent),m_supportHotplug(false),m_devCount(0),m_mutex(){}

    void setSupportHotplug(bool hotplug) {
        m_supportHotplug = hotplug;
        if(m_supportHotplug){
            start();
        }
    }
    void addDevice(void){
        QMutexLocker locker(&m_mutex);
        m_devCount++;
        if(m_supportHotplug)return;
        if(m_devCount == 1){
            if(isRunning()){
                qDebug()<<"Monitor thread is running, something error";
            }else{
                start();
            }
        }
    }
    void releaseDevice(void){
        QMutexLocker locker(&m_mutex);
        if(m_devCount>0){
            m_devCount--;
        }
    }
    int deviceCount() {
        QMutexLocker locker(&m_mutex);
        return m_devCount;
    }
protected:
    virtual void run(){
        qDebug() << "QLibUsb monitor thread running: " << QThread::currentThreadId();
        while (true)
        {
            if(deviceCount()>0){
                int r = libusb_handle_events(NULL);
                qDebug()<<"handle return"<<r <<" "<<libusb_error_name(r);
                //if(r == LIBUSB_ERROR_NOT_FOUND)return;
                this->msleep(1);
            }else{
                qDebug()<<"Device count is 0, quit the monitor thread";
                return;
            }
            //if (r < 0)
            //    ERR("libusb_handle_events() failed: %s", libusb_error_name(r));
        }
    }
private:
    bool m_supportHotplug;
    int  m_devCount;
    QMutex m_mutex;
};
QLibUsbMonitor* QLibUsbMonitor::theMonitor = 0;


void QLibUsbThread::startMonitor(libusb_context* ctx)
{
    m_ctx = ctx;
    if(m_ctx){
        if(!isRunning()){
            start();
        }
    }
}
void QLibUsbThread::stopMonitor()
{
    if(isRunning()){
        libusb_context* ctx = m_ctx;
        m_ctx = NULL;
        this->wait();
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        if(ctx){
            libusb_handle_events_timeout(ctx, &tv);
        }

    }
}

void QLibUsbThread::run()
{
    qDebug() << "QLibUsb monitor thread running: " << QThread::currentThreadId();
    while (true)
    {
        if(m_ctx){
            int r = libusb_handle_events(m_ctx);
            qDebug()<<"QLibUsbThread handle return"<<r <<" "<<libusb_error_name(r);
            this->msleep(1);
        }else{
            qDebug()<<"Context is NULL, Quit the QLibUsbThread thread";
            return;
        }
    }
}

static int LIBUSB_CALL hotplug_callback_attach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    return 0;
}

static int LIBUSB_CALL hotplug_callback_detach(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    return 0;
}


static int init_usb_env(void)
{
    static bool inited = false;
    if(inited)return 0;
    inited = true;
    int r = libusb_init(NULL);
    if (r < 0)
    {
        ERR("failed to initialise libusb: %s", libusb_error_name(r));
    }
    QLibUsbMonitor::SetSupportHotplug(libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)!=0);
    return r;
}

static int deinit_usb_env(void)
{
    libusb_exit(NULL);
    return 0;
}

extern "C" const char* getDevicePath(struct libusb_device *dev);

static bool get_usb_info(libusb_device *dev, QLibUsbInfo &info, int vid = 0, int pid = 0)
{
    if (dev)
    {
        int r;
        struct libusb_device_descriptor desc;
        if(!vid) vid = LIBUSB_HOTPLUG_MATCH_ANY;
        if(!pid) pid = LIBUSB_HOTPLUG_MATCH_ANY;
        r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0)
        {
            WARNING("Fail to get usb device descriptor %d", r);
            return false;
        }
        else
        {
            //DBG("USB Match VID %x %x PID %x %x", VID, desc.idVendor, PID, desc.idProduct);
            if (((vid == LIBUSB_HOTPLUG_MATCH_ANY) || (desc.idVendor == vid)) && ((pid == LIBUSB_HOTPLUG_MATCH_ANY) || (desc.idProduct == pid)))
            {
                uint8_t ports[32] = {0};
                info.bus = libusb_get_bus_number(dev);
                info.address = libusb_get_device_address(dev);
                info.port = libusb_get_port_number(dev);
                info.speed = libusb_get_device_speed(dev);
                int path_len = libusb_get_port_numbers(dev, ports, sizeof(ports));
                info.ports.clear();
                info.path = QString(getDevicePath(dev));
                if(path_len > 0){
                    for(int i=0;i<path_len;i++){
                        info.ports.append(ports[i]);
                    }
                }
                info.deviceDescriptor = QByteArray((char*)&desc, sizeof(desc));
                return true;
            }
        }
    }
    return false;
}

QMap<QLibUsb*, bool> QLibUsb::m_devList;
int LIBUSB_CALL QLibUsb::hotplugCallback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data)
{
    foreach(QLibUsb* d, m_devList.keys()){
        QLibUsbInfo info;
        if(get_usb_info(dev, info)){
            d->hotplugCallback(info, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event);
        }
    }
    return 0;
}

bool QLibUsb::monitor(int vid, int pid)
{
    int r;
    init_usb_env();
    if(!vid) vid = LIBUSB_HOTPLUG_MATCH_ANY;
    if(!pid) pid = LIBUSB_HOTPLUG_MATCH_ANY;
    if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))
    {
        WARNING("Hotplug capabilites are not supported on this platform\n");
        return false;
    }
    else
    {
        r = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, LIBUSB_HOTPLUG_NO_FLAGS, vid,
                                             pid, LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback_attach, NULL, NULL);
        if (LIBUSB_SUCCESS != r)
        {
            ERR("Error registering hotplug_callback_attach %s", libusb_error_name(r));
        }

        r = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_NO_FLAGS, vid,
                                             pid, LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback_detach, NULL, NULL);
        if (LIBUSB_SUCCESS != r)
        {
            ERR("Error registering hotplug_callback_detach %s", libusb_error_name(r));
        }
    }
    return true;
}




QList<QLibUsbInfo> QLibUsb::enumDevices(int vid, int pid)
{
    QList<QLibUsbInfo> r;
    init_usb_env();
    libusb_device **devs;
    ssize_t cnt;
    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0)
    {
        ERR("get usb device list error %d", cnt);
        return r;
    }
    libusb_device *dev;
    int i = 0;
    while ((dev = devs[i++]) != NULL)
    {
        QLibUsbInfo info;
        if (get_usb_info(dev, info, vid, pid)){
            r.append(info);
        }
    }
    libusb_free_device_list(devs, 1);
    return r;
}


QLibUsb::QLibUsb(QObject* parent)
    :QObject(parent),m_monitor(this)
{
    m_context = NULL;
    m_devList.insert(this, true);
}

QLibUsb::~QLibUsb()
{
    m_devList.remove(this);
    close();
}

void QLibUsb::hotplugCallback(const QLibUsbInfo& info, bool isArrival)
{
    if(isArrival){
        emit onConnect(info);
    }else{
        emit onDisconnect(info);
    }
}

static int open_usb(const QLibUsbInfo &info, libusb_device_handle **handle, libusb_context* context= NULL)
{
    libusb_device **devs;
    ssize_t cnt;
    int r = LIBUSB_ERROR_NO_DEVICE;
    cnt = libusb_get_device_list(context, &devs);
    if (cnt < 0)
    {
        ERR("get usb device list error %d when open", cnt);
        return cnt;
    }
    libusb_device *dev;
    int i = 0;
    while ((dev = devs[i++]) != NULL)
    {
        QLibUsbInfo tinfo;
        if (get_usb_info(dev, tinfo))
        {
            if (tinfo == info)
            {
                r = libusb_open(dev, handle);
                if (r < 0)
                {
                    *handle = NULL;
                    ERR("Fail to usb device %s, %s", info.toString().toStdString().c_str(), libusb_error_name(r));
                }
                break;
            }
        }
    }
    libusb_free_device_list(devs, 1);
    return r;
}


QString QLibUsb::indexStringAscii(int index)
{
    if(!m_handle)return QString();
    char buf[512];
    int r = libusb_get_string_descriptor_ascii(
                m_handle, index, (unsigned char*)(buf),
                sizeof(buf));
    if (r < 0){
        m_lastError = r;
        return QString();
    }
    return QString(buf);
}

QByteArray QLibUsb::indexString(int index, uint16_t langId)
{
    if(!m_handle)return QByteArray();
    char buf[512];
    int r;
    if(langId == 0){
        r = libusb_get_string_descriptor(m_handle, 0, 0, (unsigned char*)buf, sizeof(buf));
        if(r < 0){
            m_lastError = r;
            return QByteArray();
        }
        if (r < 4){
            m_lastError = LIBUSB_ERROR_IO;
            return QByteArray();
        }
        langId = buf[2] | (buf[3] << 8);
    }
    r = libusb_get_string_descriptor(m_handle, index, langId, (unsigned char*)buf, sizeof(buf));
    if (r < 0){
        m_lastError = r;
        return QByteArray();
    }
    return QByteArray(buf, r);
}

void QLibUsb::close()
{
    foreach(QLibUsbEP* ep, m_readEps){
        if(ep){
            ep->cancel();
        }
    }
    foreach(const QLibUsbInerfaceInfo& i, m_interfaces){
        if(m_handle){
            int r = libusb_release_interface(m_handle, i.bInterfaceNumber);
        }
    }
    m_interfaces.clear();

    if(m_context){
        m_monitor.stopMonitor();
    }

    foreach(QLibUsbEP* ep, m_readEps){
        if(ep){
            delete ep;
        }
    }
    m_readEps.clear();

    if (m_handle){
        libusb_close(m_handle);
        m_handle = NULL;
    }

    if(m_context){
        libusb_exit(m_context);
        m_context = NULL;
    }
}

bool QLibUsb::open(const QLibUsbInfo& info, int bufSize)
{
    int r = 0;
    m_info = info;
    r = libusb_init(&m_context);
    if(r<0){
        ERR("Fail to init context %s", libusb_error_name(r));
        m_context = NULL;
        return false;
    }
    if ( (r = open_usb(m_info, &m_handle, m_context)) < 0){
        ERR("Fail to open device %s", info.toString().toStdString().c_str());
        m_handle = NULL;
        m_lastError = r;
        return false;
    }
    //QLibUsbMonitor::AddDevice();
    m_monitor.startMonitor(m_context);
    libusb_config_descriptor *cfg = NULL;
    libusb_device *dev = libusb_get_device(m_handle);
    r = libusb_get_active_config_descriptor(dev, &cfg);
    if (r < 0){
        ERR("Fail to get device config_descriptor %s", libusb_error_name(r));
        m_lastError = r;
        close();
        return false;
    }
    m_interfaces.clear();
    for (int j = 0; j < cfg->bNumInterfaces; j++){
        const struct libusb_interface_descriptor *interface_desc = (cfg->interface+j)->altsetting;
        m_interfaces.append(QLibUsbInerfaceInfo(interface_desc));
    }
    libusb_free_config_descriptor(cfg);

    //r = libusb_set_configuration(m_handle, 1);
    //if(r < 0){
//        DBG("Fail to libusb_set_configuration %s", libusb_error_name(r));
    //}
    m_readEps.clear();
    m_writeEps.clear();
    foreach(const QLibUsbInerfaceInfo& info, m_interfaces){
        // on some platform, detach kernel may fail, so we just output the information,
        // don't stop the process
        r = libusb_detach_kernel_driver(m_handle, info.bInterfaceNumber);
        if(r < 0){
            DBG("Fail to libusb_detach_kernel_driver %s", libusb_error_name(r));
        }
        r = libusb_claim_interface(m_handle, info.bInterfaceNumber);
        if(r < 0){
            ERR("Fail to libusb_claim_interface %s", libusb_error_name(r));
        }

        foreach(const QLibUsbEPInfo& ep, info.endpoints){
            if(ep.isIn()){
                QLibUsbEP* pep = new QLibUsbEP(*this, ep, bufSize);
                if(pep->init()){
                    m_readEps.append(pep);
                }else{
                    delete pep;
                }
            }else{
                m_writeEps.insert(ep.bEndpointAddress, ep);
            }
        }
    }
    return true;
}

int QLibUsb::write(int epAddr, const QByteArray& d, bool needZeroPacket, unsigned int timeout)
{
    QMap<int, QLibUsbEPInfo>::iterator it = m_writeEps.find(epAddr);
    int r = -1;
    if(it != m_writeEps.end() && m_handle){
        int actLen = 0;
        int len = d.size();
        int total_write = 0;
        const char* data = d.constData();
        int (LIBUSB_CALL *bulk_or_int_transfer)(struct libusb_device_handle *dev_handle,
            unsigned char endpoint, unsigned char *data, int length, int *transferred,
            unsigned int timeout) = 0;
        if(LIBUSB_TRANSFER_TYPE_INTERRUPT == it->bmAttributes){
            bulk_or_int_transfer= libusb_interrupt_transfer;
        }else if(LIBUSB_TRANSFER_TYPE_BULK == it->bmAttributes){
            bulk_or_int_transfer= libusb_bulk_transfer;
        }
        if(bulk_or_int_transfer){
            r = bulk_or_int_transfer(m_handle, it->bEndpointAddress, (uint8_t*)data, len, &actLen, timeout);
            if(r < 0){
                ERR("USB EP%d write error %s", epAddr, libusb_error_name(r));
                m_lastError = r;
                return r;
            }
            if( needZeroPacket && (len % it->wMaxPacketSize) == 0 ){
                r = bulk_or_int_transfer(m_handle, it->bEndpointAddress, (uint8_t*)data, 0, NULL, timeout);
                if(r<0){
                    ERR("USB EP%d write 0 byte error %s", epAddr, libusb_error_name(r));
                    m_lastError = r;
                    return r;
                }
            }
            return actLen;
        }
        ERR("Transfer type not support for write");
        r = LIBUSB_ERROR_NOT_SUPPORTED;
        m_lastError = r;
        return r;
    }
    r = LIBUSB_ERROR_NO_DEVICE;
    m_lastError = r;
    return r;
}

QList<QLibUsbEPInfo> QLibUsb::readEndpoints()const
{
    QList<QLibUsbEPInfo> r;
    foreach(QLibUsbEP* ep, m_readEps){
        r.append(ep->info());
    }
    return r;
}

QLibUsbEP::QLibUsbEP(QLibUsb& parent, const QLibUsbEPInfo& epInfo, int buffer_size)
    :m_parent(parent)
    ,m_info(epInfo)
    ,m_xfer(0)
    ,m_bufSize(buffer_size)
{
    m_buf = new uint8_t[m_bufSize];
}

QLibUsbEP::~QLibUsbEP(){
    cenceled();
}

void LIBUSB_CALL QLibUsbEP::completeCallback(libusb_transfer *xfer)
{
    QLibUsbEP *ep = reinterpret_cast<QLibUsbEP *>(xfer->user_data);
    if (xfer == ep->m_xfer)
    {
        switch(xfer->status)
        {
            case LIBUSB_TRANSFER_COMPLETED:
            emit ep->m_parent.epDataReady(ep->m_info.bEndpointAddress, QByteArray((char *)xfer->buffer, xfer->actual_length));
                DBG("LIBUSB_TRANSFER_COMPLETED");
                break;
            case LIBUSB_TRANSFER_CANCELLED:
                DBG("LIBUSB_TRANSFER_CANCELLED");
                return;
                break;
            case LIBUSB_TRANSFER_NO_DEVICE:
                DBG("LIBUSB_TRANSFER_NO_DEVICE");
                break;
            case LIBUSB_TRANSFER_TIMED_OUT:
                DBG("LIBUSB_TRANSFER_TIMED_OUT");
                break;
            case LIBUSB_TRANSFER_ERROR:
                DBG("LIBUSB_TRANSFER_ERROR");
                break;
            case LIBUSB_TRANSFER_STALL:
                DBG("LIBUSB_TRANSFER_STALL");
                break;
            case LIBUSB_TRANSFER_OVERFLOW:
                DBG("LIBUSB_TRANSFER_OVERFLOW");
                break;
        }
        int r = libusb_submit_transfer(ep->m_xfer);
        if( r < 0 ){
            ERR("fail to re-submit transfer %s", libusb_error_name(r));
        }
    }
}

void QLibUsbEP::cenceled()
{
    if(m_xfer){
        //libusb_cancel_transfer(m_xfer);
        libusb_free_transfer(m_xfer);
        m_xfer = 0;
    }
    if(m_buf){
        delete [] m_buf;
        m_buf = 0;
    }
}

void QLibUsbEP::cancel()
{
    if(m_xfer){
        libusb_cancel_transfer(m_xfer);
    }
}

bool QLibUsbEP::init()
{
    if(m_xfer){
        libusb_cancel_transfer(m_xfer);
        //libusb_free_transfer(m_xfer);
        m_xfer = 0;
    }
    if(m_info.isIn()){
        if(m_info.bmAttributes == LIBUSB_TRANSFER_TYPE_INTERRUPT){
            m_xfer = m_xfer?m_xfer:libusb_alloc_transfer(0);
            libusb_fill_interrupt_transfer(m_xfer,
                                m_parent.handle(),
                                m_info.bEndpointAddress, // Endpoint ID
                                m_buf,
                                m_info.wMaxPacketSize,
                                QLibUsbEP::completeCallback,
                                this,
                                0
                                );
        }else if(m_info.bmAttributes == LIBUSB_TRANSFER_TYPE_BULK){
            m_xfer = m_xfer?m_xfer:libusb_alloc_transfer(0);
            libusb_fill_bulk_transfer(m_xfer,
                                m_parent.handle(),
                                m_info.bEndpointAddress, // Endpoint ID
                                m_buf,
                                m_bufSize,
                                QLibUsbEP::completeCallback,
                                this,
                                0
                                );
        }else{
            ERR("Transfer type not support");
            return false;
        }
    }
    if(m_xfer){
        int r = 0;
        r = libusb_submit_transfer(m_xfer);
        if( r < 0 ){
            ERR("fail to re-submit transfer %s", libusb_error_name(r));
            libusb_free_transfer(m_xfer);
            m_xfer = 0;
            return false;
        }
    }
    return true;
}


