#ifndef CHATBOT_H
#define CHATBOT_H

#include <QString>
#include <QVector>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QFile>
#include "message.h"
#include "filehandler.h"

class BaseChatbot : public QObject {
    Q_OBJECT
protected:
    QString botName;
    QString sessionId;

public:
    explicit BaseChatbot(const QString& name, QObject* parent = nullptr);
    virtual ~BaseChatbot() {}

    virtual void sendMessage(const QString& userMessage) = 0;
    virtual QString getBotName() const = 0;

signals:
    void responseReady(const QString& response);
    void errorOccurred(const QString& error);
    void typingStarted();
};

class AIChatbot : public BaseChatbot {
    Q_OBJECT
private:
    QNetworkAccessManager* networkManager;
    FileHandler* fileHandler;
    QVector<Message> conversationHistory;
    QString apiKey;
    QString currentUsername;
    bool isWaitingForResponse;

    void parseAndEmitResponse(QNetworkReply* reply);
    QJsonArray buildMessageHistory() const;
    QString currentModel;

public:
    explicit AIChatbot(const QString& apiKey, QObject* parent = nullptr);
    ~AIChatbot();

    void sendMessage(const QString& userMessage) override;
    void sendMessageWithFile(const QString& userMessage,
                             const QString& filePath,
                             const QString& fileType);
    QString getBotName() const override;

    void startNewSession(const QString& username);
    void loadSession(const QString& sessionId);
    void clearHistory();

    QVector<Message> getHistory() const;
    QString getSessionId() const;
    void setApiKey(const QString& key);
    void setModel(const QString& model);
    QString getModel() const;

private slots:
    void onNetworkReply(QNetworkReply* reply);
};

#endif // CHATBOT_H