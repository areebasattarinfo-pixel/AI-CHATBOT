#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QStackedWidget>
#include "chatbot.h"
#include "user.h"
#include "filehandler.h"
#include "voicerecorder.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QClipboard>
#include <QMimeData>
#include <QImage>
#include <QShortcut>
#include <QStandardPaths>
#include <QComboBox>
#include <QTextToSpeech>

class MainWindow : public QMainWindow {
    Q_OBJECT
bool eventFilter(QObject* obj, QEvent* event) override;
private:
    // Core objects
    AIChatbot* chatbot;
    FileHandler* fileHandler;
    User currentUser;
    bool isLoggedIn;

    // Pages
    QStackedWidget* stackedWidget;
    QWidget* loginPage;
    QWidget* chatPage;
    QWidget* historyPage;

    // Login page widgets
    QLineEdit* usernameInput;
    QLineEdit* passwordInput;
    QLineEdit* emailInput;
    QLabel* loginStatusLabel;
    QPushButton* loginBtn;
    QPushButton* registerBtn;
    bool isRegistering;

    // Chat page widgets
    QWidget* chatContainer;
    QScrollArea* scrollArea;
    QVBoxLayout* messagesLayout;
    QTextEdit* messageInput;
    QPushButton* sendBtn;
    QPushButton* newChatBtn;
    QPushButton* historyBtn;
    QPushButton* logoutBtn;
    QLabel* botStatusLabel;
    QLabel* typingLabel;
    QTimer* typingTimer;
    // Voice
    VoiceRecorder* voiceRecorder;
    QPushButton*   micBtn;
    bool           isRecording;
    bool isDarkMode;
    int typingDots;

    // History page widgets
    QWidget* historyContainer;
    QVBoxLayout* historyLayout;
    QPushButton* backFromHistoryBtn;

    // Setup methods
    void setupLoginPage();
    void setupChatPage();
    void setupHistoryPage();
    void setupProfilePage();
    void applyGlobalStyles();

    // Chat UI helpers
    void addMessageBubble(const QString& sender,
                          const QString& content,
                          bool isUser);
    void scrollToBottom();
    void clearChatDisplay();
    void loadHistoryList();
    QPushButton* emojiBtn;
    QWidget* emojiPicker;
    bool emojiPickerVisible;
    QPushButton* attachBtn;
    QString attachedFilePath;
    QString attachedFileType;
    QLabel* attachmentPreview;
    QComboBox* modelSelector;
    QWidget* profilePage;
    QPushButton* profileBtn;
    QTextToSpeech* tts;
    bool ttsEnabled;
    QPushButton* ttsToggleBtn;

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onSendMessage();
    void onBotResponse(const QString& response);
    void onBotError(const QString& error);
    void onTypingStarted();
    void onLoginClicked();
    void onRegisterClicked();
    void onLogoutClicked();
    void onNewChat();
    void onShowHistory();
    void onBackFromHistory();
    void updateTypingAnimation();
    void onMicButtonClicked();
    void onTranscriptionReady(const QString& text);
    void onVoiceError(const QString& error);
    void onSessionClicked(const QString& sessionId);
    void onToggleTheme();
    void onCopyMessage(const QString& text);
    void onDeleteChat();
    void onEmojiPicked(const QString& emoji);
    void onShowEmojiPicker();
    void onAttachFile();
    void onRemoveAttachment();
    void onPasteFromClipboard();
    void onShowProfile();
    void onSaveProfile();
    void onBackFromProfile();
    void onSpeakMessage(const QString& text);
    void onToggleTTS();
};

#endif // MAINWINDOW_H
