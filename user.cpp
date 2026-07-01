#include "user.h"
#include <QCryptographicHash>

User::User() {}

User::User(const QString& username,
           const QString& email,
           const QString& password) {
    this->username     = username;
    this->email        = email;
    this->passwordHash = hashPassword(password);
}

QString User::getUsername() const { return username; }
QString User::getEmail()    const { return email; }
QString User::getPasswordHash() const { return passwordHash; }
QVector<QString> User::getChatSessionIds() const { return chatSessionIds; }

void User::setUsername(const QString& u) { username = u; }
void User::setEmail(const QString& e)    { email = e; }

void User::addSessionId(const QString& id) {
    if (!chatSessionIds.contains(id))
        chatSessionIds.append(id);
}

QString User::hashPassword(const QString& password) const {
    QByteArray hash = QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Sha256
        );
    return QString(hash.toHex());
}

bool User::validatePassword(const QString& password) const {
    return passwordHash == hashPassword(password);
}

QString User::toFileString() const {
    // Format: username|email|passwordHash|session1,session2,...
    QString sessions = chatSessionIds.join(",");
    return username + "|" + email + "|" + passwordHash + "|" + sessions;
}

User User::fromFileString(const QString& line) {
    User user;
    QStringList parts = line.split("|");
    if (parts.size() >= 3) {
        user.username     = parts[0];
        user.email        = parts[1];
        user.passwordHash = parts[2];
        if (parts.size() >= 4 && !parts[3].isEmpty()) {
            QStringList sessions = parts[3].split(",");
            for (const QString& s : sessions)
                user.chatSessionIds.append(s);
        }
    }
    return user;
}