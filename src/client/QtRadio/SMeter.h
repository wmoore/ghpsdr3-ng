#ifndef SMETER_H
#define SMETER_H

#include <QtCore>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QFrame>
#else
#include <QFrame>
#endif

#include <QPainter>
#include "Meter.h"


// class Spectrum: public QFrame {
//    Q_OBJECT
class SMeter: public QFrame {
    Q_OBJECT

public:
    SMeter();
    SMeter(QWidget* parent=0);
    virtual ~SMeter();
    int meter_dbm;
    int sub_meter_dbm;
    void setSubRxState(bool state);

protected:
    void paintEvent(QPaintEvent*);

signals:

public slots:

private:
    Meter* SMeterMain;
    Meter* SMeterSub;
    bool subRx;
};

#endif // SMETER_H
