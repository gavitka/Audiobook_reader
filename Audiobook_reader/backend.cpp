#include "QDebug"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>

#include "backend.h"
#include "globaljson.h"
#include "audiobook.h"

BackEnd* BackEnd::m_instance = nullptr;

void BackEnd::setupAutosave() {
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, SIGNAL(timeout()), this, SLOT(autoSave()));
    m_autoSaveTimer->setInterval(5000);
    m_autoSaveTimer->start();
}

BackEnd::BackEnd(QObject *parent) :
    QObject(parent),
    m_currentPos(0),
    m_player(this),
    m_settings("Gavitka software", "Audiobook reader"),
    m_audiobook(nullptr),
    m_playlist(nullptr)
{
    m_tempoValues.append(0.5);
    m_tempoValues.append(0.6);
    m_tempoValues.append(0.7);
    m_tempoValues.append(0.8);
    m_tempoValues.append(0.9);
    m_tempoValues.append(1.0);
    m_tempoValues.append(1.1);
    m_tempoValues.append(1.2);
    m_tempoValues.append(1.3);
    m_tempoValues.append(1.4);
    m_tempoValues.append(1.5);
    m_tempoValues.append(1.6);
    m_tempoValues.append(1.7);
    m_tempoValues.append(1.8);
    m_tempoValues.append(1.9);
    m_tempoValues.append(2.0);
    m_tempo = 5;

    QObject::connect (&m_player, SIGNAL(stateChanged(QMediaPlayer::State)),
                      this, SLOT(isPlayingSlot(QMediaPlayer::State)));

    QObject::connect (&m_player, SIGNAL(stateChanged(QMediaPlayer::State)),
                      this, SLOT(isPlayingSlot(QMediaPlayer::State)));

    QObject::connect (&m_player, SIGNAL(positionChanged(int)),
                      this, SLOT(positionChangedSlot(int)));

    //    QObject::connect (&m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
    //                      this, SLOT(mediaStatusSlot(QMediaPlayer::MediaStatus)));
    // TODO: implement


    QObject::connect (&m_player, SIGNAL(onFinished()),
                      this, SLOT(onFinishedSlot()));

    setRootPath(m_settings.value("rootPath").toString());

    GlobalJSON::getInstance()->loadJSON();
    autoLoad();

    setupAutosave();
}

void BackEnd::setCurrentAudiobookIndex(int i) { // for loading audiobook from list
    QString path = m_audioBookList->at(i)->getPath();
    openAudioBook(path);
}

void BackEnd::increaseTempo() {
    int index = m_tempo + 1;
    if(index >= m_tempoValues.size()) return;
    m_tempo = index;
    m_player.setTempo(m_tempoValues.at(m_tempo));
    emit tempoChanged();
}

void BackEnd::decreaseTempo() {
    int index = m_tempo - 1;
    if(index < 0) return;
    m_tempo = index;
    m_player.setTempo(m_tempoValues.at(m_tempo));
    emit tempoChanged();
}

void BackEnd::openAudioBook(QString path) {
    QDir d(path);
    if(d.exists() && !path.isEmpty()) {
        QString path = d.absolutePath();
        if(m_audiobook != nullptr) {
            if(m_audiobook->getPath() == path) {
                return;
            } else {
                closeAudioBook();
            }
        }
        m_audiobook = new AudioBook(path);
        m_audiobook->readFileSizes();
        m_playlist = new AudioBookFileList(m_audiobook);


        QObject::connect (this, SIGNAL(playlistItemChanged()),
                          m_playlist, SLOT(playlistItemChanged()));
        QObject::connect (this, SIGNAL(playlistChanged()),
                          m_playlist, SLOT(playlistChanged()));

        updatePlayer();
        emit currentFolderChanged();
        emit playlistChanged();
        emit playListIndexChanged();
    }
}

void BackEnd::closeAudioBook() {
    if(isPlaying()) m_player.stop();

    delete m_audiobook;
    m_audiobook = nullptr;

    delete m_playlist;
    m_playlist = nullptr;

    emit isPlayingChanged();
}

void BackEnd::updatePlayer() { // updates player with appropriate file
    if(m_audiobook != nullptr) {
        QString path = m_audiobook->getCurrentFilePath();
        if(QFile(path).exists()) {
            if(isPlaying()) m_player.stop();
            emit isPlayingChanged();

            m_player.setFile(path);
            int pos = m_audiobook->getCurrentFile()->pos();
            m_player.setPosition(pos);
            m_player.setVolume(25);

            qDebug() << "set file " << path;
            qDebug() << "pos" << pos;
        }
    }
}

void BackEnd::setPlaylistIndex(int value) {
    if(m_audiobook->setCurrentFileIdex(value)) {
        updatePlayer();
        emit playListIndexChanged();
        emit playlistChanged();
    }
}

void BackEnd::setPlaylistFile(QString value) {
    if(m_audiobook != nullptr && m_audiobook->setCurrentFileName(value)) {
        updatePlayer();
        emit playListIndexChanged();
        emit playlistChanged();
    }
}

void BackEnd::closeBookFolder() {
    autoSave();
    m_player.stop();
    emit isPlayingChanged();
}

void BackEnd::play() {
    m_player.play();
}

void BackEnd::stop() {
    m_player.pause();
    emit isPlayingChanged();
}

void BackEnd::next() {
    m_audiobook->setNext();
    updatePlayer();
    play();
    emit playListIndexChanged();
    emit playlistChanged();
}

void BackEnd::prev() {
    m_audiobook->setPrevious();
    updatePlayer();
    play();
    emit playListIndexChanged();
    emit playlistChanged();
}

void BackEnd::jumpForeward(int sec) {
    m_player.jump(sec * 1000);
    // TODO: Try to get this to work
    //    while (m_player.jump(10000) != 0) {
    //        if(m_audiobook->setNext() == false) break;
    //    }
}

void BackEnd::jumpBack(int sec) {
    m_player.jump(-sec * 1000);
}

void BackEnd::speedUp() {
    m_player.setTempo(1.3f);
}

void BackEnd::positionChangedSlot(int pos){
    Q_UNUSED(pos)
    emit fileProgressChanged();
    emit playlistItemChanged();
}

void BackEnd::isPlayingSlot(QMediaPlayer::State state) {
    Q_UNUSED(state)
    emit isPlayingChanged();
}

void BackEnd::autoSave() {
    if(m_audiobook != nullptr) {
        m_audiobook->setCurrentPos(m_player.position());
        m_audiobook->writeJson();
    }
    writeCurrentJson();
    GlobalJSON::getInstance()->saveJSON();
}

void BackEnd::autoLoad() {
    QString savedFolder;
    QString savedFile;
    readCurrentJson(savedFolder, savedFile);
    openAudioBook(savedFolder);
    setPlaylistFile(savedFile);
}

void BackEnd::onFinishedSlot() {
    m_audiobook->setNext();
    updatePlayer();
    play();
}

void BackEnd::readCurrentJson(QString &savedFolder, QString &savedFile) {
    QJsonObject jsonRoot;
    jsonRoot = GlobalJSON::getInstance()->getRoot();
    if(jsonRoot.contains("currentBook") && jsonRoot["currentBook"].isDouble()) {
        savedFolder = jsonRoot["currentBook"].toString();
    }
    if(jsonRoot.contains("currentFile") && jsonRoot["currentFile"].isDouble()) {
        savedFile = jsonRoot["currentFile"].toString();
    }
}

void BackEnd::writeCurrentJson() {
    muxJson.lock();
    QJsonObject& jsonRoot = GlobalJSON::getInstance()->getRoot();
    jsonRoot["currentBook"] = m_audiobook->getPath();
    jsonRoot["currentFile"] = m_audiobook->getCurrentFile()->fileName();
    muxJson.unlock();
}

/////////// Audio book list ////////////////////////////////////

AudioBookFileList::AudioBookFileList(AudioBook *audiobook) :
    m_audiobook(audiobook)
{ }

QHash<int, QByteArray> AudioBookFileList::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[TextRole] = "text";
    roles[progressRole] = "progress";
    return roles;
}

int AudioBookFileList::rowCount(const QModelIndex &parent) const {
    if(m_audiobook == nullptr) return 0;
    return m_audiobook->fileCount();
}

QVariant AudioBookFileList::data(const QModelIndex &index, int role) const {
    if(m_audiobook == nullptr) return "";
    switch(role) {
    case TextRole:
        return m_audiobook->fileAt(index.row())->fileName();
    case progressRole:
        return m_audiobook->progressOf(index.row());
    }
    return false;
}

void AudioBookFileList::playlistItemChanged() {
    QModelIndex topLeft = createIndex(m_audiobook->index(), 0);
    QModelIndex bottomRight = createIndex(m_audiobook->index(), 0);
    QVector<int> roleVector;
    roleVector << BookFileRoles::progressRole;
    emit dataChanged(topLeft, bottomRight, roleVector);
}


void AudioBookFileList::playlistChanged() {
    QModelIndex topLeft = createIndex(0, 0);
    QModelIndex bottomRight = createIndex(m_audiobook->size()-1, 0);
    QVector<int> roleVector;
    roleVector << BookFileRoles::progressRole;
    emit dataChanged(topLeft, bottomRight, roleVector);
}
