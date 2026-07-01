#include "voicerecorder.h"
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDir>

VoiceRecorder::VoiceRecorder(const QString& key, QObject* parent)
    : QObject(parent), apiKey(key), isRecording(false)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &VoiceRecorder::onWhisperReply);

    outputFilePath = QStandardPaths::writableLocation(
                         QStandardPaths::TempLocation)
                     + "/chatbot_voice.wav";

    audioInput = new QAudioInput(this);
    session    = new QMediaCaptureSession(this);
    recorder   = new QMediaRecorder(this);

    session->setAudioInput(audioInput);
    session->setRecorder(recorder);

    // WAV format set karo
    QMediaFormat format;
    format.setFileFormat(QMediaFormat::Wave);
    format.setAudioCodec(QMediaFormat::AudioCodec::Wave);
    recorder->setMediaFormat(format);
    recorder->setQuality(QMediaRecorder::HighQuality);
    recorder->setOutputLocation(QUrl::fromLocalFile(outputFilePath));

    connect(recorder, &QMediaRecorder::recorderStateChanged,
            this, &VoiceRecorder::onRecorderStateChanged);
}

VoiceRecorder::~VoiceRecorder() {}

bool VoiceRecorder::getIsRecording() const { return isRecording; }

void VoiceRecorder::startRecording() {
    if (isRecording) return;
    QFile::remove(outputFilePath);
    recorder->record();
    isRecording = true;
    emit recordingStarted();
    qDebug() << "Recording started...";
}

void VoiceRecorder::stopRecording() {
    if (!isRecording) return;
    recorder->stop();
    isRecording = false;
    emit recordingStopped();
    qDebug() << "Recording stopped.";
}

void VoiceRecorder::onRecorderStateChanged(QMediaRecorder::RecorderState state) {
    if (state == QMediaRecorder::StoppedState) {
        QFile file(outputFilePath);
        if (file.exists() && file.size() > 0) {
            qDebug() << "Audio file size:" << file.size();
            sendToWhisper(outputFilePath);
        } else {
            emit errorOccurred("Audio file not found or empty.");
        }
    }
}

void VoiceRecorder::sendToWhisper(const QString& filePath) {
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit errorOccurred("Cannot open audio file.");
        delete file;
        return;
    }

    QUrl url("https://api.groq.com/openai/v1/audio/transcriptions");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QHttpMultiPart* multiPart = new QHttpMultiPart(
        QHttpMultiPart::FormDataType);

    // Audio file part
    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/wav");
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"file\"; filename=\"voice.wav\"");
    audioPart.setBodyDevice(file);
    file->setParent(multiPart);

    // Model part
    QHttpPart modelPart;
    modelPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        "form-data; name=\"model\"");
    modelPart.setBody("whisper-large-v3");

    // Language part
    QHttpPart langPart;
    langPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       "form-data; name=\"language\"");
    langPart.setBody("en");

    multiPart->append(audioPart);
    multiPart->append(modelPart);
    multiPart->append(langPart);

    QNetworkReply* reply = networkManager->post(request, multiPart);
    multiPart->setParent(reply);

    qDebug() << "Sending audio to Whisper...";
}

void VoiceRecorder::onWhisperReply(QNetworkReply* reply) {
    QByteArray data = reply->readAll();
    qDebug() << "Whisper Response:" << data;

    QJsonDocument doc  = QJsonDocument::fromJson(data);
    QJsonObject   root = doc.object();

    if (root.contains("error")) {
        QString err = root["error"].toObject()["message"].toString();
        emit errorOccurred("Whisper Error: " + err);
    } else if (root.contains("text")) {
        QString text = root["text"].toString().trimmed();
        if (!text.isEmpty())
            emit transcriptionReady(text);
        else
            emit errorOccurred("No speech detected.");
    }
    reply->deleteLater();
}