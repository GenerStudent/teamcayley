#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QObject>
#include <QTime>
#include <QBasicTimer>
#include <QDebug>
#include <QEasingCurve>
#include <QGeoCoordinate>
#include <QtPositioning/private/qgeoprojection_p.h>

#define ANIMATION_DURATION 4000

//! [PlaneController1]
class PlaneController: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate position READ position WRITE setPosition NOTIFY positionChanged)
//! [PlaneController1]
    //! [C++Pilot1]
    Q_PROPERTY(QGeoCoordinate from READ from WRITE setFrom NOTIFY fromChanged)
    Q_PROPERTY(QGeoCoordinate to READ to WRITE setTo NOTIFY toChanged)
    //! [C++Pilot1]

public:
    PlaneController()
    {
        easingCurve.setType(QEasingCurve::InOutQuad);
        easingCurve.setPeriod(ANIMATION_DURATION);
    }

    void setFrom(const QGeoCoordinate& from)
    {
        fromCoordinate = from;
    }

    QGeoCoordinate from() const
    {
        return fromCoordinate;
    }

    void setTo(const QGeoCoordinate& to)
    {
        toCoordinate = to;
    }

    QGeoCoordinate to() const
    {
        return toCoordinate;
    }

    void setPosition(const QGeoCoordinate &c) {
        if (currentPosition == c)
            return;

        currentPosition = c;
        emit positionChanged();
    }

    QGeoCoordinate position() const
    {
        return currentPosition;
    }

    Q_INVOKABLE bool isFlying() const {
        return timer.isActive();
    }

//! [C++Pilot2]
public slots:
    void startFlight()
    {
        if (timer.isActive())
            return;

        startTime = QTime::currentTime();
        finishTime = startTime.addMSecs(ANIMATION_DURATION);

        timer.start(15, this);
        emit departed();
    }
//! [C++Pilot2]

    void swapDestinations() {
        if (currentPosition == toCoordinate) {
            // swap destinations
            toCoordinate = fromCoordinate;
            fromCoordinate = currentPosition;
        }
    }

signals:
    void positionChanged();
    void arrived();
    void departed();
    void toChanged();
    void fromChanged();

protected:
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE
    {
        if (!event)
            return;

        if (event->timerId() == timer.timerId())
            updatePosition();
        else
            QObject::timerEvent(event);
    }

private:
    //! [C++Pilot3]
    void updatePosition()
    {
        // simple progress animation
        qreal progress;
        QTime current = QTime::currentTime();
        if (current >= finishTime) {
            progress = 1.0;
            timer.stop();
        } else {
            progress = ((qreal)startTime.msecsTo(current) / ANIMATION_DURATION);
        }

        setPosition(QGeoProjection::coordinateInterpolation(
                          fromCoordinate, toCoordinate, easingCurve.valueForProgress(progress)));

        if (!timer.isActive())
            emit arrived();
    }
    //! [C++Pilot3]

private:
    QGeoCoordinate currentPosition;
    QGeoCoordinate fromCoordinate, toCoordinate;
    QBasicTimer timer;
    QTime startTime, finishTime;
    QEasingCurve easingCurve;
//! [PlaneController2]
    // ...
};
//! [PlaneController2]

//! [PlaneControllerMain]
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    PlaneController losAngeles2NY;
    PlaneController newYork2Houston;


    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("losAngeles2NY", &losAngeles2NY);
    engine.rootContext()->setContextProperty("newYork2Houston", &newYork2Houston);

    engine.load(QUrl(QStringLiteral("qrc:/planespotter.qml")));

    return app.exec();
}
//! [PlaneControllerMain]

#include "main.moc"
