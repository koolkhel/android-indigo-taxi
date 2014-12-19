#include <QDebug>

#include "filedownloader.h"
 
FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) :
    QObject(parent), reply(NULL)
{
    
 
    QNetworkRequest request(imageUrl);
	qDebug() << "requesting url" << imageUrl;
    reply = m_WebCtrl.get(request);

	connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)),
                SLOT(fileDownloaded(QNetworkReply*)));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(downloadError(QNetworkReply::NetworkError)));

	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), SIGNAL(downloadProgress(qint64,qint64)));
}
 
FileDownloader::~FileDownloader()
{
}

void FileDownloader::abort()
{
	qDebug() << "FileDownloader::abort";
	if (reply != NULL) {
		//disconnect(reply, 0, 0, 0);
		reply->abort();
		delete reply;
		reply = NULL;
	}
}

void FileDownloader::downloadError(QNetworkReply::NetworkError error)
{
	if (reply != NULL) {
		qDebug() << "download error:" << reply->errorString();
	}
}
 
void FileDownloader::fileDownloaded(QNetworkReply* pReply)
{
	if (pReply->error() == QNetworkReply::NoError) {
		qDebug() << "download successful";
		m_DownloadedData = pReply->readAll();
		
		emit downloaded();
	} else {
		qDebug() << "download error";
		emit fileDownloadError(pReply->errorString());
	}
	disconnect(reply, 0, this, 0);    
	//emit a signal
    pReply->deleteLater();
    
}
 
QByteArray FileDownloader::downloadedData() const
{
    return m_DownloadedData;
}