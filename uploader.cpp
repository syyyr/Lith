#include "uploader.h"

#include <QUrl>
#include <QNetworkAccessManager>
#include <QHttpPart>
#include <QFile>
#include <QJsonDocument>
#include <QBuffer>
#include <QFileInfo>


Uploader::Uploader(QObject *parent) : QObject(parent)
{

}

void Uploader::upload(const QString &path) {
    QUrl url = QUrl("https://api.imgur.com/3/image");

    QNetworkAccessManager * mgr = new QNetworkAccessManager(this);

    QFile *file;
    if (path.startsWith("file://"))
        file = new QFile(QUrl(path).toLocalFile());
    else if (path.startsWith("file:assets-library")) {
        file = new QFile(QUrl(path).toLocalFile());
    }
    else
        file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        emit error("Could not open file " + path);
        return;
    }

    workingSet(true);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/application/x-www-form-urlencoded");
    request.setRawHeader("Authorization", "Client-ID a416e89635996b4");

    auto reply = mgr->post(request, file->readAll());
    file->setParent(reply);

    connect(mgr, &QNetworkAccessManager::finished, this, &Uploader::onFinished);
}

void Uploader::uploadBinary(QImage data) {
    QUrl url = QUrl("https://api.imgur.com/3/image");

    QNetworkAccessManager * mgr = new QNetworkAccessManager(this);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/application/x-www-form-urlencoded");
    request.setRawHeader("Authorization", "Client-ID a416e89635996b4");

    QByteArray arr;
    QBuffer buffer(&arr);
    buffer.open(QIODevice::WriteOnly);
    data.save(&buffer, "png");
    buffer.close();
    qCritical() << "BUFFER " << arr.size();
    qCritical() << "IMAGE " << data.size();
    auto reply = mgr->post(request, arr);

    connect(mgr, &QNetworkAccessManager::finished, this, &Uploader::onFinished);
}

void Uploader::onFinished(QNetworkReply *reply) {
    workingSet(false);
    if (reply->isReadable()) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        qCritical() << QString(doc.toJson());
        if (doc["success"].toBool()) {
            emit success(doc["data"]["link"].toString());
        }
        else {
            emit error(doc["data"]["error"].toString());
        }
    }
    else {
        emit error(reply->errorString());
    }
    reply->deleteLater();
}
