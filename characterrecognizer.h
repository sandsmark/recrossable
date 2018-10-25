#ifndef CHARACTERRECOGNIZER_H
#define CHARACTERRECOGNIZER_H

#include <QObject>

struct Network;

class CharacterRecognizer : public QObject
{
    Q_OBJECT
public:
    static CharacterRecognizer *instance();

    QString recognize(const QImage &image);

signals:

public slots:
private:
    explicit CharacterRecognizer(QObject *parent = nullptr);
    Network *m_net;
};

#endif // CHARACTERRECOGNIZER_H
