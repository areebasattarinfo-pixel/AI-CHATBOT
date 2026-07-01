#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include "message.h"
#include "user.h"

class FileHandler {
private:
    QString usersFilePath;
    QString chatsFilePath;
    QString logsFilePath;

    bool ensureFileExists(const QString& path);

public:
    FileHandler();

    bool saveUser(const User& user);
    User loadUser(const QString& username);
    bool updateUser(const User& user);
    bool deleteUser(const QString& username);
    bool userExists(const QString& username);
    QVector<User> loadAllUsers();

    bool saveMessage(const QString& sessionId, const Message& message);
    QVector<Message> loadMessages(const QString& sessionId);
    bool deleteSession(const QString& sessionId);
    QVector<QString> loadAllSessionIds();

    void logActivity(const QString& activity);
};

#endif // FILEHANDLER_H