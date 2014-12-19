#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H
 
#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
 
class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QUrl imageUrl, QObject *parent = 0);
 
    virtual ~FileDownloader();
 
    QByteArray downloadedData() const;

	void abort();
 
signals:
        void downloaded();
		void downloadProgress(qint64, qint64);
		void fileDownloadError(QString);
 
private slots:
 
    void fileDownloaded(QNetworkReply* pReply);

	void downloadError(QNetworkReply::NetworkError);
 
private:
 
    QNetworkAccessManager m_WebCtrl;
 
    QByteArray m_DownloadedData;

	QNetworkReply *reply;
 
};
 
#endif // FILEDOWNLOADER_H