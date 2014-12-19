#pragma once

#include <QtCore/QQueue>
#include <QtCore/QByteArray>

#include "hello.pb.h"

/**
	* Класс нужен для того, чтобы относительно дёшево и удобно держать очередь с необработанными
	* данными TcpSocketа/QByteArray. Заточен на char
	*
	*
	*/

#define MAXIMUM_QUEUE_SIZE 200

class IndigoOrderQueue
{
private:
	QQueue<hello> queue;

public:
	IndigoOrderQueue(void)
	{}
	~IndigoOrderQueue(void);

	void push(hello data);
    hello pop();
    hello peek();
	int size();
    bool isEmpty();
};

class IndigoQueue
{
private:
	QQueue<char> queue;

public:
	IndigoQueue(void)
	{}
	~IndigoQueue(void);

	void push(char data);
	void pushAll(QByteArray &data);
    char pop();
    char peek();
	int size();
    bool isEmpty();
};