// ***************************************************************************
// Annotate a reverse-compiled device tree
// ---------------------------------------------------------------------------
// annotate.cpp
// the actual annotation process
// ---------------------------------------------------------------------------
// Copyright (C) 2021 by t2ft - Thomas Thanner
// Waldstraße 15, 86399 Bobingen, Germany
// ---------------------------------------------------------------------------
// 2021-6-8  tt  Initial version created
// ***************************************************************************
#include "annotate.h"
#include <QFile>
#include <QMessageLogger>
#include <QDebug>
#include <stdarg.h>


Annotate::Annotate(bool beQuiet)
    : m_beQuiet(beQuiet)
    , m_lastError(noError)
{
    m_singleHandleParams << "arasan,soc-ctl-syscon";
    m_singleHandleParams << "audio-supply";
    m_singleHandleParams << "assigned-clock-parents";
    m_singleHandleParams << "backlight";
    m_singleHandleParams << "bt656-supply";
    m_singleHandleParams << "charge-dev";
    m_singleHandleParams << "connect";
    m_singleHandleParams << "ddr_timing";
    m_singleHandleParams << "devfreq-events";
    m_singleHandleParams << "extcon";
    m_singleHandleParams << "gpio1830-supply";
    m_singleHandleParams << "interrupt-parent";
    m_singleHandleParams << "iommus";
    m_singleHandleParams << "logo-memory-region";
    m_singleHandleParams << "memory-region";
    m_singleHandleParams << "mmc-pwrseq";
    m_singleHandleParams << "native-mode";
    m_singleHandleParams << "operating-points-v2";
    m_singleHandleParams << "phy-supply";
    m_singleHandleParams << "pmu1830-supply";
    m_singleHandleParams << "rockchip,pmu";
    m_singleHandleParams << "sdmmc-supply";
    m_singleHandleParams << "remote-endpoint";
    m_singleHandleParams << "rockchip,cpu";
    m_singleHandleParams << "rockchip,grf";
    m_singleHandleParams << "rockchip-serial-irq";
    m_singleHandleParams << "secure-memory-region";
    m_singleHandleParams << "simple-audio-card,mclk-fs";
    m_singleHandleParams << "sound-dai";
    m_singleHandleParams << "trip";
    m_singleHandleParams << "vbus-supply";
    m_singleHandleParams << "vcc1-supply";
    m_singleHandleParams << "vcc10-supply";
    m_singleHandleParams << "vcc11-supply";
    m_singleHandleParams << "vcc12-supply";
    m_singleHandleParams << "vcc2-supply";
    m_singleHandleParams << "vcc3-supply";
    m_singleHandleParams << "vcc4-supply";
    m_singleHandleParams << "vcc5-supply";
    m_singleHandleParams << "vcc6-supply";
    m_singleHandleParams << "vcc7-supply";
    m_singleHandleParams << "vcc8-supply";
    m_singleHandleParams << "vcc9-supply";
    m_singleHandleParams << "vddio-supply";
    m_singleHandleParams << "vin-supply";
    m_singleHandleParams << "vmmc-supply";
    m_singleHandleParams << "vqmmc-supply";
    m_singleHandleParams << "vref-supply";

    m_firstHandleParams << "discharge-gpios";
    m_firstHandleParams << "ep-gpios";
    m_firstHandleParams << "gpio";
    m_firstHandleParams << "gpios";
    m_firstHandleParams << "headset_gpio";
    m_firstHandleParams << "hp_ctrl_gpio";
    m_firstHandleParams << "int-n-gpios";
    m_firstHandleParams << "io-channels";
    m_firstHandleParams << "linein_det_gpio";
    m_firstHandleParams << "power-domains";
    m_firstHandleParams << "pwms";
    m_firstHandleParams << "reset-gpios";
    m_firstHandleParams << "rockchip,gpios";
    m_firstHandleParams << "thermal-sensors";
    m_firstHandleParams << "typec0-enable-gpios";
    m_firstHandleParams << "vbus-5v-gpios";
    m_firstHandleParams << "vsel-gpios";

    m_listHandleParams << "nvmem-cells";
    m_listHandleParams << "phys";
    m_listHandleParams << "pinctrl-0";
    m_listHandleParams << "pinctrl-1";
    m_listHandleParams << "pinctrl-2";
    m_listHandleParams << "pinctrl-3";
    m_listHandleParams << "pinctrl-4";
    m_listHandleParams << "pinctrl-5";
    m_listHandleParams << "pm_qos";
    m_listHandleParams << "ports";
    m_listHandleParams << "rockchip,codec";
}

bool Annotate::process(const QString &fnIn, const QString &fnOut)
{
    QFile fIn(fnIn);
    if (fIn.exists()) {
        if (fIn.open(QFile::ReadOnly)) {
            QByteArray src = fIn.readAll();
            if (src.isEmpty()) {
                m_lastError = InputFileReadError;
            } else {
                // input file read successfully
                log("read %u bytes from \"%s\"", src.size(), qPrintable(fnIn));
                ByteArrayList tsIn = src.split('\n');
                // create symbol table
                StringHash symbols = createSymbolHash(tsIn);
                // create phandle map
                StringHash handles = createHandleHash(tsIn);
                // replace numeric handles by symbols and write output file
                return writeOutput(fnOut, tsIn, symbols, handles);
            }
        } else {
            m_lastError = InputFileOpenError;
        }
    } else {
        m_lastError = InputFileNotFound;
    }
    return false;
}

QString Annotate::errString(Annotate::ErrCodes err)
{
    switch (err) {
    case noError: return QObject::tr("No Error");
    case InputFileNotFound: return QObject::tr("Input file not found");
    case InputFileOpenError: return QObject::tr("Cannot open input file for reading");
    case InputFileReadError: return QObject::tr("Error while reading input file");
    case OutputFileCreationError: return QObject::tr("Cannot create output file");
    case OutputFileWriteError: return QObject::tr("Error while writing to output file");
    }
    return QObject::tr("Unknown error code <%1>").arg(static_cast<int>(err));
}

QString Annotate::errString()
{
    return errString(m_lastError);
}

void Annotate::log(const char *fmt, ...)
{
    if (!m_beQuiet) {
        va_list vl;
        va_start(vl, fmt);
        qInfo().noquote() << QString().vasprintf(fmt, vl);
        va_end(vl);
    }
}

Annotate::StringHash Annotate::createSymbolHash(QByteArrayList &ts)
{
    StringHash sym;
    int n = ts.size();
    // find start of symbols area
    int inx = 0;
    for (inx=0; inx < n; ++inx) {
        QString l = ts[inx];
        if (l.contains("__symbols__ {"))
            break;
    }
    while (++inx < n) {
        QString l = ts[inx].trimmed();
        if (!l.isEmpty()) {
            if (l.contains("};"))
                break;
            QStringList sl = l.split("=");
            QString value = sl[0].trimmed();
            QString key = sl[1].mid(2, sl[1].length()-4);
            sym.insert(key, value);
        }
    }
    return sym;
}

Annotate::StringHash Annotate::createHandleHash(QByteArrayList &ts)
{
    StringHash h;
    QString path;
    int n = ts.size();
    for (int inx=0; inx < n; ++inx)  {
        QString l = ts[inx].trimmed();
        if (!adjustPath(&path, l)) {
            // no path adjustments, maybe we find a handle?
            if (l.contains("phandle")) {
                QString handle = l.split("<")[1].trimmed().chopped(2);
                h.insert(handle, path);
            }
        }
    }
    return h;
}

bool Annotate::adjustPath(QString *path, const QString &line)
{
    bool ret = false;

    int i = line.indexOf('{');
    if (i > 0) {
        // start of a new node with name
        QString nn = line.left(i).trimmed();
        if (nn=="/") {
            *path = "/";
        } else {
            if (path->right(1)!="/") {
                *path += '/' ;
            }
            *path += nn;
        }
        ret = true;
    } else {
        i = line.indexOf('}');
        if (i>=0) {
            // end of node detected
            QStringList sl = path->split("/");
            sl.removeLast();
            *path = sl.join('/');
            if (path->isEmpty())
                *path = "/";
            ret = true;
        }
    }
    return ret;
}

const QString Annotate::getParameters(const QString &line)
{
    QString x = line.split("=")[1].trimmed();
    return x.mid(1, x.length()-3);
}

const QString Annotate::handleToSymbol(const QString &h, const Annotate::StringHash &handles, const Annotate::StringHash &symbols)
{
    QString hpath = handles.value(h);
    if (hpath.isEmpty()) {
        // handle not found
        return h;
    }
    QString sym = symbols.value(hpath);
    if (sym.isEmpty()) {
        // symbol not found
        return h;
    }
    return "&"+sym;
}

const QString Annotate::rkGPIO(const QString &x)
{
    uint n = x.toUInt(nullptr, 0);
    return QString("RK_P%1%2").arg(static_cast<char>(n/8+'A')).arg(n%8);
}

const QString Annotate::gpioType(const QString &x)
{
    uint n = x.toUInt(nullptr, 0);
    switch(n) {
    case 0: return "GPIO_ACTIVE_HIGH";
    case 1: return "GPIO_ACTIVE_LOW";
    case 2: return "GPIO_OPEN_SOURCE";
    case 3: return "GPIO_OPEN_DRAIN";
    }
    return(x);
}

bool Annotate::writeOutput(const QString &fnOut, ByteArrayList &tsIn, const StringHash &symbols, const StringHash &handles)
{
    QFile f(fnOut);
    if (!f.open(QFile::Truncate | QIODevice::WriteOnly)) {
        m_lastError = OutputFileCreationError;
        return false;
    }
    QTextStream tsOut(&f);
    log("writing to \"%s\"\n", qPrintable(fnOut));
    QString path;
    int n = tsIn.size();
    for (int inx=0; inx < n; ++inx)  {
        QString l = tsIn[inx];
        if (l.contains("phandle = <0x"))
                continue;   // skip phandle lines
        if (!adjustPath(&path, l)) {
            // no path adjustments, maybe we can adjust handles or values
            QStringList sl = l.split("=");
            QString name = sl[0].trimmed();
            if (m_singleHandleParams.contains(name)) {
                // only a simple phandle exchange is required
                QString h = getParameters(l);
                l.replace(h, handleToSymbol(h, handles, symbols));
            } else if (m_firstHandleParams.contains(name)) {
                // only very first parameter is a phandle
                QStringList h = getParameters(l).split(" ");
                l = leftOfParameters(l) + "<" + handleToSymbol(h[0], handles, symbols);
                h.removeFirst();
                l += " " + h.join(" ") + ">;";
            } else if (m_listHandleParams.contains(name)) {
                // all parameters are phandles
                QStringList h = getParameters(l).split(" ");
                l = leftOfParameters(l);
                for (int i=0; i<h.size(); ++i) {
                    l += "<" + handleToSymbol(h[i], handles, symbols) + ">" + separator(i==h.size()-1);
                }
            } else {
                // special handling
                if ((name == "clocks") || (name=="dmas") || (name=="assigned-clocks")) {
                    QStringList h = getParameters(l).split(" ");
                    l = leftOfParameters(l);
                    int n = h.size();
                    if (n==1) {
                        l += "<" + handleToSymbol(h[0], handles, symbols) + ">;";
                    } else {
                        for (int i=0; i<n; i+=2) {
                            l += "<" + handleToSymbol(h[i], handles, symbols) + " " + h[i+1] + ">" + separator(i==n-2);
                        }
                    }
                } else if (name == "rockchip,pins") {
                    QStringList h = getParameters(l).split(" ");
                    l = leftOfParameters(l) + QString("<RK_GPIO%1 ").arg(h[0].toInt(nullptr, 0));
                    l += rkGPIO(h[1]);
                    uint n = h[2].toUInt(nullptr, 0);
                    if (n==0) {
                        l += " RK_FUNC_GPIO";
                    } else {
                        l += QString(" RK_FUNC_ALT%1").arg(n);
                    }
                    l += " " + handleToSymbol(h[3], handles, symbols) + ">;";
                } else if (name == "rockchip,power-ctrl") {
                    QStringList h = getParameters(l).split(" ");
                    l = leftOfParameters(l);
                    int n = h.size();
                    for (int i=0; i<n; i+=3) {
                        l += "<" + handleToSymbol(h[i], handles, symbols) + " " + rkGPIO(h[i+1]) + " " + gpioType(h[i+2]) + ">" + separator(i==n-3);
                    }
                } else if (name == "interrupts") {
                    QStringList h = getParameters(l).split(" ");
                    int n = h.size();
                    l = leftOfParameters(l);
                    if (n%5==0) {
                        for (int i=0; i<n; i+=5) {
                            l += "<" + handleToSymbol(h[i], handles, symbols) + " " + h[i+1] + " " + h[i+2] + " " + h[i+3] + " " + h[i+4] + ">" + separator(i==n-5);
                        }
                    } else if (n%4==0) {
                        for (int i=0; i<n; i+=4) {
                            l += "<" + handleToSymbol(h[i], handles, symbols) + " " + h[i+1] + " " + h[i+2] + " " + h[i+3] + ">" + separator(i==n-4);
                        }
                    } else {
                        for (int i=0; i<n; i+=2) {
                            l += "<" + rkGPIO(h[i]) + " " + h[i+1] + ">" + separator(i==n-2);
                        }
                    }
                }
            }
        }
        tsOut << l << "\n";
    }
    return true;
}

