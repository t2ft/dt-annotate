// ***************************************************************************
// Annotate a reverse-compiled device tree
// ---------------------------------------------------------------------------
// annotate.h
// header file for annotate.cpp
// ---------------------------------------------------------------------------
// Copyright (C) 2021 by t2ft - Thomas Thanner
// Waldstraße 15, 86399 Bobingen, Germany
// ---------------------------------------------------------------------------
// 2021-6-8  tt  Initial version created
// ***************************************************************************
#ifndef ANNOTATE_H
#define ANNOTATE_H

#include <QString>
#include <QTextStream>
#include <QHash>
#include <QSet>

class Annotate
{
public:
    Annotate(bool beQiet = false);

    typedef enum {
        noError = 0,
        InputFileNotFound,
        InputFileOpenError,
        InputFileReadError,
        OutputFileCreationError,
        OutputFileWriteError
    } ErrCodes;

    static QString errString(ErrCodes err);

    bool process(const QString &fnIn, const QString &fnOut);
    QString errString();


private:
    typedef QHash<QString, QString> StringHash;
    typedef  QList<QByteArray> ByteArrayList;

    bool            m_beQuiet;
    ErrCodes        m_lastError;
    QSet<QString>   m_singleHandleParams;
    QSet<QString>   m_firstHandleParams;
    QSet<QString>   m_listHandleParams;

    void log(const char *fmt, ...);
    StringHash createHandleHash(ByteArrayList &ts);
    StringHash createSymbolHash(ByteArrayList &ts);
    bool writeOutput(const QString &fnOut, ByteArrayList &tsIn, const StringHash &symbols, const StringHash &handles);
    bool adjustPath(QString *path, const QString &line);
    const QString getParameters(const QString &line);
    const QString handleToSymbol(const QString &h, const StringHash &handles, const StringHash &symbols);
    const QString leftOfParameters(const QString l) { return l.split("=")[0] + "= "; }
    const QString separator(bool last) { return (last ? ";" : ", "); }
    const QString rkGPIO(const QString &x);
    const QString gpioType(const QString &x);
};

#endif // ANNOTATE_H
