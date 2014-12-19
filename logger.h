#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QTextEdit>

class Logger : public QObject
{
	Q_OBJECT

public:
	explicit Logger(QTextEdit *_logText, QObject *parent = 0);
	~Logger();

	void addLine(QString line);
private:
	QTextEdit *logText;
};

extern Logger *logger;

#endif // LOGGER_H
