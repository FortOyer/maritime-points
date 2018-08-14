#ifndef MARITIMETABLE_H
#define MARITIMETABLE_H

#include <QObject>

class QQmlEngine;

class MaritimeTableFactory : public QObject
{
    Q_OBJECT
public:
    explicit MaritimeTableFactory(QQmlEngine* appEngine, QObject* parent = nullptr);

    Q_INVOKABLE void loadAIS(QObject* table, const QString& path);

private:
    QQmlEngine* m_appEngine;
};

#endif // MARITIMETABLE_H
