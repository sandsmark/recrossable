#include "crossword.h"


#include "cwc/dict.hh"
#include "cwc/letterdict.hh"
#include "cwc/cwc.hh"

#include <random>
#include <algorithm>
#include <sstream>

#include <QDebug>
#include <QFile>
#include <QElapsedTimer>
#include <QDir>

Crossword::Crossword(QObject *parent) : QObject(parent),
    m_grid(nullptr)
{
    parseWordlist(":/nyt.tsv");
    generateCrossword();
}

QString Crossword::hintAt(const int index)
{
    if (!m_answers) {
        return {};
    }

    if (m_answers->celltoclue.find(index) == m_answers->celltoclue.end()) {
        return QString();
    }

    int num = m_answers->celltoclue[index];
    QString ret = QString::number(num);

    if (m_answers->across.clues.count(num)) {
        ret += "→";
    } else {
        ret += "↓";
    }

    return ret;

}

QString Crossword::correctAt(const int index)
{
    if (!m_grid) {
        return {};
    }

    if (m_grid->cellno(index).isoutside()) {
        return "x";
    }
    return QString::fromStdString(m_grid->cellno(index).tostring()).toUpper();

}

bool Crossword::isOpen(const int index)
{
    if (!m_grid) {
        return false;
    }

    if (m_grid->cellno(index).isoutside()) {
        return false;
    }
    return true;
}

QStringList Crossword::hintsAcross()
{
    if (!m_answers) {
        return {};
    }

    QStringList ret;
    for (const int &num : m_answers->across.clues) {
        QString clue = QString::number(num) + "→: ";
        clue += m_hints[QString::fromStdString(m_answers->across.cluetoanswer[num])];
        ret.append(clue);
    }

    return ret;
}

QStringList Crossword::hintsDown()
{
    if (!m_answers) {
        return {};
    }
    QStringList ret;
    for (const int &num : m_answers->down.clues) {
        QString clue = QString::number(num) + "↓: ";
        clue += m_hints[QString::fromStdString(m_answers->down.cluetoanswer[num])];
        ret.append(clue);
    }

    return ret;
}

QString Crossword::hintTextAt(int index)
{
    if (!m_answers) {
        return {};
    }
    if (m_answers->across.celltoanswer.find(index) != m_answers->across.celltoanswer.end()) {
        return m_hints[QString::fromStdString(m_answers->across.celltoanswer[index])];
    }
    if (m_answers->down.celltoanswer.find(index) != m_answers->down.celltoanswer.end()) {
        return m_hints[QString::fromStdString(m_answers->down.celltoanswer[index])];
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
        hint.replace("\"\"", "\"");
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
}

void Crossword::generateCrossword()
{
    QElapsedTimer timer;
    timer.start();

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

    if (m_grid) {
        qWarning() << "========================";
        delete m_grid;
    }
    m_grid = new Grid;
    QStringList patterns = QDir(":/patterns/").entryList(QDir::Files);
    if (!patterns.isEmpty()) {
        QString patternName = patterns[qrand() % patterns.size()];
        patternName = "ginsberg";
        qDebug() << "Loading pattern" << patternName;
        QFile patternFile(":/patterns/" + patternName);
        std::string pattern;
        if (patternFile.open(QIODevice::ReadOnly)) {
            pattern = patternFile.readAll().toStdString();
        }
//        qDebug().noquote() << QByteArray::fromStdString(pattern);
        if (!pattern.empty()) {
            std::istringstream istr(pattern);
            m_grid->load_template(istr);
        } else {
            qWarning() << "failed to load pattern";
        }
    }

    qDebug() << m_grid->numopen() << "open cells";
    FloodWalker walker(*m_grid);
    SmartBacktracker backtracker(*m_grid);

    Compiler compiler(*m_grid, walker, backtracker, dict);
    if (!compiler.compile()) {
        qWarning() << "Failed to compile";
    }
    m_answers = new Answers;
    *m_answers = m_grid->getanswers();
    m_grid->dump_ascii(std::cout, m_answers);
    m_answers->dump(std::cout);


    delete dict.wl;

    if (m_columns != m_grid->w) {
        m_columns = m_grid->w;
        emit columnsChanged();
    }

    if (m_rows != m_grid->h) {
        m_rows = m_grid->h;
        emit rowsChanged();
    }
    qDebug() << "Generated crossword in" << timer.elapsed() << "ms";
}
