#include "indigoqueue.h"
#include <QtCore>

IndigoQueue::~IndigoQueue(void)
{
}

void IndigoQueue::push(char data)
{
    queue.enqueue(data);
}

void IndigoQueue::pushAll(QByteArray &array) 
{
	foreach (char a, array)
	{
		push(a);
	}
}


char IndigoQueue::pop()
{
    if(queue.size() <= 0)
    {
        // You should handle Queue underflow the way you want here.
		qDebug() << "queue underflow!!!";
        return -1;
    }

	return queue.dequeue();
}

char IndigoQueue::peek()
{
    if(queue.size() <= 0)
    {
        // You should handle Queue underflow the way you want here.
		qDebug() << "queue underflow!!!";
		return -1; // сообщение ни о чём
    }

    return queue.head();
}

bool IndigoQueue::isEmpty()
{
    return queue.size() == 0;
}

int IndigoQueue::size() 
{
	return queue.size();
}


// ----------------------
IndigoOrderQueue::~IndigoOrderQueue(void)
{
}

void IndigoOrderQueue::push(hello data)
{
    queue.enqueue(data);
}

hello IndigoOrderQueue::pop()
{
    if(queue.size() <= 0)
    {
        // You should handle Queue underflow the way you want here.
		qDebug() << "queue underflow!!!";
		return hello::default_instance();
    }

	return queue.dequeue();
}

hello IndigoOrderQueue::peek()
{
    if(queue.size() <= 0)
    {
        // You should handle Queue underflow the way you want here.
		qDebug() << "queue underflow!!!";
		return hello::default_instance(); // сообщение ни о чём
    }

    return queue.head();
}

bool IndigoOrderQueue::isEmpty()
{
    return queue.size() == 0;
}

int IndigoOrderQueue::size() 
{
	return queue.size();
}