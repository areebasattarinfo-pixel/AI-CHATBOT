#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QDateTime>

class Message {
private:
    QString sender;
    QString content;
    QDateTime timestamp;
    QString messageId;

public:
    Message();
    Message(const QString& sender, const QString& content);

    QString getSender() const;
    QString getContent() const;
    QDateTime getTimestamp() const;
    QString getMessageId() const;

    void setSender(const QString& sender);
    void setContent(const QString& content);

    QString toFileString() const;
    static Message fromFileString(const QString& line);
};

#endif // MESSAGE_H
