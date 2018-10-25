#include "crossword.h"


#include "cwc/dict.hh"
#include "cwc/letterdict.hh"
#include "cwc/cwc.hh"

#include <random>
#include <algorithm>

#include <QDebug>
#include <QFile>
#include <QElapsedTimer>

Crossword::Crossword(QObject *parent) : QObject(parent),
    m_rows(5),
    m_columns(4),
    m_grid(m_columns, m_rows)
{
    parseWordlist(":/nyt.tsv");
    generateCrossword();
}

QString Crossword::hintAt(const int index)
{
    if (m_answers.celltoclue.find(index) == m_answers.celltoclue.end()) {
        return QString();
    }

    int num = m_answers.celltoclue[index];
    QString ret = QString::number(num);

    if (m_answers.across.clues.count(num)) {
        ret += "→";
    } else {
        ret += "↓";
    }

    return ret;

}

QString Crossword::correctAt(const int index)
{
    if (m_grid.cellno(index).isoutside()) {
        return "x";
    }
    return QString::fromStdString(m_grid.cellno(index).tostring());

}

QStringList Crossword::hintsAcross()
{
    QStringList ret;
    for (const int &num : m_answers.across.clues) {
        QString clue = QString::number(num) + "→: ";
        clue += m_hints[QString::fromStdString(m_answers.across.cluetoanswer[num])];
        ret.append(clue);
    }

    return ret;
}

QStringList Crossword::hintsDown()
{
    QStringList ret;
    for (const int &num : m_answers.down.clues) {
        QString clue = QString::number(num) + "→: ";
        clue += m_hints[QString::fromStdString(m_answers.down.cluetoanswer[num])];
        ret.append(clue);
    }

    return ret;
}

QString Crossword::hintTextAt(int index)
{
    if (m_answers.across.celltoanswer.find(index) != m_answers.across.celltoanswer.end()) {
        return m_hints[QString::fromStdString(m_answers.across.celltoanswer[index])];
    }
    if (m_answers.down.celltoanswer.find(index) != m_answers.down.celltoanswer.end()) {
        return m_hints[QString::fromStdString(m_answers.down.celltoanswer[index])];
    }
    return QString();
}

void Crossword::parseWordlist(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load" << filePath;
        return;
    }
    QElapsedTimer timer;
    timer.start();

    while (!file.atEnd()) {
        QString line = file.readLine();
        QStringList parts = line.split('\t', QString::SkipEmptyParts);

        QString hint = parts.first().trimmed();
        QString word = parts.last().trimmed().toLower();
        if (hint.startsWith('"') && hint.endsWith('"')) {
            hint.remove(0, 1);
            hint.chop(1);
        }
        if (hint.isEmpty() || word.isEmpty())  {
            qWarning() << "Invalid line";
            continue;
        }

        m_hints[word] = hint;
    }
    qDebug() << "loaded word list in" << timer.elapsed() << "ms";

    if (m_hints.isEmpty()) {
        qWarning() << "No words in file";
        return;
    }

//    std::random_device rd;
//    std::mt19937 g(rd());

//    std::shuffle(m_words.begin(), m_words.end(), g);

//    qDebug() << "Loaded" << m_words.count() << "words";

//    for (int i=0; i<10; i++) {
//        qDebug() << m_words[i].word << m_words[i].hint;
//    }
}

void Crossword::generateCrossword()
{
    LetterDict dict;

    dict.wl = new WordList;
    for (const QString &word : m_hints.keys()) {
        dict.wl->addWord(word.toStdString());
    }
    int nwords = dict.wl->numwords();
    qDebug() << "Added" << nwords << "words";
    for (int i=0; i<nwords; i++) {
        dict.addword((*dict.wl)[i], i);
    }
    qDebug() << m_grid.numopen() << "open cells";
    FloodWalker walker(m_grid);
    SmartBacktracker backtracker(m_grid);

    Compiler compiler(m_grid, walker, backtracker, dict);
//    compiler.verbose = true;
//    compiler.showsteps = true;
    compiler.compile();
    m_answers = m_grid.getanswers();
    m_grid.dump_ascii(std::cout, &m_answers);
    m_answers.dump(std::cout);
    qDebug() << "compile completed";

    delete dict.wl;
}
