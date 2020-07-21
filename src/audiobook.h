#pragma once

#include <QAbstractListModel>
#include <QDir>
#include <QList>
#include <QObject>
#include <QUrl>

#include "player.h"
#include "audiobookinfo.h"
#include "backend.h"

struct AudioBookFile  {
    qint64 size = 0;
    QString name = "";
};

class AudioBook : public AudioBookInfo
{

    Q_OBJECT

public:

    AudioBook(QString path, QObject* parent = nullptr);
    void openBook();
    void closeBook();
    void readJson();
    void writeJson();
    bool setIndex(int i);
    const AudioBookFile* getCurrentFile();
    QString getCurrentFilePath();
    int size();
    int index();
    const AudioBookFile &fileAt(int i);
    const AudioBookFile &current();
    bool setNext();
    bool setPrevious();
    QString path() const;
    QString folderName() const;
    qreal progressCurrentFile();
    qreal progressOf(int i);
    int progressInt();
    int sizeTotal();
    void readFileSizes();
    qint64 getCurrentFilePos();
    void setCurrentFilePos(qint64 pos, qint64 dur);
    void requestUpdateSizes();
    void updateSizes();

    void setSize(int index, qint64 value);
    QString filePathAt(int i);

public slots:

    void sizeReadySlot();

signals:

    void indexChanged();

private:


    QString m_name;
    QString m_path;

    QVector<AudioBookFile>* m_data;

    bool sizeReady = false;
    QThread* m_thread;
};











