#include "crossword.h"

#include <random>
#include <algorithm>

#include <QDebug>
#include <QFile>

Crossword::Crossword(QObject *parent) : QObject(parent),
    m_rows(4),
    m_columns(4)
{
    parseWordlist("/home/sandsmark/src/remarcrossword/nyt.tsv");

}

QString Crossword::hintAt(const int row, const int column)
{
    int index = row * m_columns + column;
    if (index > m_words.size()) {
        return QString();
    }
    return m_words[index].hint;
}

void Crossword::parseWordlist(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load" << filePath;
        return;
    }

    while (!file.atEnd()) {
        QString line = file.readLine();
        QStringList parts = line.split('\t', QString::SkipEmptyParts);

        Word word;
        word.hint = parts.first().simplified();
        word.word = parts.last().simplified();
        if (word.hint.startsWith('"') && word.hint.endsWith('"')) {
            word.hint.remove(0, 1);
            word.hint.chop(1);
        }
        if (word.hint.isEmpty() || word.word.isEmpty())  {
            qWarning() << "Invalid line";
            continue;
        }

        m_words.append(word);
    }
    if (m_words.isEmpty()) {
        qWarning() << "No words in file";
        return;
    }

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(m_words.begin(), m_words.end(), g);

    qDebug() << "Loaded" << m_words.count() << "words";

    for (int i=0; i<10; i++) {
        qDebug() << m_words[i].word << m_words[i].hint;
    }
}
