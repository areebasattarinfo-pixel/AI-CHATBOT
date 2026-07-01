#ifndef USER_H
#define USER_H

#include <QString>
#include <QVector>
#include "message.h"

class User {
private:
    QString username;
    QString passwordHash;
    QString email;
    QVector<QString> chatSessionIds;

public:
    User();
    User(const QString& username, const QString& email, const QString& password);

    QString getUsername() const;
    QString getEmail() const;
    QString getPasswordHash() const;
    QVector<QString> getChatSessionIds() const;

    void setUsername(const QString& username);
    void setEmail(const QString& email);

    void addSessionId(const QString& id);
    bool validatePassword(const QString& password) const;
    QString hashPassword(const QString& password) const;

    QString toFileString() const;
    static User fromFileString(const QString& line);
};

#endif // USER_H
