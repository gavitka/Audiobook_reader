#include "audiobookmodel.h"
#include "backend.h"

AudioBookModel::AudioBookModel()
{
}

QHash<int, QByteArray> AudioBookModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TextRole] = "text";
    return roles;
}

int AudioBookModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || m_audiobook == nullptr)
        return 0;

    return m_audiobook->size();
}

QVariant AudioBookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_audiobook)
        return QVariant();
    int i = index.row();
    const AudioBookFile& item = m_audiobook->fileAt(i);
    switch (role) {
    case TextRole:
        return QVariant(item.name);
    }

    return QVariant();
}

AudioBook *AudioBookModel::audioBook()
{
    return m_audiobook;
}

void AudioBookModel::setAudioBook(AudioBook *value) {
    if(value == nullptr) return;
    beginResetModel();
    m_audiobook = value;
    endResetModel();
    emit indexChanged();
    QObject::connect(m_audiobook, &AudioBook::indexChanged, this, &AudioBookModel::indexChangedSlot);
}

int AudioBookModel::index() const
{
    if(m_audiobook == nullptr) return 0;

    return m_audiobook->index();
}

void AudioBookModel::indexChangedSlot()
{
    emit indexChanged();
}

void AudioBookModel::setIndex(int value)
{
    if(m_audiobook == nullptr) return;

    m_audiobook->setIndex(value);
    indexChangedSlot();
    emit indexChanged();
}
