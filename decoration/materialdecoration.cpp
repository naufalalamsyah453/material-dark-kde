// Material Dark - window decoration (KDecoration3)
// SPDX-License-Identifier: MIT
#include "materialdecoration.h"

#include "material.h"
#include "materialbutton.h"

#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/DecorationShadow>
#include <KDecoration3/ScaleHelpers>

#include <KPluginFactory>

#include <QFontMetricsF>
#include <QImage>
#include <QPainter>

#include <cmath>

K_PLUGIN_FACTORY_WITH_JSON(MaterialDecoFactory, "material.json", registerPlugin<Material::Decoration>(); registerPlugin<Material::Button>();)

namespace Material
{
using KDecoration3::ColorGroup;
using KDecoration3::ColorRole;

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
{
}

bool Decoration::init()
{
    auto s = settings();
    connect(s.get(), &KDecoration3::DecorationSettings::fontChanged, this, &Decoration::recalculate);
    connect(s.get(), &KDecoration3::DecorationSettings::spacingChanged, this, &Decoration::recalculate);
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::recalculate);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);

    auto w = window();
    // recalculate() juga memperbarui outline dan bayangan, jadi warna aktif/tidak aktif ikut berganti
    connect(w, &KDecoration3::DecoratedWindow::activeChanged, this, &Decoration::recalculate);
    connect(w, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::recalculate);
    connect(w, &KDecoration3::DecoratedWindow::maximizedHorizontallyChanged, this, &Decoration::recalculate);
    connect(w, &KDecoration3::DecoratedWindow::maximizedVerticallyChanged, this, &Decoration::recalculate);
    connect(w, &KDecoration3::DecoratedWindow::adjacentScreenEdgesChanged, this, &Decoration::recalculate);
    connect(w, &KDecoration3::DecoratedWindow::shadedChanged, this, &Decoration::recalculate);
    connect(w, &KDecoration3::DecoratedWindow::widthChanged, this, &Decoration::recalculate);
    connect(w, &KDecoration3::DecoratedWindow::nextScaleChanged, this, &Decoration::recalculate);
    connect(w, &KDecoration3::DecoratedWindow::captionChanged, this, [this]() {
        update(titleBar());
    });
    connect(w, &KDecoration3::DecoratedWindow::paletteChanged, this, &Decoration::recalculate);
    connect(this, &KDecoration3::Decoration::bordersChanged, this, &Decoration::updateButtonsGeometry);

    createButtons();
    recalculate();
    return true;
}

// ---------------------------------------------------------------------------
bool Decoration::isLeftEdge() const
{
    return window()->isMaximizedHorizontally() || window()->adjacentScreenEdges().testFlag(Qt::LeftEdge);
}
bool Decoration::isRightEdge() const
{
    return window()->isMaximizedHorizontally() || window()->adjacentScreenEdges().testFlag(Qt::RightEdge);
}
bool Decoration::isTopEdge() const
{
    return window()->isMaximizedVertically() || window()->adjacentScreenEdges().testFlag(Qt::TopEdge);
}
bool Decoration::isBottomEdge() const
{
    return window()->isMaximizedVertically() || window()->adjacentScreenEdges().testFlag(Qt::BottomEdge);
}

bool Decoration::isTiled() const
{
    return isLeftEdge() || isRightEdge() || isTopEdge() || isBottomEdge();
}

// jari-jari sudut jendela; 0 bila compositor tidak mendukung alpha
qreal Decoration::cornerRadius() const
{
    if (!settings()->isAlphaChannelSupported()) {
        return 0;
    }
    return KDecoration3::snapToPixelGrid(Radius, window()->nextScale());
}

int Decoration::buttonBox() const
{
    const int box = qRound(QFontMetricsF(settings()->font()).height()) + 4;
    return qMax<int>(ButtonSize, box + (box % 2));
}

qreal Decoration::titleHeight() const
{
    return buttonBox() + 2 * TitlePad;
}

qreal Decoration::topInset() const
{
    return isTopEdge() ? 0 : KDecoration3::snapToPixelGrid(Border, window()->nextScale());
}

// ---------------------------------------------------------------------------
// warna: ikut color scheme (WM section + palette), jadi aman di skema lain
// ---------------------------------------------------------------------------
QColor Decoration::outlineColor() const
{
    // "outline variant" M3: garis halus di sekeliling jendela
    const QPalette pal = window()->palette();
    const QColor text = pal.color(QPalette::WindowText);
    const QColor bg = pal.color(QPalette::Window);
    const qreal t = window()->isActive() ? 0.80 : 0.88;
    return QColor::fromRgbF(text.redF() * (1 - t) + bg.redF() * t, text.greenF() * (1 - t) + bg.greenF() * t, text.blueF() * (1 - t) + bg.blueF() * t);
}

QColor Decoration::titleBarColor() const
{
    return window()->color(window()->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::TitleBar);
}

QColor Decoration::fontColor() const
{
    return window()->color(window()->isActive() ? ColorGroup::Active : ColorGroup::Inactive, ColorRole::Foreground);
}

// ---------------------------------------------------------------------------
void Decoration::recalculate()
{
    const qreal sc = window()->nextScale();
    auto snap = [sc](qreal v) {
        return KDecoration3::snapToPixelGrid(v, sc);
    };

    const qreal left = isLeftEdge() ? 0 : snap(Border);
    const qreal right = isRightEdge() ? 0 : snap(Border);
    const qreal bottom = (isBottomEdge() && !window()->isShaded()) ? 0 : snap(Border);
    const qreal top = topInset() + snap(titleHeight()) + snap(Divider);
    setBorders(QMarginsF(left, top, right, bottom));

    // area tarik tambahan di luar jendela supaya bingkai tipis tetap mudah di-resize
    const qreal ext = snap(6);
    setResizeOnlyBorders(QMarginsF(isLeftEdge() ? 0 : ext, isTopEdge() ? 0 : ext, isRightEdge() ? 0 : ext, isBottomEdge() ? 0 : ext));

    setTitleBar(QRectF(left, topInset(), window()->width() - left - right, snap(titleHeight()) + snap(Divider)));

    // sudut membulat: compositor ikut memotong isi jendela (klien) dengan radius ini
    const qreal r = isTiled() ? 0 : cornerRadius();
    const KDecoration3::BorderRadius radius(r, r, r, r);
    setBorderRadius(radius);
    if (isTiled() || window()->isMaximized()) {
        setBorderOutline(KDecoration3::BorderOutline());
    } else {
        const qreal thickness = qMax<qreal>(KDecoration3::pixelSize(sc), snap(1));
        setBorderOutline(KDecoration3::BorderOutline(thickness, outlineColor(), radius));
    }
    // dekorasi punya sudut transparan, kecuali saat maksimal
    setOpaque(window()->isMaximized());

    updateButtonsGeometry();
    updateShadow();
    update();
}

void Decoration::createButtons()
{
    m_leftButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Left, this, &Button::create);
    m_rightButtons = new KDecoration3::DecorationButtonGroup(KDecoration3::DecorationButtonGroup::Position::Right, this, &Button::create);
}

void Decoration::updateButtonsGeometryDelayed()
{
    QMetaObject::invokeMethod(this, &Decoration::updateButtonsGeometry, Qt::QueuedConnection);
}

void Decoration::updateButtonsGeometry()
{
    if (!m_leftButtons || !m_rightButtons) {
        return;
    }
    const qreal sc = window()->nextScale();
    const int box = buttonBox();

    const auto all = m_leftButtons->buttons() + m_rightButtons->buttons();
    for (KDecoration3::DecorationButton *b : all) {
        b->setGeometry(QRectF(0, 0, box, box));
    }

    const qreal y = topInset() + KDecoration3::snapToPixelGrid((titleHeight() - box) / 2.0, sc);
    m_leftButtons->setSpacing(ButtonGap);
    m_rightButtons->setSpacing(ButtonGap);
    m_leftButtons->setPos(QPointF(borderLeft() + SideMargin, y));
    m_rightButtons->setPos(QPointF(size().width() - borderRight() - SideMargin - m_rightButtons->geometry().width(), y));
    update();
}

QRectF Decoration::captionRect() const
{
    const qreal x0 = m_leftButtons && !m_leftButtons->buttons().isEmpty() ? m_leftButtons->geometry().right() + SideMargin : borderLeft() + SideMargin + 4;
    const qreal x1 = m_rightButtons && !m_rightButtons->buttons().isEmpty() ? m_rightButtons->geometry().left() - SideMargin : size().width() - borderRight() - SideMargin;
    return QRectF(x0, topInset(), qMax<qreal>(0, x1 - x0), titleHeight());
}

// ---------------------------------------------------------------------------
void Decoration::paint(QPainter *painter, const QRectF &repaintRegion)
{
    const qreal w = size().width();
    const qreal h = size().height();
    const qreal r = cornerRadius();
    const bool round = r > 0 && !isTiled();
    const qreal barH = topInset() + titleHeight();

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // sudut transparan
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->fillRect(rect(), Qt::transparent);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

    // badan bingkai di bawah titlebar (bagian bawah + sisi), sudut bawah membulat
    painter->save();
    painter->setClipRect(QRectF(0, barH, w, h - barH), Qt::IntersectClip);
    painter->setBrush(window()->palette().color(QPalette::Window));
    if (round) {
        painter->drawRoundedRect(rect(), r, r);
    } else {
        painter->drawRect(rect());
    }
    painter->restore();

    // titlebar (surface tonal), sudut atas membulat
    const QRectF bar(0, 0, w, barH);
    painter->save();
    painter->setClipRect(bar, Qt::IntersectClip);
    painter->setBrush(titleBarColor());
    if (round) {
        painter->drawRoundedRect(bar, r, r);
        painter->drawRect(QRectF(0, barH - r, w, r)); // hilangkan pembulatan di sisi bawah titlebar
    } else {
        painter->drawRect(bar);
    }
    painter->restore();

    // garis pemisah halus di bawah titlebar
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->fillRect(QRectF(borderLeft(), barH, w - borderLeft() - borderRight(), Divider), outlineColor());

    // judul
    QFont font = settings()->font();
    painter->setFont(font);
    painter->setPen(fontColor());
    const QRectF cap = captionRect();
    const QString text = QFontMetricsF(font).elidedText(window()->caption(), Qt::ElideRight, cap.width());
    painter->drawText(cap, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, text);
    painter->restore();

    m_leftButtons->paint(painter, repaintRegion);
    m_rightButtons->paint(painter, repaintRegion);
}

// ---------------------------------------------------------------------------
// bayangan lembut (elevation): tekstur 9-patch dari fungsi jarak ke kotak membulat.
// Lubang berbentuk jendela (sudut membulat) dipotong supaya bayangan tidak tembus di sudut.
// ---------------------------------------------------------------------------
void Decoration::updateShadow()
{
    if (isTiled()) {
        setShadow(nullptr);
        return;
    }
    const int S = ShadowSize;
    const qreal R = qMax<qreal>(1, cornerRadius());
    const int Rc = qMax(1, qRound(R));
    const int N = 2 * (S + Rc) + 1;
    const int peak = window()->isActive() ? 120 : 55;

    QImage img(N, N, QImage::Format_ARGB32_Premultiplied);
    const qreal cx = S + Rc;
    const qreal cy = S + Rc + 3; // bayangan sedikit turun
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            const qreal d = std::hypot(std::abs(x - cx), std::abs(y - cy)) - Rc;
            const qreal f = qBound<qreal>(0.0, 1.0 - d / S, 1.0);
            img.setPixel(x, y, qRgba(0, 0, 0, qRound(peak * f * f))); // hitam: premultiplied == non-premultiplied
        }
    }

    // lubang jendela; masuk 2px ke bawah jendela untuk menghindari celah
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    const QRectF hole = QRectF(S, S, 2 * Rc + 1, 2 * Rc + 1).adjusted(2, 2, -2, -2);
    p.drawRoundedRect(hole, qMax<qreal>(0, R - 2), qMax<qreal>(0, R - 2));
    p.end();

    auto shadow = std::make_shared<KDecoration3::DecorationShadow>();
    shadow->setPadding(QMarginsF(S, S, S, S));
    shadow->setInnerShadowRect(QRectF(S + Rc, S + Rc, 1, 1));
    shadow->setShadow(img);
    setShadow(shadow);
}
}

#include "materialdecoration.moc"
