#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "common.h"
#include "qmlobjectlist.h"

#include <QObject>
#include <QDateTime>
#include <QAbstractListModel>
#include <QSet>
#include <QPointer>

class Buffer;
class BufferLine;
class LineModel;

#include <cstdint>


class Nick : public QObject {
    Q_OBJECT
    PROPERTY(char, visible)
    PROPERTY(char, group)
    PROPERTY(int, level)
    PROPERTY(QString, name)
    PROPERTY(QString, color)
    PROPERTY(QString, prefix)
    PROPERTY(QString, prefix_color)

    PROPERTY(pointer_t, ptr)
public:
    Nick(Buffer *parent = nullptr);

};

class Buffer : public QObject {
    Q_OBJECT
    PROPERTY(int, number)
    PROPERTY(QString, name)
    PROPERTY(QString, short_name)
    ALIAS(QString, name, full_name)
    PROPERTY(QString, title)
    PROPERTY(QStringList, local_variables)

    PROPERTY(int, unreadMessages)
    PROPERTY(int, hotMessages)

    Q_PROPERTY(QmlObjectList *lines READ lines CONSTANT)
    Q_PROPERTY(QmlObjectList *nicks READ nicks CONSTANT)
public:
    Buffer(QObject *parent, pointer_t pointer);
    //BufferLine *getLine(pointer_t ptr);
    void prependLine(BufferLine *line);
    void appendLine(BufferLine *line);

    bool isAfterInitialFetch();

    QmlObjectList *lines();
    QmlObjectList *nicks();
    Q_INVOKABLE Nick *getNick(pointer_t ptr);
    void addNick(pointer_t ptr, Nick* nick);
    void removeNick(pointer_t ptr);
    void clearNicks();
    Q_INVOKABLE QStringList getVisibleNicks();

public slots:
    void input(const QString &data);
    void fetchMoreLines();

private:
    QmlObjectList *m_lines { nullptr };
    QmlObjectList *m_nicks { nullptr };
    pointer_t m_ptr;
    bool m_afterInitialFetch { false };
    int m_lastRequestedCount { 0 };
};

class BufferLineSegment : public QObject {
    Q_OBJECT
public:
    enum Type {
        PLAIN,
        LINK,
        EMBED,
        IMAGE,
        VIDEO
    }; Q_ENUMS(Type)
    PROPERTY(Type, type)
    PROPERTY(QString, plainText)
    Q_PROPERTY(QString summary READ summaryGet NOTIFY summaryChanged)
    PROPERTY(QString, embedUrl)
public:
    BufferLineSegment(BufferLine *parent = nullptr, const QString &text = QString(), Type type = PLAIN);

signals:
    void summaryChanged();

private:
    QString summaryGet();
};

class BufferLine : public QObject {
    Q_OBJECT
    PROPERTY(QDateTime, date)
    PROPERTY(bool, displayed)
    PROPERTY(bool, highlight)
    PROPERTY(QStringList, tags_array)

    Q_PROPERTY(QString prefix READ prefixGet WRITE prefixSet NOTIFY prefixChanged)
    Q_PROPERTY(QString message READ messageGet WRITE messageSet NOTIFY messageChanged)

    Q_PROPERTY(bool isPrivMsg READ isPrivMsg NOTIFY tags_arrayChanged)
    Q_PROPERTY(QString colorlessNickname READ colorlessNicknameGet NOTIFY messageChanged)
    Q_PROPERTY(QString colorlessText READ colorlessTextGet NOTIFY messageChanged) // used here because segments is already chopped up
    Q_PROPERTY(QObject *buffer READ bufferGet CONSTANT)
    Q_PROPERTY(QList<QObject*> segments READ segments NOTIFY segmentsChanged)
public:
    BufferLine(Buffer *parent);

    void setParent(Buffer *parent);

    QString prefixGet() const;
    void prefixSet(const QString &o);
    QString messageGet() const;
    void messageSet(const QString &o);

    bool isPrivMsg();
    QString colorlessNicknameGet();
    QString colorlessTextGet();

    QObject *bufferGet();

    QList<QObject*> segments();

signals:
    void segmentsChanged();
    void messageChanged();
    void prefixChanged();

private slots:
    void onMessageChanged();

private:
    QString m_message;
    QString m_prefix;
    QList<QObject*> m_segments;
};

class HotListItem : public QObject {
    Q_OBJECT
    PROPERTY(QList<int>, count)
    Q_PROPERTY(Buffer* buffer READ bufferGet WRITE bufferSet NOTIFY bufferChanged)
public:
    HotListItem(QObject *parent = nullptr);

    Buffer *bufferGet();
    void bufferSet(Buffer *o);

signals:
    void bufferChanged();

private slots:
    void onCountChanged();

private:
    QPointer<Buffer> m_buffer;
};
#endif // DATAMODEL_H
