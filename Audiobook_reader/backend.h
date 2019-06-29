#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QFileSystemModel>
#include <QDebug>
#include <QUrl>
#include <QMediaPlayer>
#include <QTimer>
#include <QSettings>
#include <QJsonObject>
#include <QMutex>

#include "player.h"

class BookFile;
class AudioBook;

typedef QPair<QString, qint64> FileTimePair;

class AudioBookFileList : public QAbstractListModel {
     Q_OBJECT
public:
    enum BookFileRoles {
        TextRole = Qt::UserRole +1
    };
    AudioBookFileList(AudioBook *audiobook);
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void setAudiobook(AudioBook* value) {
        m_audiobook = value;
    }

private:

    AudioBook* m_audiobook;
};

class BackEnd : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentFolderUrl READ currentFolderUrl NOTIFY currentFolderChanged)
    Q_PROPERTY(QString currentFolder READ currentFolder WRITE setCurrentFolder NOTIFY currentFolderChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(qreal fileProgress READ fileProgress NOTIFY fileProgressChanged)
    Q_PROPERTY(QString rootPath READ rootPath WRITE setRootPath NOTIFY rootPathChanged)
    Q_PROPERTY(QString rootPathUrl READ rootPathUrl WRITE setRootPathUrl NOTIFY rootPathChanged)
    Q_PROPERTY(QList<QObject*> bookFileList READ bookFileList NOTIFY bookFileListChanged)
    Q_PROPERTY(int currentBookFileIndex READ currentBookFileIndex WRITE setCurrentBookFileIndex NOTIFY currentBookFileIndexChanged)
    Q_PROPERTY(AudioBookFileList* audioBookFileList READ audioBookFileList NOTIFY audioBookFileListChanged)

public:
    explicit BackEnd(QObject *parent = nullptr);

    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine) {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);
        return (QObject*)getInstance();
    }

    static BackEnd* getInstance() {
        if(!m_instance) {
            m_instance = new BackEnd();
        }
        return m_instance;
    }

    QString currentFolder() {
        return m_currentFolder;
    }

    QString currentFolderUrl() {
        QUrl u = QUrl::fromLocalFile(currentFolder());
        QString s = u.url();
        return s;
    }

    void setCurrentFolder(QString value);

    QString FileName() {return m_currentFileName;}

    QUrl getFileUrl() {
        QDir d;
        QUrl u = QUrl::fromLocalFile(m_currentFolder + d.separator() + m_currentFileName);
        qDebug() << "getFileUrl" << u ;
        return u;
    }

    bool isPlaying() {
        //return m_player.state() == QMediaPlayer::PlayingState;
        return m_player.state() == QMediaPlayer::PlayingState;
    }

    qreal fileProgress() {
        if(m_player.duration() != 0) {
            qreal r;
            r = (qreal)m_player.position() / m_player.duration();
            return r;
        } else {
            return 0;
        }
    }

    QString rootPath() {
        return m_rootPath;
    }

    void setRootPath(QString value) {
        m_rootPath = value;
        m_settings.setValue("rootPath", value);
        emit rootPathChanged();
    }

    QString rootPathUrl() {
        QUrl u;
        QDir d(m_rootPath);
        u = u.fromLocalFile(d.absolutePath());
        QString s = u.toString();
        return s;
    }

    void setRootPathUrl(QString url) {
        QUrl u = QUrl(url);
        setRootPath(u.toLocalFile());
    }

    QList<QObject*> bookFileList() {
        return m_bookFileList;
    }

    int currentBookFileIndex() {
        return m_currentBookFile;
    }

    void setCurrentBookFileIndex(int value);

    AudioBookFileList* audioBookFileList() {
        if(m_audiobook == nullptr) {
            return nullptr;
        }
        return m_audiobookFileList;
    }

    AudioBook* audioBook() {
        return m_audiobook;
    }

    void openAudioBook(QString folder);

signals:

    void currentFolderChanged();
    void isPlayingChanged();
    void fileProgressChanged();
    void rootPathChanged();
    void bookFileListChanged();
    void currentBookFileIndexChanged();
    void audioBookFileListChanged();

public slots:

    void openBookFolder(QString value);
    void closeBookFolder();
    void play();
    void stop();
    void next();
    void prev();
    void jumpForeward();
    void jumpBack();

    void speedUp();
    void positionChangedSlot(int pos);
    void isPlayingSlot(QMediaPlayer::State state);
    void autoSave();
    void autoLoad();
    void onFinishedSlot();

private:
    
    bool loadJson();
    bool saveJson();

    void writeBookJson(QJsonObject &bookObject);
    void readBookJson(const QJsonObject &bookObject);
    void writeFileJson(QJsonObject &fileObject);
    void readFileJson(const QJsonObject &fileObject);
    void writeBookArrayJson();
    void readBookArrayJson();

    void writeCurrentJson();
    void readCurrentJson(QString &savedFolder, QString &savedFile);

    void setupAutosave();

    static BackEnd* m_instance;
    QString m_currentFolder;
    QString m_currentFileName;
    qint64 m_currentPos;
    Player m_player;
    QTimer* m_autoSaveTimer;
    QSettings m_settings;
    QJsonObject m_jsonRoot;
    QString m_jsonFileName = "save.json";
    QMutex muxJson;
    QString m_rootPath;
    QList<QObject*> m_bookFileList;
    int m_currentBookFile;
    AudioBook* m_audiobook;
    AudioBookFileList* m_audiobookFileList;
    void setMedia();
    void closeAudioBook();
};
