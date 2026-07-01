#include "chatbot.h"
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>

// ─── BaseChatbot ──────────────────────────────────────────
BaseChatbot::BaseChatbot(const QString& name, QObject* parent)
    : QObject(parent), botName(name) {}

// ─── AIChatbot ────────────────────────────────────────────
AIChatbot::AIChatbot(const QString& key, QObject* parent)
    : BaseChatbot("Claude AI", parent),
    apiKey(key),
    isWaitingForResponse(false),
    currentModel("llama-3.3-70b-versatile")
{
    networkManager = new QNetworkAccessManager(this);
    fileHandler    = new FileHandler();
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &AIChatbot::onNetworkReply);
    startNewSession("default");
}AIChatbot::~AIChatbot() {
    delete fileHandler;
}

QString AIChatbot::getBotName() const { return botName; }
QVector<Message> AIChatbot::getHistory() const { return conversationHistory; }
QString AIChatbot::getSessionId() const { return sessionId; }
void AIChatbot::setApiKey(const QString& key) { apiKey = key; }

void AIChatbot::startNewSession(const QString& username) {
    currentUsername = username;
    sessionId = "session_" + QUuid::createUuid().toString()
                                 .remove("{").remove("}").remove("-")
                                 .left(12);
    conversationHistory.clear();
    fileHandler->logActivity("New session: " + sessionId
                             + " for user: " + username);
}

void AIChatbot::loadSession(const QString& sid) {
    sessionId = sid;
    conversationHistory = fileHandler->loadMessages(sid);
}

void AIChatbot::clearHistory() {
    conversationHistory.clear();
}

QJsonArray AIChatbot::buildMessageHistory() const {
    QJsonArray arr;
    for (const Message& msg : conversationHistory) {
        QJsonObject obj;
        obj["role"]    = (msg.getSender() == "You") ? "user" : "assistant";
        obj["content"] = msg.getContent();
        arr.append(obj);
    }
    return arr;
}

void AIChatbot::sendMessage(const QString& userMessage) {
    if (isWaitingForResponse || userMessage.trimmed().isEmpty()) return;

    isWaitingForResponse = true;
    emit typingStarted();

    Message userMsg("You", userMessage);
    conversationHistory.append(userMsg);
    fileHandler->saveMessage(sessionId, userMsg);

    QUrl url("https://api.groq.com/openai/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    // Only last 10 messages bhejo
    QJsonArray messages;

    QJsonObject sysMsg;
    sysMsg["role"]    = "system";
    sysMsg["content"] = "You are a helpful, friendly and intelligent AI assistant.";
    messages.append(sysMsg);

    int start = qMax(0, conversationHistory.size() - 10);
    for (int i = start; i < conversationHistory.size(); i++) {
        const Message& msg = conversationHistory[i];
        QJsonObject obj;
        obj["role"]    = (msg.getSender() == "You") ? "user" : "assistant";
        obj["content"] = msg.getContent();
        messages.append(obj);
    }

    QJsonObject body;
    body["model"] = currentModel;
    body["messages"]    = messages;
    body["max_tokens"]  = 1024;
    body["temperature"] = 0.7;

    QJsonDocument doc(body);
    QByteArray requestData = doc.toJson();

    qDebug() << "Sending to Groq:" << requestData;

    networkManager->post(request, requestData);
}
void AIChatbot::onNetworkReply(QNetworkReply* reply) {
    isWaitingForResponse = false;
    parseAndEmitResponse(reply);
    reply->deleteLater();
}

void AIChatbot::parseAndEmitResponse(QNetworkReply* reply) {
    QByteArray responseData = reply->readAll();

    qDebug() << "=== GROQ RESPONSE ===";
    qDebug() << "HTTP Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "Full Response:" << responseData;
    qDebug() << "====================";

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network Error:" << reply->errorString();
    }

    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject root  = doc.object();

    if (root.contains("error")) {
        QString errMsg = root["error"].toObject()["message"].toString();
        qDebug() << "API Error Message:" << errMsg;
        emit errorOccurred("API Error: " + errMsg);
        return;
    }

    QJsonArray choices = root["choices"].toArray();
    if (choices.isEmpty()) {
        emit errorOccurred("Empty response from Groq.");
        return;
    }

    QString responseText = choices[0].toObject()
                               ["message"].toObject()
                                       ["content"].toString();

    Message botMsg("Assistant", responseText);
    conversationHistory.append(botMsg);
    fileHandler->saveMessage(sessionId, botMsg);

    emit responseReady(responseText);
}
void AIChatbot::sendMessageWithFile(const QString& userMessage,
                                    const QString& filePath,
                                    const QString& fileType) {
    if (isWaitingForResponse) return;

    isWaitingForResponse = true;
    emit typingStarted();

    // Read file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Cannot open file.");
        isWaitingForResponse = false;
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    QUrl url("https://api.groq.com/openai/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QJsonArray messages;

    // System message
    QJsonObject sysMsg;
    sysMsg["role"]    = "system";
    sysMsg["content"] = "You are a helpful AI assistant. Analyze files and images carefully and provide detailed, accurate responses.";
    messages.append(sysMsg);

    // Previous history
    int start = qMax(0, conversationHistory.size() - 8);
    for (int i = start; i < conversationHistory.size(); i++) {
        const Message& msg = conversationHistory[i];
        QJsonObject obj;
        obj["role"]    = (msg.getSender() == "You") ? "user" : "assistant";
        obj["content"] = msg.getContent();
        messages.append(obj);
    }

    // Current message with file
    QJsonObject userMsg;
    userMsg["role"] = "user";

    if (fileType == "image") {
        // Image — base64 encode
        QString base64 = QString::fromLatin1(fileData.toBase64());
        QString ext = QFileInfo(filePath).suffix().toLower();
        QString mimeType = "image/jpeg";
        if (ext == "png")  mimeType = "image/png";
        if (ext == "gif")  mimeType = "image/gif";
        if (ext == "webp") mimeType = "image/webp";

        QJsonArray contentArr;

        // Text part
        QJsonObject textPart;
        textPart["type"] = "text";
        textPart["text"] = userMessage.isEmpty()
                               ? "Please analyze this image in detail."
                               : userMessage;
        contentArr.append(textPart);

        // Image part
        QJsonObject imgPart;
        imgPart["type"] = "image_url";
        QJsonObject imgUrl;
        imgUrl["url"] = "data:" + mimeType + ";base64," + base64;
        imgPart["image_url"] = imgUrl;
        contentArr.append(imgPart);

        userMsg["content"] = contentArr;

    } else {
        // Text file — read content directly
        QString fileContent = QString::fromUtf8(fileData);
        QString fileName    = QFileInfo(filePath).fileName();
        QString prompt      = userMessage.isEmpty()
                             ? "Please analyze this file and provide insights:"
                             : userMessage;

        userMsg["content"] = prompt + "\n\n📄 File: " + fileName +
                             "\n\n" + fileContent.left(8000);
    }

    messages.append(userMsg);

    // Save user message
    QString displayMsg = userMessage.isEmpty()
                             ? "[File: " + QFileInfo(filePath).fileName() + "]"
                             : userMessage + "\n[File: " + QFileInfo(filePath).fileName() + "]";
    Message savedMsg("You", displayMsg);
    conversationHistory.append(savedMsg);
    fileHandler->saveMessage(sessionId, savedMsg);

    QJsonObject body;
    body["model"] = (fileType == "image")
                        ? "meta-llama/llama-4-scout-17b-16e-instruct"
                        : "llama-3.3-70b-versatile";
    body["messages"]   = messages;
    body["max_tokens"] = 1024;

    QJsonDocument doc(body);
    networkManager->post(request, doc.toJson());
}
void AIChatbot::setModel(const QString& model) {
    currentModel = model;
}

QString AIChatbot::getModel() const {
    return currentModel;
}