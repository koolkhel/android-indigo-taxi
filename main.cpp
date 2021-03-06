#include "indigotaxi.h"
#include <QApplication>

#include <QDesktopWidget>
#include <QScreen>

#include <QtCore>
#include <QtGui>
#include <QtGlobal>
#include <QDebug>

#ifdef UNDER_ANDROID
#include <QAndroidJniObject>
#endif

#include "hello.pb.h"

// #include "InputDevice/inputdevice.h"

#include "backend.h"
#include "logger.h"

#ifdef UNDER_CE
#include <winbase.h>
#endif

IndigoTaxi *mainWindow = NULL;

void myMessageOutput(QtMsgType type, const QMessageLogContext& Context, const QString &Message)
 {
	 //in this function, you can write the message to any stream!
	 switch (type) {
	 case QtDebugMsg:
		 //fprintf(stderr, "Debug: %s\n", msg);
         if (Message.contains('\n')) {
            fprintf(stderr, "%s", Message.toStdString().c_str());
			
		 } else {
             fprintf(stderr, "%s\n", Message.toStdString().c_str());
		 }
		 if (mainWindow != NULL) {
			//mainWindow->log(msg);
		 }

         break;
     case QtWarningMsg:
         fprintf(stderr, "Warning: %s\n", Message.toStdString().c_str());
         break;
     case QtCriticalMsg:
         fprintf(stderr, "Critical: %s\n", Message.toStdString().c_str());
         break;
     case QtFatalMsg:
         fprintf(stderr, "Fatal: %s\n", Message.toStdString().c_str());
//         abort();
     }
 }

Logger *logger = NULL;

void LoadFont(QString name)
{
	QByteArray segoe;
	QResource r(name);
	if (r.isValid()) {
		QByteArray b( reinterpret_cast< const char* >( r.data() ), r.size() );
		qDebug() << "font" << name;
		QFontDatabase::addApplicationFontFromData(b);
	}
}

int main(int argc, char *argv[])
{	
	int retcode = 0;
	bool result = 
#ifdef UNDER_CE
	QResource::registerResource("\\ResidentFlash\\IndigoTaxi\\sound.rcc");	
    QResource::registerResource("\\ResidentFlash\\IndigoTaxi\\UI.rcc");
#else
	QResource::registerResource("C:\\sound.rcc");
	QResource::registerResource("C:\\UI.rcc");
#endif
	qDebug() << "sounds loaded:" << result;
	qDebug() << "font status:" << QResource(":/IndigoTaxi/segoeui.ttf").isValid();
	qDebug() << "font bold status:" << QResource(":/IndigoTaxi/segoeuib.ttf").isValid();
	
//#ifndef UNDER_ANDROID
//    qInstallMessageHandler(myMessageOutput);
//#endif

	QApplication a(argc, argv);
	a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	QStringList argList = qApp->arguments();
	foreach (QString arg, argList) {
#ifdef UNDER_CE
		bool isNumber = false;
		int ceLaunchId = arg.toInt(&isNumber);
		if (isNumber) {
			SignalStarted(ceLaunchId);
		}
#endif
		qDebug() << "arg:" << arg;
	}
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("CP1251"));

		
	LoadFont(":/IndigoTaxi/segoeui.ttf");
	LoadFont(":/IndigoTaxi/segoeuib.ttf");

	// for virtual keyboard
    //InputDevice inputdevice;
	
	/*
	Debug: "Windows" 
Debug: "WindowsCE" 
Debug: "WindowsMobile" 
*/
#ifdef UNDER_CE
		QApplication::setStyle("WindowsCE");
#else
		QApplication::setStyle("Windows");
#endif
        qApp->setAutoSipEnabled(true);
	
		IndigoTaxi w;
		mainWindow = &w;

        QScreen *screen = QApplication::screens().at(0);

        int availableHeight = screen->size().height();

        int availableWidth = screen->size().width();

        // DEXP
        if (availableWidth == 854) {
            w.showMaximized();
        // SAMSUNG
        } else if (availableWidth == 800) {
            // FIXME everything
            int statusBarHeight = 38;
            w.setGeometry(0, statusBarHeight, availableWidth, availableHeight - statusBarHeight);
            w.showNormal();
        }

		qDebug() << "screen height:" << QApplication::desktop()->heightMM() << "width:" << QApplication::desktop()->widthMM();
		qDebug() << "physical screen dpi height:" << w.physicalDpiY() << "width:" << w.physicalDpiX();
		//qDebug() << "logical screen dpi height:" << w.logicalDpiY() << "width:" << w.logicalDpiX();

#ifdef UNDER_ANDROID
        // пока для событий подключения наушников
        QAndroidJniObject::callStaticMethod<void>("ru/indigosystem/taxi/android/NativeFunctions", "registerBroadcastReceiver", "()V");
#endif

		retcode = a.exec();	

		return retcode;

	
#if 0
	QList<QSerialPortInfo> serialPortInfoList = QSerialPortInfo::availablePorts();
	QStringList results;
	const QString blankString = QObject::tr("N/A");
	QString description;
    QString manufacturer;
    QString serialNumber;
	qDebug() << "Hello, world!";

	foreach (const QSerialPortInfo &serialPortInfo, serialPortInfoList) {
        description = serialPortInfo.description();
        manufacturer = serialPortInfo.manufacturer();
        serialNumber = serialPortInfo.serialNumber();
		
        qDebug() << endl
            << QObject::tr("Port: ") << serialPortInfo.portName() << endl
            << QObject::tr("Location: ") << serialPortInfo.systemLocation() << endl
            << QObject::tr("Description: ") << (!description.isEmpty() ? description : blankString) << endl
            << QObject::tr("Manufacturer: ") << (!manufacturer.isEmpty() ? manufacturer : blankString) << endl
            << QObject::tr("Serial number: ") << (!serialNumber.isEmpty() ? serialNumber : blankString) << endl
            << QObject::tr("Vendor Identifier: ") << (serialPortInfo.hasVendorIdentifier() ? QByteArray::number(serialPortInfo.vendorIdentifier(), 16) : blankString) << endl
            << QObject::tr("Product Identifier: ") << (serialPortInfo.hasProductIdentifier() ? QByteArray::number(serialPortInfo.productIdentifier(), 16) : blankString) << endl
            << QObject::tr("Busy: ") << (serialPortInfo.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) << endl;

	
	}
#endif
	
	
}
