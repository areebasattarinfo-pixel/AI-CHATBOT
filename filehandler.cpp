#include "filehandler.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

FileHandler::FileHandler() {
    QString dataPath = QStandardPaths::writableLocation(
                           QStandardPaths::AppDataLocation)
                       + "/AIChatbot/";
    QDir().mkpath(dataPath);

    usersFilePath = dataPath + "users.txt";
    chatsFilePath = dataPath + "chats/";
    logsFilePath  = dataPath + "activity.log";

    QDir().mkpath(chatsFilePath);
    ensureFileExists(usersFilePath);
    ensureFileExists(logsFilePath);
}

bool FileHandler::ensureFileExists(const QString& path) {
    QFile file(path);
    if (!file.exists())
        return file.open(QIODevice::WriteOnly);
    return true;
}

// ─── USER CRUD ────────────────────────────────────────────

bool FileHandler::saveUser(const User& user) {
    if (userExists(user.getUsername())) return false;
    QFile file(usersFilePath);
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return false;
    QTextStream out(&file);
    out << user.toFileString() << "\n";
    file.close();
    logActivity("User registered: " + user.getUsername());
    return true;
}

User FileHandler::loadUser(const QString& username) {
    QFile file(usersFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return User();
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        User u = User::fromFileString(line);
        if (u.getUsername() == username) {
            file.close();
            return u;
        }
    }
    file.close();
    return User();
}

bool FileHandler::updateUser(const User& user) {
    QVector<User> allUsers = loadAllUsers();
    QFile file(usersFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&file);
    for (User& u : allUsers) {
        if (u.getUsername() == user.getUsername())
            out << user.toFileString() << "\n";
        else
            out << u.toFileString() << "\n";
    }
    file.close();
    logActivity("User updated: " + user.getUsername());
    return true;
}

bool FileHandler::deleteUser(const QString& username) {
    QVector<User> allUsers = loadAllUsers();
    QFile file(usersFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&file);
    for (const User& u : allUsers) {
        if (u.getUsername() != username)
            out << u.toFileString() << "\n";
    }
    file.close();
    logActivity("User deleted: " + username);
    return true;
}

bool FileHandler::userExists(const QString& username) {
    QFile file(usersFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        User u = User::fromFileString(line);
        if (u.getUsername() == username) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

QVector<User> FileHandler::loadAllUsers() {
    QVector<User> users;
    QFile file(usersFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return users;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty())
            users.append(User::fromFileString(line));
    }
    file.close();
    return users;
}

// ─── MESSAGE CRUD ─────────────────────────────────────────

bool FileHandler::saveMessage(const QString& sessionId,
                              const Message& message) {
    QString filePath = chatsFilePath + sessionId + ".txt";
    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return false;
    QTextStream out(&file);
    out << message.toFileString() << "\n";
    file.close();
    return true;
}

QVector<Message> FileHandler::loadMessages(const QString& sessionId) {
    QVector<Message> messages;
    QString filePath = chatsFilePath + sessionId + ".txt";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return messages;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty())
            messages.append(Message::fromFileString(line));
    }
    file.close();
    return messages;
}

bool FileHandler::deleteSession(const QString& sessionId) {
    QString filePath = chatsFilePath + sessionId + ".txt";
    logActivity("Session deleted: " + sessionId);
    return QFile::remove(filePath);
}

QVector<QString> FileHandler::loadAllSessionIds() {
    QVector<QString> ids;
    QDir dir(chatsFilePath);
    QStringList files = dir.entryList(QStringList() << "*.txt",
                                      QDir::Files);
    for (const QString& f : files)
        ids.append(QString(f).replace(".txt", ""));
    return ids;
}

void FileHandler::logActivity(const QString& activity) {
    QFile file(logsFilePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) return;
    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString(Qt::ISODate)
        << " | " << activity << "\n";
    file.close();
}