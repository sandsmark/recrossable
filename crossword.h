#ifndef CROSSWORD_H
#define CROSSWORD_H

#include <QObject>
#include <QVector>

class Crossword : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int rows READ rows NOTIFY rowsChanged)
    Q_PROPERTY(int columns READ columns NOTIFY columnsChanged)

public:
    explicit Crossword(QObject *parent = nullptr);

    int rows() const { return m_rows; }
    int columns() const { return m_columns; }

signals:
    void columnsChanged();
    void rowsChanged();

public slots:
    QString hintAt(const int row, const int column);

private:
    void parseWordlist(const QString &filePath);
    void generateCrossword();

    struct Word {
        QString word;
        QString hint;
    };

    QVector<Word> m_words;

    int m_rows = 0;
    int m_columns = 0;

};

#endif // CROSSWORD_H
