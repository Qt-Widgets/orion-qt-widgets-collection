#include "OriTools.h"

#include <QUrl>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QUrlQuery>
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#include <QRandomGenerator>
#endif

//--------------------------------------------------------------------------------------------------

namespace Ori {
namespace Tools {

QString getParamStr(const QUrl& url, const QString& name)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    return QUrlQuery(url).queryItemValue(name);
#else
    return url.queryItemValue(name);
#endif
}

int getParamInt(const QUrl& url, const QString& name)
{
    return getParamStr(url, name).toInt();
}

} // namespace Tools
} // namespace Ori

//--------------------------------------------------------------------------------------------------

namespace Ori {
namespace Color {

QString formatHtml(const QColor& c)
{
    return QString("#%1%2%3")
            .arg(QString::number(c.red(), 16), 2, '0')
            .arg(QString::number(c.green(), 16), 2, '0')
            .arg(QString::number(c.blue(), 16), 2, '0');
}

QString formatRgb(const QColor& c)
{
    int R, G, B;
    c.getRgb(&R, &G, &B);
    return QString("rgb(%1, %2, %3)").arg(R).arg(G).arg(B);
}

QString formatHsl(const QColor& c)
{
    int H, S, L;
    c.getHsl(&H, &S, &L);
    return QString("hsl(%1, %2, %3)").arg(H).arg(S).arg(L);
}

QColor blend(const QColor& color1, const QColor& color2, qreal r)
{
    return QColor(color1.red() * (1-r) + color2.red()*r,
                  color1.green() * (1-r) + color2.green()*r,
                  color1.blue() * (1-r) + color2.blue()*r,
                  255);
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
namespace {
inline int qrand() {
    return QRandomGenerator::global()->generate();
}
}
#endif

QColor random()
{
    return QColor(qrand() % 255, qrand() % 255, qrand() % 255);
}

QColor random(int darkenThan, int lightenThan, int minDistance)
{
    static int prevH = 0, prevS = 0, prevL = 0;
    int H, S, L, tryCount = 0;
    while (tryCount++ < 100)
    {
        H = qrand() % 255;
        S = qrand() % 255;
        L = qrand() % 255;
        if (L < lightenThan) continue;
        if (L > darkenThan) continue;
        if (qAbs(H - prevH) < minDistance &&
            qAbs(S - prevS) < minDistance &&
            qAbs(L - prevL) < minDistance)
            continue;
        break;
    }
    prevH = H, prevS = S, prevL = L;
    return QColor::fromHsl(H, S, L);
}

} // namespace Color
} // namespace Ori

//--------------------------------------------------------------------------------------------------
