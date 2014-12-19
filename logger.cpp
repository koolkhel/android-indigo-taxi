#include "logger.h"
#include <QScrollBar>

Logger::Logger(QTextEdit *_logText, QObject *parent)
	: QObject(parent),
	logText(_logText)
{

}

Logger::~Logger()
{

}

void Logger::addLine(QString line)
{
	logText->setPlainText(logText->property("plainText").toString() + line);
	QScrollBar *v = logText->verticalScrollBar();
	v->setValue(v->maximum());
}
