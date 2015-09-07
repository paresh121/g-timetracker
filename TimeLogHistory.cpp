#include <QThread>

#include <QLoggingCategory>

#include "TimeLogHistory.h"
#include "TimeLogHistoryWorker.h"

Q_LOGGING_CATEGORY(TIME_LOG_HISTORY_CATEGORY, "TimeLogHistory", QtInfoMsg)

TimeLogHistory::TimeLogHistory(QObject *parent) :
    QObject(parent),
    m_thread(new QThread(this)),
    m_worker(new TimeLogHistoryWorker(this)),
    m_size(0)
{
    connect(m_worker, SIGNAL(error(QString)),
            this, SIGNAL(error(QString)));
    connect(m_worker, SIGNAL(dataAvailable(QVector<TimeLogEntry>)),
            this, SIGNAL(dataAvailable(QVector<TimeLogEntry>)));
    connect(m_worker, SIGNAL(sizeChanged(qlonglong)),
            this, SLOT(workerSizeChanged(qlonglong)));
    connect(m_worker, SIGNAL(categoriesChanged(QSet<QString>)),
            this, SLOT(workerCategoriesChanged(QSet<QString>)));
}

TimeLogHistory::~TimeLogHistory()
{
    if (m_thread->isRunning()) {
        m_thread->quit();
    }
}

bool TimeLogHistory::init()
{
    return m_worker->init();
}

void TimeLogHistory::madeAsync()
{
    if (m_worker->thread() != thread()) {
        return;
    }

    m_thread->setParent(0);
    m_worker->setParent(0);

    connect (m_thread, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
    connect (m_worker, SIGNAL(destroyed()), m_thread, SLOT(deleteLater()));

    m_worker->moveToThread(m_thread);

    m_thread->start();
}

qlonglong TimeLogHistory::size() const
{
    return m_size;
}

QSet<QString> TimeLogHistory::categories() const
{
    return m_categories;
}

void TimeLogHistory::insert(const TimeLogEntry &data)
{
    QMetaObject::invokeMethod(m_worker, "insert", Qt::AutoConnection, Q_ARG(TimeLogEntry, data));
}

bool TimeLogHistory::insert(const QVector<TimeLogEntry> &data)
{
    bool result;
    QMetaObject::invokeMethod(m_worker, "insert", Qt::AutoConnection, Q_RETURN_ARG(bool, result),
                              Q_ARG(QVector<TimeLogEntry>, data));

    return result;
}

void TimeLogHistory::remove(const QUuid &uuid)
{
    QMetaObject::invokeMethod(m_worker, "remove", Qt::AutoConnection, Q_ARG(QUuid, uuid));
}

void TimeLogHistory::edit(const TimeLogEntry &data)
{
    QMetaObject::invokeMethod(m_worker, "edit", Qt::AutoConnection, Q_ARG(TimeLogEntry, data));
}

void TimeLogHistory::getHistory(const QDateTime &begin, const QDateTime &end, const QString &category) const
{
    QMetaObject::invokeMethod(m_worker, "getHistory", Qt::AutoConnection, Q_ARG(QDateTime, begin),
                              Q_ARG(QDateTime, end), Q_ARG(QString, category));
}

void TimeLogHistory::getHistory(const uint limit, const QDateTime &until) const
{
    QMetaObject::invokeMethod(m_worker, "getHistory", Qt::AutoConnection, Q_ARG(uint, limit),
                              Q_ARG(QDateTime, until));
}

void TimeLogHistory::workerSizeChanged(qlonglong size)
{
    m_size = size;
}

void TimeLogHistory::workerCategoriesChanged(QSet<QString> categories)
{
    m_categories.swap(categories);
}