#include "mainwindow.h"
#include <QScrollBar>
#include <QUuid>
#include <QDateTime>
#include <QMessageBox>
#include "mainwindow.h"
#include <QScrollBar>
#include <QUuid>
#include <QDateTime>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), isLoggedIn(false), isRegistering(false),
    typingDots(0), isRecording(false),isDarkMode(true)

{
    setWindowTitle("AI Chatbot");
    setMinimumSize(900, 650);
    resize(1000, 700);

    fileHandler = new FileHandler();

    QString apiKey = "YOUR_API_KEY";
    chatbot = new AIChatbot(apiKey, this);

    connect(chatbot, &AIChatbot::responseReady,
            this,    &MainWindow::onBotResponse);
    connect(chatbot, &AIChatbot::errorOccurred,
            this,    &MainWindow::onBotError);
    connect(chatbot, &AIChatbot::typingStarted,
            this,    &MainWindow::onTypingStarted);

    typingTimer = new QTimer(this);
    tts = new QTextToSpeech(this);
    ttsEnabled = false;
    connect(typingTimer, &QTimer::timeout,
            this, &MainWindow::updateTypingAnimation);

    // Voice recorder
    voiceRecorder = new VoiceRecorder(apiKey, this);
    connect(voiceRecorder, &VoiceRecorder::transcriptionReady,
            this, &MainWindow::onTranscriptionReady);
    connect(voiceRecorder, &VoiceRecorder::errorOccurred,
            this, &MainWindow::onVoiceError);
    connect(voiceRecorder, &VoiceRecorder::recordingStarted, this, [this]() {
        micBtn->setStyleSheet(
            "background:#ef4444; border-radius:14px; color:white; font-size:20px; border:none;");
        micBtn->setText("⏹");
        typingLabel->setText("🔴 Recording...");
    });
    connect(voiceRecorder, &VoiceRecorder::recordingStopped, this, [this]() {
        micBtn->setStyleSheet(
            "background:#1e2640; border-radius:14px; color:#7c83fd; font-size:20px; border:none;");
        micBtn->setText("🎤");
        typingLabel->setText("⏳ Transcribing...");
    });

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupLoginPage();
    setupChatPage();
    setupHistoryPage();
    applyGlobalStyles();

    stackedWidget->setCurrentWidget(loginPage);
}

MainWindow::~MainWindow() { delete fileHandler; }

void MainWindow::setupLoginPage() {
    loginPage = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(loginPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget* leftPanel = new QWidget();
    leftPanel->setObjectName("leftPanel");
    leftPanel->setFixedWidth(420);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setAlignment(Qt::AlignCenter);
    leftLayout->setSpacing(16);

    QLabel* logoIcon = new QLabel("🤖");
    logoIcon->setAlignment(Qt::AlignCenter);
    logoIcon->setStyleSheet("font-size: 72px;");

    QLabel* brandName = new QLabel("AI Chatbot");
    brandName->setObjectName("brandName");
    brandName->setAlignment(Qt::AlignCenter);

    QLabel* brandSub = new QLabel("Your intelligent conversation partner");
    brandSub->setObjectName("brandSub");
    brandSub->setAlignment(Qt::AlignCenter);

    QFrame* divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setObjectName("brandDivider");
    divider->setFixedWidth(200);

    QLabel* features = new QLabel("✦  Smart AI Responses\n✦  Secure Login System\n✦  Voice Input Support\n✦  Chat History Saved");
    features->setObjectName("featureList");
    features->setAlignment(Qt::AlignCenter);

    leftLayout->addStretch();
    leftLayout->addWidget(logoIcon);
    leftLayout->addWidget(brandName);
    leftLayout->addWidget(brandSub);
    leftLayout->addWidget(divider, 0, Qt::AlignCenter);
    leftLayout->addWidget(features);
    leftLayout->addStretch();

    QWidget* rightPanel = new QWidget();
    rightPanel->setObjectName("rightPanel");
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setAlignment(Qt::AlignCenter);

    QWidget* formCard = new QWidget();
    formCard->setObjectName("formCard");
    formCard->setFixedWidth(360);
    QVBoxLayout* formLayout = new QVBoxLayout(formCard);
    formLayout->setSpacing(14);
    formLayout->setContentsMargins(36, 40, 36, 40);

    QLabel* formTitle = new QLabel("Welcome Back");
    formTitle->setObjectName("formTitle");
    formTitle->setAlignment(Qt::AlignCenter);

    QLabel* formSubtitle = new QLabel("Sign in to continue");
    formSubtitle->setObjectName("formSubtitle");
    formSubtitle->setAlignment(Qt::AlignCenter);

    emailInput = new QLineEdit();
    emailInput->setPlaceholderText("Email address");
    emailInput->setObjectName("formInput");
    emailInput->setFixedHeight(44);
    emailInput->hide();

    usernameInput = new QLineEdit();
    usernameInput->setPlaceholderText("Username");
    usernameInput->setObjectName("formInput");
    usernameInput->setFixedHeight(44);

    passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setObjectName("formInput");
    passwordInput->setFixedHeight(44);

    loginStatusLabel = new QLabel("");
    loginStatusLabel->setObjectName("statusLabel");
    loginStatusLabel->setAlignment(Qt::AlignCenter);
    loginStatusLabel->setWordWrap(true);

    loginBtn = new QPushButton("Sign In");
    loginBtn->setObjectName("primaryBtn");
    loginBtn->setFixedHeight(46);
    loginBtn->setCursor(Qt::PointingHandCursor);

    registerBtn = new QPushButton("Create an account");
    registerBtn->setObjectName("linkBtn");
    registerBtn->setCursor(Qt::PointingHandCursor);

    connect(loginBtn,    &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(registerBtn, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
    connect(passwordInput, &QLineEdit::returnPressed, this, &MainWindow::onLoginClicked);

    formLayout->addWidget(formTitle);
    formLayout->addWidget(formSubtitle);
    formLayout->addSpacing(10);
    formLayout->addWidget(emailInput);
    formLayout->addWidget(usernameInput);
    formLayout->addWidget(passwordInput);
    formLayout->addWidget(loginStatusLabel);
    formLayout->addSpacing(4);
    formLayout->addWidget(loginBtn);
    formLayout->addWidget(registerBtn);

    rightLayout->addWidget(formCard, 0, Qt::AlignCenter);
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel, 1);
    stackedWidget->addWidget(loginPage);
}
void MainWindow::setupChatPage() {
    chatPage = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(chatPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget* sidebar = new QWidget();
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(220);
    QVBoxLayout* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(12, 20, 12, 20);
    sideLayout->setSpacing(8);

    QLabel* sideTitle = new QLabel("🤖  AI Chatbot");
    sideTitle->setObjectName("sideTitle");

    newChatBtn = new QPushButton("＋  New Chat");
    newChatBtn->setObjectName("sideBtn");
    newChatBtn->setFixedHeight(42);
    newChatBtn->setCursor(Qt::PointingHandCursor);

    historyBtn = new QPushButton("🕐  Chat History");
    historyBtn->setObjectName("sideBtnGhost");
    historyBtn->setFixedHeight(42);
    historyBtn->setCursor(Qt::PointingHandCursor);

    QFrame* sideDivider = new QFrame();
    sideDivider->setFrameShape(QFrame::HLine);
    sideDivider->setObjectName("sideDivider");

    botStatusLabel = new QLabel("● Online");
    botStatusLabel->setObjectName("onlineStatus");

    QPushButton* deleteBtn = new QPushButton("🗑️  Delete Chat");
    deleteBtn->setObjectName("deleteBtn");
    deleteBtn->setFixedHeight(38);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    connect(deleteBtn, &QPushButton::clicked,
            this, &MainWindow::onDeleteChat);
    sideLayout->addWidget(deleteBtn);


    logoutBtn = new QPushButton("⎋  Logout");
    logoutBtn->setObjectName("logoutBtn");
    logoutBtn->setFixedHeight(38);
    logoutBtn->setCursor(Qt::PointingHandCursor);

    connect(newChatBtn, &QPushButton::clicked, this, &MainWindow::onNewChat);
    connect(historyBtn, &QPushButton::clicked, this, &MainWindow::onShowHistory);
    connect(logoutBtn,  &QPushButton::clicked, this, &MainWindow::onLogoutClicked);

    sideLayout->addWidget(sideTitle);
    sideLayout->addSpacing(10);
    sideLayout->addWidget(newChatBtn);
    sideLayout->addWidget(historyBtn);
    sideLayout->addWidget(sideDivider);
    sideLayout->addWidget(botStatusLabel);
    sideLayout->addStretch();
    sideLayout->addWidget(logoutBtn);

    QWidget* chatArea = new QWidget();
    chatArea->setObjectName("chatArea");
    QVBoxLayout* chatLayout = new QVBoxLayout(chatArea);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    QWidget* headerBar = new QWidget();
    headerBar->setObjectName("headerBar");
    headerBar->setFixedHeight(60);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerBar);
    headerLayout->setContentsMargins(24, 0, 24, 0);

    QLabel* headerTitle = new QLabel("💬  New Conversation");
    headerTitle->setObjectName("headerTitle");

    typingLabel = new QLabel("");
    typingLabel->setObjectName("typingLabel");

    headerLayout->addWidget(headerTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(typingLabel);
    // Model selector
    modelSelector = new QComboBox();
    modelSelector->setObjectName("modelSelector");
    modelSelector->setCursor(Qt::PointingHandCursor);
    modelSelector->addItem("⚡ Llama 3.3 70B", "llama-3.3-70b-versatile");
    modelSelector->addItem("🚀 Llama 4 Scout", "meta-llama/llama-4-scout-17b-16e-instruct");
    modelSelector->addItem("💎 Mixtral 8x7B", "mixtral-8x7b-32768");
    modelSelector->addItem("🌟 Gemma 2 9B", "gemma2-9b-it");
    modelSelector->addItem("⚡ Llama 3.1 8B", "llama-3.1-8b-instant");
    modelSelector->setFixedHeight(34);
    modelSelector->setFixedWidth(180);

    connect(modelSelector, &QComboBox::currentIndexChanged,
            this, [this](int index) {
                QString model = modelSelector->itemData(index).toString();
                chatbot->setModel(model);
                botStatusLabel->setText("✅ Model changed!");
                QTimer::singleShot(2000, this, [this]() {
                    botStatusLabel->setText("● Online");
                });
            });

    headerLayout->addWidget(modelSelector);

    // Theme toggle button
    QPushButton* themeBtn = new QPushButton("☀️");
    themeBtn->setObjectName("themeBtn");
    themeBtn->setFixedSize(36, 36);
    themeBtn->setCursor(Qt::PointingHandCursor);
    themeBtn->setToolTip("Toggle Light/Dark Mode");
    connect(themeBtn, &QPushButton::clicked,
            this, &MainWindow::onToggleTheme);
    headerLayout->addWidget(themeBtn);
    ttsToggleBtn = new QPushButton("🔇");
    ttsToggleBtn->setObjectName("micBtn");
    ttsToggleBtn->setFixedSize(36, 36);
    ttsToggleBtn->setCursor(Qt::PointingHandCursor);
    ttsToggleBtn->setToolTip("Toggle Text to Speech");
    connect(ttsToggleBtn, &QPushButton::clicked,
            this, &MainWindow::onToggleTTS);
    headerLayout->addWidget(ttsToggleBtn);

    scrollArea = new QScrollArea();
    scrollArea->setObjectName("scrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    chatContainer = new QWidget();
    chatContainer->setObjectName("chatContainer");
    messagesLayout = new QVBoxLayout(chatContainer);
    messagesLayout->setContentsMargins(24, 20, 24, 20);
    messagesLayout->setSpacing(16);
    messagesLayout->addStretch();
    scrollArea->setWidget(chatContainer);

    QWidget* welcomeWidget = new QWidget();
    QVBoxLayout* wLayout = new QVBoxLayout(welcomeWidget);
    wLayout->setAlignment(Qt::AlignCenter);
    wLayout->setSpacing(8);

    QLabel* wIcon = new QLabel("✨");
    wIcon->setAlignment(Qt::AlignCenter);
    wIcon->setStyleSheet("font-size: 48px;");

    QLabel* wTitle = new QLabel("How can I help you today?");
    wTitle->setObjectName("welcomeTitle");
    wTitle->setAlignment(Qt::AlignCenter);

    QLabel* wSub = new QLabel("Type or use 🎤 voice input — I am here to assist!");
    wSub->setObjectName("welcomeSub");
    wSub->setAlignment(Qt::AlignCenter);

    wLayout->addWidget(wIcon);
    wLayout->addWidget(wTitle);
    wLayout->addWidget(wSub);
    messagesLayout->insertWidget(0, welcomeWidget);

    QWidget* inputArea = new QWidget();
    inputArea->setObjectName("inputArea");
    inputArea->setFixedHeight(100);
    QHBoxLayout* inputLayout = new QHBoxLayout(inputArea);
    inputLayout->setContentsMargins(20, 16, 20, 16);
    inputLayout->setSpacing(12);

    micBtn = new QPushButton("🎤");
    micBtn->setObjectName("micBtn");
    micBtn->setFixedSize(52, 52);
    micBtn->setCursor(Qt::PointingHandCursor);
    micBtn->setToolTip("Click to record voice");
    connect(micBtn, &QPushButton::clicked,
            this, &MainWindow::onMicButtonClicked);

    messageInput = new QTextEdit();
    messageInput->setObjectName("messageInput");
    messageInput->setPlaceholderText("Type a message or click 🎤 to speak...");
    messageInput->setFixedHeight(68);
    messageInput->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // Enter = send, Shift+Enter = new line
    installEventFilter(this);
    messageInput->installEventFilter(this);
    // Ctrl+V shortcut for clipboard paste
    QShortcut* pasteShortcut = new QShortcut(
        QKeySequence("Ctrl+V"), messageInput);
    connect(pasteShortcut, &QShortcut::activated,
            this, &MainWindow::onPasteFromClipboard);

    sendBtn = new QPushButton("➤");
    sendBtn->setObjectName("sendBtn");
    sendBtn->setFixedSize(52, 52);
    sendBtn->setCursor(Qt::PointingHandCursor);
    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::onSendMessage);

    // Attach button
    attachBtn = new QPushButton("📎");
    attachBtn->setObjectName("micBtn");
    attachBtn->setFixedSize(52, 52);
    attachBtn->setCursor(Qt::PointingHandCursor);
    attachBtn->setToolTip("Attach image or file");
    connect(attachBtn, &QPushButton::clicked,
            this, &MainWindow::onAttachFile);

    // Attachment preview label
    attachmentPreview = new QLabel("");
    attachmentPreview->setObjectName("attachmentPreview");
    attachmentPreview->setStyleSheet(
        "background: #1e2640; border-radius: 8px;"
        "color: #7c83fd; font-size: 12px; padding: 4px 10px;"
        );
    attachmentPreview->hide();
    attachmentPreview->setCursor(Qt::PointingHandCursor);
    connect(attachmentPreview, &QLabel::linkActivated,
            this, &MainWindow::onRemoveAttachment);
    // Emoji button
    emojiBtn = new QPushButton("😊");
    emojiBtn->setObjectName("micBtn");
    emojiBtn->setFixedSize(52, 52);
    emojiBtn->setCursor(Qt::PointingHandCursor);
    emojiBtn->setToolTip("Emoji picker");
    connect(emojiBtn, &QPushButton::clicked,
            this, &MainWindow::onShowEmojiPicker);

    // Emoji picker widget
    emojiPicker = new QWidget(chatPage);
    emojiPicker->setObjectName("emojiPicker");
    emojiPicker->setStyleSheet(
        "QWidget#emojiPicker { background: #161b2e; border-radius: 12px;"
        "border: 1px solid #2a2f4a; }"
        "QPushButton { background: transparent; border: none;"
        "font-size: 22px; padding: 4px; border-radius: 6px; }"
        "QPushButton:hover { background: #2a2f4a; }"
        );
    emojiPicker->hide();
    emojiPickerVisible = false;

    QGridLayout* emojiGrid = new QGridLayout(emojiPicker);
    emojiGrid->setSpacing(4);
    emojiGrid->setContentsMargins(10, 10, 10, 10);

    QStringList emojis = {
        "😊","😂","🥰","😍","🤔","😎","🥳","😢",
        "👍","👎","❤️","🔥","✅","⭐","🎉","🙏",
        "😅","🤣","😭","😤","🤯","😇","🥺","😏",
        "👋","💪","🤝","👏","💡","📝","🚀","💬"
    };

    int col = 0, row = 0;
    for (const QString& emoji : emojis) {
        QPushButton* eb = new QPushButton(emoji);
        eb->setFixedSize(38, 38);
        eb->setCursor(Qt::PointingHandCursor);
        QString em = emoji;
        connect(eb, &QPushButton::clicked, this, [this, em]() {
            onEmojiPicked(em);
        });
        emojiGrid->addWidget(eb, row, col);
        col++;
        if (col >= 8) { col = 0; row++; }
    }

    inputLayout->addWidget(attachBtn, 0, Qt::AlignBottom);
    inputLayout->addWidget(emojiBtn, 0, Qt::AlignBottom);
    inputLayout->addWidget(micBtn, 0, Qt::AlignBottom);
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendBtn, 0, Qt::AlignBottom);
    chatLayout->addWidget(headerBar);
    chatLayout->addWidget(scrollArea, 1);
    chatLayout->addWidget(attachmentPreview);
    chatLayout->addWidget(inputArea);
    chatLayout->addWidget(inputArea);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(chatArea, 1);
    stackedWidget->addWidget(chatPage);
}

void MainWindow::setupHistoryPage() {
    historyPage = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(historyPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget* header = new QWidget();
    header->setObjectName("headerBar");
    header->setFixedHeight(60);
    QHBoxLayout* hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(20, 0, 20, 0);

    backFromHistoryBtn = new QPushButton("← Back");
    backFromHistoryBtn->setObjectName("linkBtn");
    backFromHistoryBtn->setCursor(Qt::PointingHandCursor);
    connect(backFromHistoryBtn, &QPushButton::clicked,
            this, &MainWindow::onBackFromHistory);

    QLabel* histTitle = new QLabel("Chat History");
    histTitle->setObjectName("headerTitle");

    hLayout->addWidget(backFromHistoryBtn);
    hLayout->addStretch();
    hLayout->addWidget(histTitle);
    hLayout->addStretch();

    QScrollArea* histScroll = new QScrollArea();
    histScroll->setWidgetResizable(true);
    histScroll->setObjectName("scrollArea");

    historyContainer = new QWidget();
    historyContainer->setObjectName("chatContainer");
    historyLayout = new QVBoxLayout(historyContainer);
    historyLayout->setContentsMargins(30, 24, 30, 24);
    historyLayout->setSpacing(12);
    historyLayout->addStretch();
    histScroll->setWidget(historyContainer);

    mainLayout->addWidget(header);
    mainLayout->addWidget(histScroll, 1);
    stackedWidget->addWidget(historyPage);

}
void MainWindow::setupProfilePage() {
    profilePage = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(profilePage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header
    QWidget* header = new QWidget();
    header->setObjectName("headerBar");
    header->setFixedHeight(60);
    QHBoxLayout* hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(20, 0, 20, 0);

    QPushButton* backBtn = new QPushButton("← Back");
    backBtn->setObjectName("linkBtn");
    backBtn->setCursor(Qt::PointingHandCursor);
    connect(backBtn, &QPushButton::clicked,
            this, &MainWindow::onBackFromProfile);

    QLabel* title = new QLabel("My Profile");
    title->setObjectName("headerTitle");

    hLayout->addWidget(backBtn);
    hLayout->addStretch();
    hLayout->addWidget(title);
    hLayout->addStretch();

    // Content
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setObjectName("scrollArea");

    QWidget* content = new QWidget();
    content->setObjectName("chatContainer");
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(40, 30, 40, 30);
    contentLayout->setSpacing(20);
    contentLayout->setAlignment(Qt::AlignTop);

    // Avatar card
    QWidget* avatarCard = new QWidget();
    avatarCard->setStyleSheet(
        "background: #161b2e; border-radius: 16px; border: 1px solid #1e2640;");
    QVBoxLayout* avatarLayout = new QVBoxLayout(avatarCard);
    avatarLayout->setAlignment(Qt::AlignCenter);
    avatarLayout->setSpacing(12);
    avatarLayout->setContentsMargins(20, 24, 20, 24);

    // Avatar emoji selector
    QLabel* avatarLabel = new QLabel("👤");
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setStyleSheet("font-size: 64px;");
    avatarLabel->setObjectName("avatarDisplay");

    QLabel* changeAvatarLbl = new QLabel("Choose your avatar:");
    changeAvatarLbl->setStyleSheet("color: #6b74a0; font-size: 13px;");
    changeAvatarLbl->setAlignment(Qt::AlignCenter);

    // Avatar options
    QWidget* avatarOptions = new QWidget();
    QHBoxLayout* avatarRow = new QHBoxLayout(avatarOptions);
    avatarRow->setAlignment(Qt::AlignCenter);
    avatarRow->setSpacing(8);

    QStringList avatars = {"👤","🧑","👩","🧑‍💻","👩‍💻","🦸","🦸‍♀️","🤖","👨‍🎓","👩‍🎓"};
    for (const QString& av : avatars) {
        QPushButton* avBtn = new QPushButton(av);
        avBtn->setFixedSize(42, 42);
        avBtn->setCursor(Qt::PointingHandCursor);
        avBtn->setStyleSheet(
            "QPushButton { background: #1e2640; border-radius: 10px;"
            "font-size: 20px; border: 2px solid transparent; }"
            "QPushButton:hover { border-color: #7c83fd; }"
            );
        QString a = av;
        connect(avBtn, &QPushButton::clicked, this, [avatarLabel, a]() {
            avatarLabel->setText(a);
        });
        avatarRow->addWidget(avBtn);
    }

    avatarLayout->addWidget(avatarLabel);
    avatarLayout->addWidget(changeAvatarLbl);
    avatarLayout->addWidget(avatarOptions);

    // Info card
    QWidget* infoCard = new QWidget();
    infoCard->setStyleSheet(
        "background: #161b2e; border-radius: 16px; border: 1px solid #1e2640;");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(24, 20, 24, 20);
    infoLayout->setSpacing(14);

    QLabel* infoTitle = new QLabel("Account Information");
    infoTitle->setStyleSheet(
        "color: #7c83fd; font-size: 14px; font-weight: 600;");

    // Username field
    QLabel* userLbl = new QLabel("Username");
    userLbl->setStyleSheet("color: #6b74a0; font-size: 12px;");
    QLineEdit* userField = new QLineEdit();
    userField->setObjectName("formInput");
    userField->setFixedHeight(42);
    userField->setReadOnly(true);
    userField->setStyleSheet(
        "background: #0f1117; border: 1.5px solid #1e2640;"
        "border-radius: 10px; padding: 0 14px; font-size: 14px;"
        "color: #6b74a0;");

    // Email field
    QLabel* emailLbl = new QLabel("Email");
    emailLbl->setStyleSheet("color: #6b74a0; font-size: 12px;");
    QLineEdit* emailField = new QLineEdit();
    emailField->setObjectName("formInput");
    emailField->setFixedHeight(42);
    emailField->setStyleSheet(
        "background: #0f1117; border: 1.5px solid #1e2640;"
        "border-radius: 10px; padding: 0 14px;"
        "font-size: 14px; color: #e8eaf0;");

    // Display name field
    QLabel* nameLbl = new QLabel("Display Name");
    nameLbl->setStyleSheet("color: #6b74a0; font-size: 12px;");
    QLineEdit* nameField = new QLineEdit();
    nameField->setObjectName("formInput");
    nameField->setFixedHeight(42);
    nameField->setPlaceholderText("Enter display name...");
    nameField->setStyleSheet(
        "background: #0f1117; border: 1.5px solid #1e2640;"
        "border-radius: 10px; padding: 0 14px;"
        "font-size: 14px; color: #e8eaf0;");

    // Store fields for later use
    userField->setObjectName("profileUsername");
    emailField->setObjectName("profileEmail");
    nameField->setObjectName("profileDisplayName");
    avatarLabel->setObjectName("avatarDisplay");

    infoLayout->addWidget(infoTitle);
    infoLayout->addWidget(userLbl);
    infoLayout->addWidget(userField);
    infoLayout->addWidget(emailLbl);
    infoLayout->addWidget(emailField);
    infoLayout->addWidget(nameLbl);
    infoLayout->addWidget(nameField);

    // Stats card
    QWidget* statsCard = new QWidget();
    statsCard->setStyleSheet(
        "background: #161b2e; border-radius: 16px; border: 1px solid #1e2640;");
    QVBoxLayout* statsLayout = new QVBoxLayout(statsCard);
    statsLayout->setContentsMargins(24, 20, 24, 20);
    statsLayout->setSpacing(12);

    QLabel* statsTitle = new QLabel("Your Stats");
    statsTitle->setStyleSheet(
        "color: #7c83fd; font-size: 14px; font-weight: 600;");

    QVector<QString> allSessions = fileHandler->loadAllSessionIds();
    int totalChats = allSessions.size();
    int totalMsgs  = 0;
    for (const QString& sid : allSessions)
        totalMsgs += fileHandler->loadMessages(sid).size();

    QHBoxLayout* statsRow = new QHBoxLayout();

    auto makeStatWidget = [](const QString& val, const QString& label) {
        QWidget* w = new QWidget();
        w->setStyleSheet(
            "background: #1e2640; border-radius: 12px; padding: 4px;");
        QVBoxLayout* l = new QVBoxLayout(w);
        l->setAlignment(Qt::AlignCenter);
        l->setSpacing(4);
        QLabel* v = new QLabel(val);
        v->setStyleSheet(
            "color: #7c83fd; font-size: 24px; font-weight: 700;");
        v->setAlignment(Qt::AlignCenter);
        QLabel* lb = new QLabel(label);
        lb->setStyleSheet("color: #6b74a0; font-size: 12px;");
        lb->setAlignment(Qt::AlignCenter);
        l->addWidget(v);
        l->addWidget(lb);
        return w;
    };

    statsRow->addWidget(makeStatWidget(
        QString::number(totalChats), "Total Chats"));
    statsRow->addWidget(makeStatWidget(
        QString::number(totalMsgs), "Total Messages"));

    statsLayout->addWidget(statsTitle);
    statsLayout->addLayout(statsRow);

    // Save button
    QPushButton* saveBtn = new QPushButton("💾  Save Profile");
    saveBtn->setObjectName("primaryBtn");
    saveBtn->setFixedHeight(46);
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked,
            this, &MainWindow::onSaveProfile);

    contentLayout->addWidget(avatarCard);
    contentLayout->addWidget(infoCard);
    contentLayout->addWidget(statsCard);
    contentLayout->addWidget(saveBtn);

    scroll->setWidget(content);
    mainLayout->addWidget(header);
    mainLayout->addWidget(scroll, 1);
    stackedWidget->addWidget(profilePage);
}
void MainWindow::applyGlobalStyles() {
    setStyleSheet(
        "QMainWindow, QWidget { background: #0f1117; color: #e8eaf0;"
        "font-family: 'Segoe UI', Arial; }"
        "#leftPanel { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #1a1f35, stop:1 #0f1117); border-right: 1px solid #1e2235; }"
        "#brandName { font-size: 32px; font-weight: 700; color: #7c83fd; }"
        "#brandSub  { font-size: 13px; color: #8891b0; }"
        "#brandDivider { color: #2a2f4a; }"
        "#featureList { font-size: 13px; color: #6b74a0; }"
        "#rightPanel { background: #0f1117; }"
        "#formCard { background: #161b2e; border-radius: 16px;"
        "border: 1px solid #1e2640; }"
        "#formTitle { font-size: 24px; font-weight: 700; color: #e8eaf0; }"
        "#formSubtitle { font-size: 13px; color: #6b74a0; margin-bottom: 8px; }"
        "#formInput { background: #0f1117; border: 1.5px solid #1e2640;"
        "border-radius: 10px; padding: 0 14px; font-size: 14px; color: #e8eaf0; }"
        "#formInput:focus { border-color: #7c83fd; }"
        "#statusLabel { font-size: 12px; min-height: 16px; }"
        "#primaryBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #7c83fd, stop:1 #a78bfa); border: none; border-radius: 10px;"
        "color: white; font-size: 15px; font-weight: 600; }"
        "#primaryBtn:hover { background: #6366f1; }"
        "#linkBtn { background: transparent; border: none;"
        "color: #7c83fd; font-size: 13px; }"
        "#linkBtn:hover { color: #a78bfa; }"
        "#sidebar { background: #0d1121; border-right: 1px solid #1a2040; }"
        "#sideTitle { font-size: 16px; font-weight: 700;"
        "color: #7c83fd; padding: 4px 8px; }"
        "#sideBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #7c83fd, stop:1 #a78bfa); border: none; border-radius: 10px;"
        "color: white; font-size: 13px; font-weight: 600;"
        "text-align: left; padding-left: 14px; }"
        "#sideBtn:hover { background: #6366f1; }"
        "#sideBtnGhost { background: transparent; border: 1px solid #1e2640;"
        "border-radius: 10px; color: #8891b0; font-size: 13px;"
        "text-align: left; padding-left: 14px; }"
        "#sideBtnGhost:hover { background: #161b2e; color: #e8eaf0; }"
        "#onlineStatus { color: #4ade80; font-size: 12px; padding: 4px 8px; }"
        "#logoutBtn { background: transparent; border: 1px solid #2d1f2f;"
        "border-radius: 8px; color: #f87171; font-size: 13px; }"
        "#logoutBtn:hover { background: #2d1520; }"
        "#chatArea { background: #0f1117; }"
        "#headerBar { background: #0d1121; border-bottom: 1px solid #1a2040; }"
        "#headerTitle { font-size: 16px; font-weight: 600; color: #e8eaf0; }"
        "#typingLabel { font-size: 12px; color: #7c83fd; font-style: italic; }"
        "#scrollArea { background: #0f1117; border: none; }"
        "#chatContainer { background: #0f1117; }"
        "#welcomeTitle { font-size: 22px; font-weight: 700; color: #e8eaf0; }"
        "#welcomeSub { font-size: 14px; color: #6b74a0; }"
        "#inputArea { background: #0d1121; border-top: 1px solid #1a2040; }"
        "#messageInput { background: #161b2e; border: 1.5px solid #1e2640;"
        "border-radius: 12px; padding: 10px 14px;"
        "font-size: 14px; color: #e8eaf0; }"
        "#messageInput:focus { border-color: #7c83fd; }"
        "#sendBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #7c83fd, stop:1 #a78bfa); border: none;"
        "border-radius: 14px; color: white; font-size: 20px; }"
        "#sendBtn:hover { background: #6366f1; }"
        "#micBtn { background: #1e2640; border-radius: 14px;"
        "color: #7c83fd; font-size: 20px; border: none; }"
        "#micBtn:hover { background: #2a3060; }"
        "QScrollBar:vertical { background: #0f1117; width: 6px; border-radius: 3px; }"
        "QScrollBar::handle:vertical { background: #2a2f4a; border-radius: 3px; min-height: 30px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        );
}

void MainWindow::addMessageBubble(const QString& sender,
                                  const QString& content,
                                  bool isUser) {
    QWidget* row = new QWidget();
    row->setStyleSheet("background: transparent;");
    QHBoxLayout* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);

    QLabel* avatar = new QLabel(isUser ? "👤" : "🤖");
    avatar->setFixedSize(36, 36);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        isUser ? "background:#1e2640; border-radius:18px; font-size:16px;"
               : "background:#2a1f4a; border-radius:18px; font-size:16px;"
        );

    QLabel* bubble = new QLabel(content);
    bubble->setWordWrap(true);
    bubble->setMaximumWidth(520);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bubble->setStyleSheet(
        isUser
            ? "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
              "stop:0 #7c83fd,stop:1 #a78bfa);"
              "color:white; border-radius:14px; padding:12px 16px; font-size:14px;"
            : isDarkMode
                  ? "background:#161b2e; color:#e8eaf0; border-radius:14px;"
                    "padding:12px 16px; font-size:14px; border:1px solid #1e2640;"
                  : "background:#f0f2ff; color:#1a1a2e; border-radius:14px;"
                    "padding:12px 16px; font-size:14px; border:1px solid #d0d4ff;"
        );

    // Copy button
    QPushButton* copyBtn = new QPushButton("📋");
    copyBtn->setFixedSize(28, 28);
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setToolTip("Copy message");
    copyBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; font-size: 14px; }"
        "QPushButton:hover { background: #2a2f4a; border-radius: 6px; }"
        );
    QString msgContent = content;
    QPushButton* speakBtn = nullptr;
    if (!isUser) {
        speakBtn = new QPushButton("🔊");
        speakBtn->setFixedSize(28, 28);
        speakBtn->setCursor(Qt::PointingHandCursor);
        speakBtn->setToolTip("Listen to this message");
        speakBtn->setStyleSheet(
            "QPushButton { background: transparent; border: none; font-size: 14px; }"
            "QPushButton:hover { background: #2a2f4a; border-radius: 6px; }"
            );
        QString msgText = content;
        connect(speakBtn, &QPushButton::clicked, this, [this, msgText]() {
            onSpeakMessage(msgText);
        });
    }
    connect(copyBtn, &QPushButton::clicked, this, [this, msgContent]() {
        onCopyMessage(msgContent);
    });

    QWidget* bubbleCol = new QWidget();
    bubbleCol->setStyleSheet("background: transparent;");
    QVBoxLayout* colLayout = new QVBoxLayout(bubbleCol);
    colLayout->setContentsMargins(0, 0, 0, 0);
    colLayout->setSpacing(3);

    QLabel* senderLabel = new QLabel(sender);
    senderLabel->setStyleSheet(
        isUser ? "color:#7c83fd; font-size:11px; font-weight:600;"
               : "color:#a78bfa; font-size:11px; font-weight:600;"
        );
    senderLabel->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);

    QLabel* time = new QLabel(
        QDateTime::currentDateTime().toString("hh:mm ap"));
    time->setStyleSheet("color:#3d4466; font-size:10px;");
    time->setAlignment(isUser ? Qt::AlignRight : Qt::AlignLeft);

    // Bottom row — time + copy
    QHBoxLayout* bottomRow = new QHBoxLayout();
    bottomRow->setContentsMargins(0, 0, 0, 0);
    if (isUser) {
        bottomRow->addStretch();
        bottomRow->addWidget(copyBtn);
        bottomRow->addWidget(time);
    } else {
        bottomRow->addWidget(time);
        bottomRow->addWidget(copyBtn);
        if (speakBtn) bottomRow->addWidget(speakBtn);
        bottomRow->addStretch();
    }


    colLayout->addWidget(senderLabel);
    colLayout->addWidget(bubble);
    colLayout->addLayout(bottomRow);

    if (isUser) {
        rowLayout->addStretch();
        rowLayout->addWidget(bubbleCol);
        rowLayout->addWidget(avatar);
    } else {
        rowLayout->addWidget(avatar);
        rowLayout->addWidget(bubbleCol);
        rowLayout->addStretch();
    }

    // Slide in animation
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(row);
    row->setGraphicsEffect(effect);
    QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(300);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    int count = messagesLayout->count();
    messagesLayout->insertWidget(count - 1, row);
    scrollToBottom();
}

void MainWindow::scrollToBottom() {
    QTimer::singleShot(50, this, [this]() {
        scrollArea->verticalScrollBar()->setValue(
            scrollArea->verticalScrollBar()->maximum());
    });
}

void MainWindow::clearChatDisplay() {
    while (messagesLayout->count() > 1) {
        QLayoutItem* item = messagesLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
}

void MainWindow::onSendMessage() {
    QString text = messageInput->toPlainText().trimmed();

    if (!attachedFilePath.isEmpty()) {
        // Send with file
        if (text.isEmpty()) text = "";
        addMessageBubble("You",
                         text.isEmpty()
                             ? "📎 " + QFileInfo(attachedFilePath).fileName()
                             : text + "\n📎 " + QFileInfo(attachedFilePath).fileName(),
                         true);
        chatbot->sendMessageWithFile(text, attachedFilePath, attachedFileType);
        messageInput->clear();
        attachedFilePath.clear();
        attachedFileType.clear();
        attachmentPreview->hide();
        attachmentPreview->setText("");
        sendBtn->setEnabled(false);
        return;
    }

    if (text.isEmpty()) return;
    messageInput->clear();
    addMessageBubble("You", text, true);
    chatbot->sendMessage(text);
    sendBtn->setEnabled(false);
}

void MainWindow::onBotResponse(const QString& response) {
    typingTimer->stop();
    typingLabel->setText("");
    addMessageBubble("Assistant", response, false);
    sendBtn->setEnabled(true);

    if (ttsEnabled) {
        onSpeakMessage(response);
    }
}

void MainWindow::onBotError(const QString& error) {
    typingTimer->stop();
    typingLabel->setText("");
    addMessageBubble("System", "⚠ " + error, false);
    sendBtn->setEnabled(true);
}

void MainWindow::onTypingStarted() {
    typingDots = 0;
    typingTimer->start(500);
}

void MainWindow::updateTypingAnimation() {
    typingDots = (typingDots + 1) % 4;
    QString dots(typingDots, '.');
    typingLabel->setText("Assistant is typing" + dots);
}

void MainWindow::onLoginClicked() {
    QString username = usernameInput->text().trimmed();
    QString password = passwordInput->text();

    if (username.isEmpty() || password.isEmpty()) {
        loginStatusLabel->setStyleSheet("color: #f87171;");
        loginStatusLabel->setText("Please fill in all fields.");
        return;
    }
    if (isRegistering) {
        QString email = emailInput->text().trimmed();
        if (email.isEmpty()) {
            loginStatusLabel->setStyleSheet("color: #f87171;");
            loginStatusLabel->setText("Please enter your email.");
            return;
        }
        if (fileHandler->userExists(username)) {
            loginStatusLabel->setStyleSheet("color: #f87171;");
            loginStatusLabel->setText("Username already exists.");
            return;
        }
        currentUser = User(username, email, password);
        fileHandler->saveUser(currentUser);
        loginStatusLabel->setStyleSheet("color: #4ade80;");
        loginStatusLabel->setText("Account created! Signing in...");
    } else {
        if (!fileHandler->userExists(username)) {
            loginStatusLabel->setStyleSheet("color: #f87171;");
            loginStatusLabel->setText("User not found.");
            return;
        }
        currentUser = fileHandler->loadUser(username);
        if (!currentUser.validatePassword(password)) {
            loginStatusLabel->setStyleSheet("color: #f87171;");
            loginStatusLabel->setText("Incorrect password.");
            return;
        }
    }
    isLoggedIn = true;
    chatbot->startNewSession(username);
    stackedWidget->setCurrentWidget(chatPage);
    usernameInput->clear();
    passwordInput->clear();
    emailInput->clear();
    loginStatusLabel->clear();
}

void MainWindow::onRegisterClicked() {
    isRegistering = !isRegistering;
    if (isRegistering) {
        emailInput->show();
        loginBtn->setText("Create Account");
        registerBtn->setText("Already have an account? Sign in");
    } else {
        emailInput->hide();
        loginBtn->setText("Sign In");
        registerBtn->setText("Create an account");
    }
    loginStatusLabel->clear();
}

void MainWindow::onLogoutClicked() {
    isLoggedIn = false;
    clearChatDisplay();
    chatbot->clearHistory();
    stackedWidget->setCurrentWidget(loginPage);
}

void MainWindow::onNewChat() {
    clearChatDisplay();
    chatbot->clearHistory();
    chatbot->startNewSession(currentUser.getUsername());
    typingLabel->setText("");
    sendBtn->setEnabled(true);

}

void MainWindow::onShowHistory() {
    loadHistoryList();
    stackedWidget->setCurrentWidget(historyPage);
}

void MainWindow::onBackFromHistory() {
    stackedWidget->setCurrentWidget(chatPage);
}

void MainWindow::loadHistoryList() {
    while (historyLayout->count() > 1) {
        QLayoutItem* item = historyLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    QVector<QString> sessions = fileHandler->loadAllSessionIds();
    if (sessions.isEmpty()) {
        QLabel* empty = new QLabel("No chat history found.");
        empty->setStyleSheet("color:#6b74a0; font-size:14px;");
        empty->setAlignment(Qt::AlignCenter);
        historyLayout->insertWidget(0, empty);
        return;
    }

    for (const QString& sid : sessions) {
        QVector<Message> msgs = fileHandler->loadMessages(sid);
        if (msgs.isEmpty()) continue;

        QWidget* card = new QWidget();
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(
            "QWidget { background:#161b2e; border-radius:12px;"
            "border:1px solid #1e2640; }"
            "QWidget:hover { border:1px solid #7c83fd;"
            "background:#1a2040; }"
            );

        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 14, 16, 14);
        cl->setSpacing(6);

        QString preview = msgs.first().getContent().left(80);
        if (msgs.first().getContent().length() > 80) preview += "...";

        // Top row - session id + message count
        QHBoxLayout* topRow = new QHBoxLayout();
        QLabel* sessionLabel = new QLabel("🕐  Session: " + sid.right(12));
        sessionLabel->setStyleSheet(
            "color:#7c83fd; font-size:11px; font-weight:600;");

        QLabel* countLabel = new QLabel(
            QString::number(msgs.size()) + " messages");
        countLabel->setStyleSheet("color:#3d4466; font-size:11px;");

        topRow->addWidget(sessionLabel);
        topRow->addStretch();
        topRow->addWidget(countLabel);

        // Preview text
        QLabel* previewLabel = new QLabel(preview);
        previewLabel->setStyleSheet("color:#e8eaf0; font-size:13px;");
        previewLabel->setWordWrap(true);

        // Bottom row - date + open button
        QHBoxLayout* bottomRow = new QHBoxLayout();
        QLabel* dateLabel = new QLabel(
            msgs.last().getTimestamp().toString("MMM d, yyyy  hh:mm ap"));
        dateLabel->setStyleSheet("color:#3d4466; font-size:11px;");

        QPushButton* openBtn = new QPushButton("Open & Continue →");
        openBtn->setStyleSheet(
            "QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #7c83fd, stop:1 #a78bfa); border:none; border-radius:8px;"
            "color:white; font-size:12px; font-weight:600;"
            "padding: 6px 14px; }"
            "QPushButton:hover { background:#6366f1; }"
            );
        openBtn->setCursor(Qt::PointingHandCursor);

        // Capture sessionId for lambda
        QString capturedSid = sid;
        connect(openBtn, &QPushButton::clicked, this, [this, capturedSid]() {
            onSessionClicked(capturedSid);
        });

        bottomRow->addWidget(dateLabel);
        bottomRow->addStretch();
        bottomRow->addWidget(openBtn);

        cl->addLayout(topRow);
        cl->addWidget(previewLabel);
        cl->addLayout(bottomRow);

        historyLayout->insertWidget(historyLayout->count() - 1, card);
    }
}

void MainWindow::onMicButtonClicked() {
    if (!isRecording) {
        isRecording = true;
        voiceRecorder->startRecording();
    } else {
        isRecording = false;
        voiceRecorder->stopRecording();
    }
}

void MainWindow::onTranscriptionReady(const QString& text) {
    typingLabel->setText("");
    if (text.isEmpty()) {
        addMessageBubble("System", "🎤 No speech detected, try again.", false);
        return;
    }
    // Clear input and set voice text
    messageInput->clear();
    messageInput->setPlainText(text.trimmed());

    // Small delay phir send karo
    QTimer::singleShot(100, this, [this]() {
        onSendMessage();
    });
}

void MainWindow::onVoiceError(const QString& error) {
    typingLabel->setText("");
    addMessageBubble("System", "🎤 " + error, false);
    isRecording = false;
    micBtn->setText("🎤");
    micBtn->setStyleSheet(
        "background:#1e2640; border-radius:14px;"
        "color:#7c83fd; font-size:20px; border:none;");
}
void MainWindow::onSessionClicked(const QString& sessionId) {
    // Load session messages
    chatbot->loadSession(sessionId);

    // Clear display
    clearChatDisplay();

    // Load all messages and show them
    QVector<Message> msgs = fileHandler->loadMessages(sessionId);
    for (const Message& msg : msgs) {
        bool isUser = (msg.getSender() == "You");
        addMessageBubble(msg.getSender(), msg.getContent(), isUser);
    }

    // Switch to chat page
    stackedWidget->setCurrentWidget(chatPage);

    // Scroll to bottom
    scrollToBottom();
}
void MainWindow::onCopyMessage(const QString& text) {
    QApplication::clipboard()->setText(text);
    // Flash confirm
    botStatusLabel->setText("✅ Copied!");
    QTimer::singleShot(2000, this, [this]() {
        botStatusLabel->setText("● Online");
    });
}

void MainWindow::onToggleTheme() {
    isDarkMode = !isDarkMode;
    if (isDarkMode) {
        setStyleSheet(
            "QMainWindow, QWidget { background: #0f1117; color: #e8eaf0;"
            "font-family: 'Segoe UI', Arial; }"
            "#leftPanel { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 #1a1f35, stop:1 #0f1117); border-right: 1px solid #1e2235; }"
            "#brandName { font-size: 32px; font-weight: 700; color: #7c83fd; }"
            "#brandSub  { font-size: 13px; color: #8891b0; }"
            "#formCard { background: #161b2e; border-radius: 16px; border: 1px solid #1e2640; }"
            "#formTitle { font-size: 24px; font-weight: 700; color: #e8eaf0; }"
            "#formSubtitle { font-size: 13px; color: #6b74a0; }"
            "#formInput { background: #0f1117; border: 1.5px solid #1e2640;"
            "border-radius: 10px; padding: 0 14px; font-size: 14px; color: #e8eaf0; }"
            "#formInput:focus { border-color: #7c83fd; }"
            "#statusLabel { font-size: 12px; min-height: 16px; }"
            "#primaryBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #7c83fd, stop:1 #a78bfa); border: none; border-radius: 10px;"
            "color: white; font-size: 15px; font-weight: 600; }"
            "#primaryBtn:hover { background: #6366f1; }"
            "#linkBtn { background: transparent; border: none; color: #7c83fd; font-size: 13px; }"
            "#sidebar { background: #0d1121; border-right: 1px solid #1a2040; }"
            "#sideTitle { font-size: 16px; font-weight: 700; color: #7c83fd; padding: 4px 8px; }"
            "#sideBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #7c83fd, stop:1 #a78bfa); border: none; border-radius: 10px;"
            "color: white; font-size: 13px; font-weight:600; text-align:left; padding-left:14px; }"
            "#sideBtnGhost { background: transparent; border: 1px solid #1e2640;"
            "border-radius: 10px; color: #8891b0; font-size: 13px;"
            "text-align: left; padding-left: 14px; }"
            "#onlineStatus { color: #4ade80; font-size: 12px; padding: 4px 8px; }"
            "#logoutBtn { background: transparent; border: 1px solid #2d1f2f;"
            "#deleteBtn { background: transparent; border: 1px solid #2d1f2f;"
            "border-radius: 8px; color: #f87171; font-size: 13px; }"
            "#deleteBtn:hover { background: #2d1520; }"
            "border-radius: 8px; color: #f87171; font-size: 13px; }"
            "#chatArea { background: #0f1117; }"
            "#headerBar { background: #0d1121; border-bottom: 1px solid #1a2040; }"
            "#headerTitle { font-size: 16px; font-weight: 600; color: #e8eaf0; }"
            "#typingLabel { font-size: 12px; color: #7c83fd; font-style: italic; }"
            "#scrollArea { background: #0f1117; border: none; }"
            "#chatContainer { background: #0f1117; }"
            "#welcomeTitle { font-size: 22px; font-weight: 700; color: #e8eaf0; }"
            "#welcomeSub { font-size: 14px; color: #6b74a0; }"
            "#inputArea { background: #0d1121; border-top: 1px solid #1a2040; }"
            "#messageInput { background: #161b2e; border: 1.5px solid #1e2640;"
            "border-radius: 12px; padding: 10px 14px; font-size: 14px; color: #e8eaf0; }"
            "#sendBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 #7c83fd, stop:1 #a78bfa); border:none; border-radius:14px;"
            "color:white; font-size:20px; }"
            "#micBtn { background:#1e2640; border-radius:14px; color:#7c83fd; font-size:20px; border:none; }"
            "#themeBtn { background:#1e2640; border-radius:10px; border:none; font-size:16px; }"
            "QScrollBar:vertical { background:#0f1117; width:6px; border-radius:3px; }"
            "QScrollBar::handle:vertical { background:#2a2f4a; border-radius:3px; min-height:30px; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }"
            "QComboBox#modelSelector { background: #1e2640; border: 1px solid #2a2f4a;"
            "border-radius: 8px; color: #e8eaf0; padding: 0 10px; font-size: 12px; }"
            "QComboBox#modelSelector:hover { border-color: #7c83fd; }"
            "QComboBox QAbstractItemView { background: #161b2e; color: #e8eaf0;"
            "border: 1px solid #2a2f4a; selection-background-color: #7c83fd; }"
            );
    } else {
        setStyleSheet(
            "QMainWindow, QWidget { background: #f5f7ff; color: #1a1a2e;"
            "font-family: 'Segoe UI', Arial; }"
            "#leftPanel { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 #e8eaff, stop:1 #f5f7ff); border-right: 1px solid #d0d4ff; }"
            "#brandName { font-size: 32px; font-weight: 700; color: #6366f1; }"
            "#brandSub  { font-size: 13px; color: #6b74a0; }"
            "#formCard { background: white; border-radius: 16px; border: 1px solid #e0e4ff; }"
            "#formTitle { font-size: 24px; font-weight: 700; color: #1a1a2e; }"
            "#formSubtitle { font-size: 13px; color: #6b74a0; }"
            "#formInput { background: #f5f7ff; border: 1.5px solid #d0d4ff;"
            "border-radius: 10px; padding: 0 14px; font-size: 14px; color: #1a1a2e; }"
            "#formInput:focus { border-color: #6366f1; }"
            "#statusLabel { font-size: 12px; min-height: 16px; }"
            "#primaryBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #6366f1, stop:1 #a78bfa); border: none; border-radius: 10px;"
            "color: white; font-size: 15px; font-weight: 600; }"
            "#linkBtn { background: transparent; border: none; color: #6366f1; font-size: 13px; }"
            "#sidebar { background: #eef0ff; border-right: 1px solid #d0d4ff; }"
            "#sideTitle { font-size: 16px; font-weight: 700; color: #6366f1; padding: 4px 8px; }"
            "#sideBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
            "stop:0 #6366f1, stop:1 #a78bfa); border:none; border-radius:10px;"
            "color:white; font-size:13px; font-weight:600; text-align:left; padding-left:14px; }"
            "#sideBtnGhost { background: transparent; border: 1px solid #d0d4ff;"
            "border-radius: 10px; color: #6b74a0; font-size: 13px;"
            "text-align: left; padding-left: 14px; }"
            "#onlineStatus { color: #16a34a; font-size: 12px; padding: 4px 8px; }"
            "#logoutBtn { background: transparent; border: 1px solid #fecaca;"
            "#deleteBtn { background: transparent; border: 1px solid #fecaca;"
            "border-radius: 8px; color: #ef4444; font-size: 13px; }"
            "#deleteBtn:hover { background: #fff0f0; }"
            "border-radius: 8px; color: #ef4444; font-size: 13px; }"
            "#chatArea { background: #f5f7ff; }"
            "#headerBar { background: #eef0ff; border-bottom: 1px solid #d0d4ff; }"
            "#headerTitle { font-size: 16px; font-weight: 600; color: #1a1a2e; }"
            "#typingLabel { font-size: 12px; color: #6366f1; font-style: italic; }"
            "#scrollArea { background: #f5f7ff; border: none; }"
            "#chatContainer { background: #f5f7ff; }"
            "#welcomeTitle { font-size: 22px; font-weight: 700; color: #1a1a2e; }"
            "#welcomeSub { font-size: 14px; color: #6b74a0; }"
            "#inputArea { background: #eef0ff; border-top: 1px solid #d0d4ff; }"
            "#messageInput { background: white; border: 1.5px solid #d0d4ff;"
            "border-radius: 12px; padding: 10px 14px; font-size: 14px; color: #1a1a2e; }"
            "#sendBtn { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "stop:0 #6366f1, stop:1 #a78bfa); border:none; border-radius:14px;"
            "color:white; font-size:20px; }"
            "#micBtn { background:#e0e4ff; border-radius:14px; color:#6366f1; font-size:20px; border:none; }"
            "#themeBtn { background:#e0e4ff; border-radius:10px; border:none; font-size:16px; }"
            "QScrollBar:vertical { background:#f5f7ff; width:6px; border-radius:3px; }"
            "QScrollBar::handle:vertical { background:#c0c4f0; border-radius:3px; min-height:30px; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }"
            "QComboBox#modelSelector { background: #e8eaff; border: 1px solid #d0d4ff;"
            "border-radius: 8px; color: #1a1a2e; padding: 0 10px; font-size: 12px; }"
            "QComboBox#modelSelector:hover { border-color: #6366f1; }"
            "QComboBox QAbstractItemView { background: white; color: #1a1a2e;"
            "border: 1px solid #d0d4ff; selection-background-color: #6366f1; }"
            );
    }
}
bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj == messageInput) {
        // Enter to send
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return &&
                !(keyEvent->modifiers() & Qt::ShiftModifier)) {
                onSendMessage();
                return true;
            }
            // Ctrl+V — check clipboard for image
            if (keyEvent->key() == Qt::Key_V &&
                keyEvent->modifiers() & Qt::ControlModifier) {
                QClipboard* clipboard = QApplication::clipboard();
                const QMimeData* mimeData = clipboard->mimeData();
                if (mimeData->hasImage()) {
                    onPasteFromClipboard();
                    return true;
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
void MainWindow::onDeleteChat() {
    // Confirm dialog
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Delete Chat",
        "Are you sure you want to delete this chat?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QString sid = chatbot->getSessionId();
        fileHandler->deleteSession(sid);
        clearChatDisplay();
        chatbot->clearHistory();
        chatbot->startNewSession(currentUser.getUsername());
        botStatusLabel->setText("🗑️ Chat deleted!");
        QTimer::singleShot(2000, this, [this]() {
            botStatusLabel->setText("● Online");
        });
    }
}

void MainWindow::onShowEmojiPicker() {
    emojiPickerVisible = !emojiPickerVisible;
    if (emojiPickerVisible) {
        // Position above input area
        QPoint pos = emojiBtn->mapTo(chatPage,
                                     QPoint(0, -emojiPicker->sizeHint().height() - 10));
        emojiPicker->move(pos);
        emojiPicker->resize(340, 200);
        emojiPicker->raise();
        emojiPicker->show();
    } else {
        emojiPicker->hide();
    }
}

void MainWindow::onEmojiPicked(const QString& emoji) {
    messageInput->insertPlainText(emoji);
    emojiPicker->hide();
    emojiPickerVisible = false;
    messageInput->setFocus();
}
void MainWindow::onAttachFile() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select File or Image",
        "",
        "Images & Files (*.png *.jpg *.jpeg *.gif *.webp "
        "*.txt *.pdf *.cpp *.h *.py *.js *.html *.css *.json)"
        );

    if (filePath.isEmpty()) return;

    QString ext = QFileInfo(filePath).suffix().toLower();
    QStringList imageExts = {"png", "jpg", "jpeg", "gif", "webp"};

    attachedFilePath = filePath;
    attachedFileType = imageExts.contains(ext) ? "image" : "text";

    QString fileName = QFileInfo(filePath).fileName();
    QString icon     = (attachedFileType == "image") ? "🖼️" : "📄";

    attachmentPreview->setText(
        icon + " " + fileName + "  ✕"
        );
    attachmentPreview->show();
}

void MainWindow::onRemoveAttachment() {
    attachedFilePath.clear();
    attachedFileType.clear();
    attachmentPreview->hide();
    attachmentPreview->setText("");
}
void MainWindow::onPasteFromClipboard() {
    QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();

    if (mimeData->hasImage()) {
        // Image clipboard se lo
        QImage image = clipboard->image();
        if (image.isNull()) return;

        // Temp file mein save karo
        QString tempPath = QStandardPaths::writableLocation(
                               QStandardPaths::TempLocation)
                           + "/chatbot_paste_"
                           + QString::number(QDateTime::currentMSecsSinceEpoch())
                           + ".png";

        if (image.save(tempPath, "PNG")) {
            attachedFilePath = tempPath;
            attachedFileType = "image";
            attachmentPreview->setText("🖼️ Pasted Image  ✕");
            attachmentPreview->show();

            // Preview dikhao
            botStatusLabel->setText("🖼️ Image ready to send!");
            QTimer::singleShot(2000, this, [this]() {
                botStatusLabel->setText("● Online");
            });
        }
    } else if (mimeData->hasText()) {
        // Normal text paste — default behavior
        messageInput->insertPlainText(mimeData->text());
    }
}
void MainWindow::onShowProfile() {
    // Fill fields with current user data
    QLineEdit* userField = profilePage->findChild<QLineEdit*>("profileUsername");
    QLineEdit* emailField = profilePage->findChild<QLineEdit*>("profileEmail");
    QLineEdit* nameField = profilePage->findChild<QLineEdit*>("profileDisplayName");

    if (userField)  userField->setText(currentUser.getUsername());
    if (emailField) emailField->setText(currentUser.getUsername());
    if (nameField)  nameField->setPlaceholderText(currentUser.getUsername());

    stackedWidget->setCurrentWidget(profilePage);
}

void MainWindow::onSaveProfile() {
    QLabel* statusLbl = new QLabel("✅ Profile saved!");
    statusLbl->setStyleSheet("color: #4ade80; font-size: 13px;");
    botStatusLabel->setText("✅ Profile saved!");
    QTimer::singleShot(2000, this, [this]() {
        botStatusLabel->setText("● Online");
    });
    stackedWidget->setCurrentWidget(chatPage);
}

void MainWindow::onBackFromProfile() {
    stackedWidget->setCurrentWidget(chatPage);
}
void MainWindow::onSpeakMessage(const QString& text) {
    if (tts->state() == QTextToSpeech::Speaking) {
        tts->stop();
    }
    QString cleanText = text;
    cleanText.remove(QRegularExpression(
        "[^\\x{0000}-\\x{007F}\\x{0080}-\\x{00FF}"
        "\\x{0100}-\\x{017F}\\x{0180}-\\x{024F}]"));
    tts->say(cleanText);
}

void MainWindow::onToggleTTS() {
    ttsEnabled = !ttsEnabled;
    if (ttsEnabled) {
        ttsToggleBtn->setText("🔊");
        ttsToggleBtn->setStyleSheet(
            "background: #4ade80; border-radius: 10px;"
            "border: none; font-size: 16px;");
        botStatusLabel->setText("🔊 Voice ON");
    } else {
        ttsToggleBtn->setText("🔇");
        ttsToggleBtn->setStyleSheet(
            "background: #1e2640; border-radius: 10px;"
            "border: none; font-size: 16px;");
        if (tts->state() == QTextToSpeech::Speaking)
            tts->stop();
        botStatusLabel->setText("🔇 Voice OFF");
    }
    QTimer::singleShot(2000, this, [this]() {
        botStatusLabel->setText("● Online");
    });
}