#include <QJsonArray>
#include <QHash>
#include <QByteArray>
#include <stdexcept>
#include <QString>

#include "audiobook.h"
#include "globaljson.h"
#include "backend.h"
#include "filesizerequest.h"

AudioBook::AudioBook(QString path, QObject *parent) :
    AudioBookInfo(path, parent)
{
    QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();
    QAudioFormat format = device.preferredFormat();
    m_path = path;
    QDir d(path);
    d.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    d.setSorting(QDir::Name);
    QStringList filters;
    filters << "*.mp3";
    d.setNameFilters(filters);
    QFileInfoList list = d.entryInfoList();
    for(int i = 0; i < list.size(); ++i) {
        const QFileInfo &fi = list.at(i);
        QString sname = fi.fileName();
        AudioBookFile abf;
        abf.name = sname;
        m_data.append(abf);
    }
    emit indexChanged();
    readJson();
    requestUpdateSizes();
}

int AudioBook::size() {
    return m_data.size();
}

int AudioBook::index()
{
    return m_index;
}

void AudioBook::readJson() {
    QJsonObject bookObject = GlobalJSON::getInstance()->getBook(m_path);
    if(bookObject.contains("index") && bookObject["index"].isDouble()) {
        setIndex(bookObject["index"].toInt());
    }
    AudioBookInfo::readJson();
}

void AudioBook::writeJson() {
    QJsonObject bookObject;
    bookObject["url"] = m_path;
    bookObject["index"] = m_index;
    bookObject["file_pos"] = m_currentFilePos;
    bookObject["size_before"] = m_sizeBefore;
    bookObject["size_total"] = m_sizeTotal;
    GlobalJSON::getInstance()->setBook(bookObject, m_path);
}

bool AudioBook::setIndex(int i) {
    if(m_index == i) {
        return false;
    }
    if(i >=0 && i < m_data.size()) {
        m_index = i;
        updateSizes();
        emit indexChanged();
        return true;
    }
    else return false;
}

QString AudioBook::getCurrentFilePath() {
    return getFilePath(m_index);
}

const AudioBookFile &AudioBook::fileAt(int i){
    if(i < 0 || i >= m_data.size()) {
        qDebug() << "index overflow" << i;
        return m_data.at(0);
    }
    return m_data.at(i);
}

const AudioBookFile &AudioBook::current()
{
    return fileAt(m_index);
}

QString AudioBook::getFilePath(int i) {
    QDir d;
    QString path = m_path + d.separator() + fileAt(i).name;
    QFile f(path);
    if(f.exists()) {
        return path;
    } else {
        return QString("");
    }
}

bool AudioBook::setNext() {
    if(m_index >= m_data.size()) return false;
    ++m_index;
    emit indexChanged();
    return true;
}

bool AudioBook::setPrevious() {
    if(m_index <= 0) return false;
    --m_index;
    emit indexChanged();
    return true;
}

QString AudioBook::path() const {
    return m_path;
}

QString AudioBook::folderName() const {
    QFileInfo fi(m_path);
    return fi.fileName();
}

void AudioBook::requestUpdateSizes() { // move to thread

    return;
    // This shit is very unstable
    for(int i = 0; i < m_data.size(); ++i) {
        QString path = getFilePath(i);
        QThread* thr = new QThread();
        FileSizeRequest *fsr = new FileSizeRequest(i, path);
        fsr->moveToThread(thr);
        //TODO: error handling
        //connect(fsr, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
        connect(thr, SIGNAL(started()), fsr, SLOT(process()));
        connect(fsr, SIGNAL(finished()), thr, SLOT(quit()));
        connect(fsr, SIGNAL(finished()), fsr, SLOT(deleteLater()));
        connect(thr, SIGNAL(finished()), thr, SLOT(deleteLater()));
        connect(fsr, SIGNAL(metaDataChanged(int, qint64)),
                this, SLOT(requestResult(int, qint64)));
        thr->start();
    }
}

void AudioBook::updateSizes() { // when ready
    if(sizeReady == true) {
        int res = 0;
        for(int i = 0; i < m_data.size(); ++i) {
            if(i == m_index)
            {
                m_sizeBefore = res;
                m_sizeCurrentFile = fileAt(i).size;
            }
            res += fileAt(i).size;
        }
        m_sizeTotal = res;
    }
}

void AudioBook::requestResult(int index, qint64 vsize)
{
    m_data[index].size = vsize;
    bool flag = true;
    for(int i = 0; i < m_data.size(); ++i) {
        if(m_data.at(i).size <= 0) flag = false;
    }
    if(flag)
    {
        qDebug() << "All sizes ready";
        sizeReady = true;
        updateSizes();
    }
}

qint64 AudioBook::getCurrentFilePos() {
    return m_currentFilePos;
}

void AudioBook::setCurrentFilePos(qint64 pos) {
    m_currentFilePos = pos;
}

qreal AudioBook::progressOf(int i)
{
    if(i < m_index) return 1.0;
    if(i < m_index) return progressCurrentFile();
    if(i > m_index) return 0.0;
    return 0.0;
}

qreal AudioBook::progressCurrentFile() {
    if(m_sizeCurrentFile > 0) {
        return (qreal)(m_currentFilePos) / m_sizeCurrentFile;
    }
    else return 0;
}

int AudioBook::progressInt() {
    return m_sizeBefore + m_currentFilePos;
}

int AudioBook::sizeTotal() {
    return m_sizeTotal;
}
