#pragma once

#include <QString>
#include <QObject>

class AudioBookInfo : public QObject
{

    Q_OBJECT

public:

    AudioBookInfo(QString path, QObject* parent);

    QString name();
    qreal progress();
    QString path();
    void readJson();

protected:


    qint64 m_currentFilePos = 0;
    qint64 m_sizeBefore = 0;
    qint64 m_sizeCurrentFile = 0;
    qint64 m_sizeTotal = 0;

    int m_index;

private:

    QString m_path;
    QString m_name;
};
