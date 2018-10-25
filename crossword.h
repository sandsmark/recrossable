#ifndef CROSSWORD_H
#define CROSSWORD_H

#include <QObject>
#include <QVector>
#include <QHash>

#include "cwc/grid.hh"

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
    QString hintAt(const int index);
    QString correctAt(const int index);

    QStringList hintsAcross();
    QStringList hintsDown();

    QString hintTextAt(int index);

private:
    void parseWordlist(const QString &filePath);
    void generateCrossword();

    QHash<QString, QString> m_hints;

    int m_rows = 0;
    int m_columns = 0;

    Grid m_grid;
    Answers m_answers;
};

#endif // CROSSWORD_H
