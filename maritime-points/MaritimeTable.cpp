#include "MaritimeTable.h"

#include <ArcGISRuntimeEnvironment.h>
#include <Point.h>

#include <exception>

#include <QDebug>
#include <QFile>
#include <QQmlProperty>
#include <QQmlComponent>
#include <QQmlEngine>

#include <marnav/nmea/nmea.hpp>
#include <marnav/nmea/ais_helper.hpp>
#include <marnav/nmea/sentence.hpp>
#include <marnav/nmea/vdm.hpp>
#include <marnav/ais/ais.hpp>
#include <marnav/ais/message_01.hpp>


namespace EA = Esri::ArcGISRuntime;


using ScopedSentence = std::unique_ptr<marnav::nmea::sentence>;

namespace
{

std::vector<ScopedSentence> collectSentences(QFile& mar,
                                             std::function<void (std::vector<ScopedSentence>)> callback)
{
    std::vector<ScopedSentence> sentences;
    while (!mar.atEnd())
    {
        QString rawSentence = mar.readLine();
        rawSentence.chop(1); // Remove newline.
        try
        {
            auto sentence = marnav::nmea::make_sentence(rawSentence.toStdString());

            // we process only VDM messages in this demo
            if (sentence->id() == marnav::nmea::sentence_id::VDM)
            {
                const auto vdm = marnav::nmea::sentence_cast<marnav::nmea::vdm>(sentence.get());
                const auto n_fragments = vdm->get_n_fragments();
                const auto fragment = vdm->get_fragment();

                // collect sentence
                sentences.push_back(std::move(sentence));

                // check if all necessary sentences have arrived and if they have,
                // process the AIS message.
                if (fragment == n_fragments)
                {
                    std::vector<ScopedSentence> collected;
                    std::swap(collected, sentences);
                    callback(std::move(collected));
                }
            }
        }
        catch (const std::exception& e)
        {
            qWarning() << QString("Could not parse: %1. Skipping").arg(rawSentence);
            qWarning() << e.what();
        }
    }
    return sentences;
}

}


MaritimeTableFactory::MaritimeTableFactory(QQmlEngine* appEngine, QObject* parent) :
    QObject(parent),
    m_appEngine(appEngine)
{

}

void MaritimeTableFactory::loadAIS(QObject* table, const QString& path)
{
    QFile mar(path);
    if (mar.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QVariantList features;
        collectSentences(
            mar,
            [this, table, &features](std::vector<ScopedSentence> sentences)
            {
                auto payload = marnav::nmea::collect_payload(sentences.begin(), sentences.end());
                auto message = marnav::ais::make_message(payload);

                if (marnav::ais::message_id::position_report_class_a != message->type())
                {
                    // If this is static ship data then skip. We want position data.
                    return;
                }

                qDebug() << QString("Created message");

                auto msg = marnav::ais::message_cast<marnav::ais::message_01>(message);
                auto lon = msg->get_longitude();
                auto lat = msg->get_latitude();

                if (lon.has_value() && lat.has_value())
                {
                    qDebug() << QString("Creating point at %1, %2.").arg(lon->get()).arg(lat->get());

                    QObject* f = nullptr;
                    QMetaObject::invokeMethod(table, "createFeature", QGenericReturnArgument("QmlFeature*", &f));

                    QQmlComponent component(m_appEngine, f);
                    component.setData(QString("import Esri.ArcGISRuntime 100.2; Point { x: %1; y: %2 }")
                                      .arg(lon->get()).arg(lat->get()).toLatin1(),
                                      QUrl());

                    if( component.status() != QQmlComponent::Ready)
                    {
                        qCritical() << "Not ready!";
                        if (component.status() == QQmlComponent::Error )
                            qCritical() << "Error:" << component.errorString();
                    }
                    else
                    {
                        auto p = component.create();
                        bool success = QQmlProperty(f, "geometry").write(QVariant::fromValue(p));
                        qDebug() << QString("  > Applied point: %1.").arg(success);

                        features << QVariant::fromValue(f);
                    }
                }
                else
                {
                    qDebug() << QString("Message is missing lat and/or lon component. Skipping");
                }
            }
        );
        QMetaObject::invokeMethod(table, "addFeatures", QGenericArgument("QVariantList", &features));
    }
    mar.close();
}
