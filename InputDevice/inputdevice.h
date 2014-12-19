#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <QObject>
#include "keyboard.h"
#include "numpad.h"
#include "customedit.h"
#include <QCoreApplication>
#include <QPlainTextEdit>
#include <QPlatformInputContext>

class InputDevice : public QInputContext
{
    Q_OBJECT
public:
    explicit InputDevice();

private:
    Keyboard *keyboard;
    Numpad *numpad;
    bool eventFilter(QObject *obj,  QEvent *event);
    QObject * tmpEditObj;

    QString getText();
    void setText(QString text);

    void updatePosition();

protected:
    QString identifierName();
    void reset();
    bool isComposing() const;
    QString language();

private slots:
    void dataSet(QVariant *);
};

#endif // INPUTDEVICE_H
