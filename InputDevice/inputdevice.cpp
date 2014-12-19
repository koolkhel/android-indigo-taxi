#include "inputdevice.h"
#include <QVariant>
#include <QValidator>
#include <QApplication>
#include <QDesktopWidget>

InputDevice::InputDevice()
{
    keyboard = new Keyboard;
    numpad = new Numpad;
    keyboard->setObjectName("keyboard");
    numpad->setObjectName("numpad");
    connect(keyboard,SIGNAL(dataSet(QVariant*)),this,SLOT(dataSet(QVariant*)));
    connect(numpad,SIGNAL(dataSet(QVariant*)),this,SLOT(dataSet(QVariant*)));
    qApp->installEventFilter(this);
}

bool InputDevice::eventFilter(QObject *obj,  QEvent *event)
{
    QString tmpClassName;
	// грязный хак
	if ((obj->objectName() == "driverNameLineEdit" || (obj->objectName() == "passwordEdit") || (obj->objectName() == "driverNumberLineEdit"))
		&& event->type() == QEvent::FocusIn) {
		QEvent eventTo(QEvent::RequestSoftwareInputPanel);
		QApplication::sendEvent(obj, &eventTo);
	}
    
	if (event->type() == QEvent::RequestSoftwareInputPanel)
    {
        if (obj != NULL)
        {
            if (obj->property("enabled").toBool())
            {
                if (obj->property("keyboard").toBool())
                {
                    if (dynamic_cast<QLineEdit *>(obj))
                    {
                        tmpEditObj=obj;
                        const QValidator * tmpValidator = dynamic_cast<QLineEdit *>(tmpEditObj)->validator();
                        if (qobject_cast<const QDoubleValidator *>(tmpValidator) || qobject_cast<const QIntValidator *>(tmpValidator))
                        {
                            updatePosition();
                            numpad->activate(getText(),static_cast<QLineEdit *>(tmpEditObj)->validator());
                            numpad->setEchoMode(dynamic_cast<QLineEdit *>(tmpEditObj)->echoMode());
                        }
                        else
                        {
                            updatePosition();
                            keyboard->activate(getText(),obj->property("maxLength").toInt(),false);
                            keyboard->setEchoMode(dynamic_cast<QLineEdit *>(tmpEditObj)->echoMode());
                            keyboard->setValidator(dynamic_cast<QLineEdit *>(tmpEditObj)->validator());
                        }
                    }
                }
            }
        }
        return true;
    }
    else if (event->type() == QEvent::CloseSoftwareInputPanel)
    {
            numpad->close();
            keyboard->close();
            return true;
    }
    return false;
}

QString InputDevice::getText()
{

    if (dynamic_cast<QLineEdit *>(tmpEditObj))
    {
        return dynamic_cast<QLineEdit *>(tmpEditObj)->text();
    }
    else
    {
        return "";
    }
}

void InputDevice::setText(QString text)
{
    if (dynamic_cast<QLineEdit *>(tmpEditObj))
    {
        dynamic_cast<QLineEdit *>(tmpEditObj)->setText(text);
        dynamic_cast<QLineEdit *>(tmpEditObj)->clearFocus();
    }
}

void InputDevice::dataSet(QVariant * data)
{
    setText(data->toString());
}


void InputDevice::updatePosition()
{
   /* QWidget *widget = focusWidget();
    if (!widget)
        return;

    QRect widgetRect = widget->rect();*/


    QRect screenGeo = QApplication::desktop()->screenGeometry();
    QRect keyGeo = keyboard->geometry();
    QRect numGeo = numpad->geometry();

    /*QPoint panelPos = QPoint(widgetRect.left(), widgetRect.bottom() + 2);
    panelPos = widget->mapToGlobal(panelPos);*/
	
	numpad->setMinimumSize((int) screenGeo.width(), (int) screenGeo.height() * 0.8);
	numpad->setMaximumSize((int) (screenGeo.width()), (int) screenGeo.height() * 0.9);

    keyboard->move(QPoint((screenGeo.width()/2)-(keyGeo.width()/2),(screenGeo.height()/2)-(keyGeo.height()/2)));
    numpad->move(QPoint(0, 0));


    //numpad->move(panelPos);
    //numpad->move(panelPos);
}


QString InputDevice::identifierName()
{
    return "InputDevice";
}

void InputDevice::reset()
{
}

bool InputDevice::isComposing() const
{
    return false;
}

QString InputDevice::language()
{
    return "ru_RU";
}
