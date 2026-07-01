#include "message.h"
#include <QUuid>

Message::Message() {
    messageId = QUuid::createUuid().toString();
    timestamp = QDateTime::currentDateTime();
}

Message::Message(const QString& sender, const QString& content) {
    this->sender = sender;
    this->content = content;
    this->timestamp = QDateTime::currentDateTime();
    this->messageId = QUuid::createUuid().toString();
}

QString Message::getSender() const { return sender; }
QString Message::getContent() const { return content; }
QDateTime Message::getTimestamp() const { return timestamp; }
QString Message::getMessageId() const { return messageId; }

void Message::setSender(const QString& s) { sender = s; }
void Message::setContent(const QString& c) { content = c; }

QString Message::toFileString() const {
    // Format: messageId|sender|timestamp|content
    QString safeContent = content;
    safeContent.replace("\n", "<<NEWLINE>>");
    safeContent.replace("|", "<<PIPE>>");
    return messageId + "|" + sender + "|" +
           timestamp.toString(Qt::ISODate) + "|" + safeContent;
}

Message Message::fromFileString(const QString& line) {
    Message msg;
    QStringList parts = line.split("|");
    if (parts.size() >= 4) {
        msg.messageId = parts[0];
        msg.sender    = parts[1];
        msg.timestamp = QDateTime::fromString(parts[2], Qt::ISODate);
        QString content = parts[3];
        content.replace("<<NEWLINE>>", "\n");
        content.replace("<<PIPE>>", "|");
        msg.content = content;
    }
    return msg;
}