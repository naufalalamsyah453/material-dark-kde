// Material Dark - window decoration (KDecoration3)
// SPDX-License-Identifier: MIT
#include "materialbutton.h"

#include "material.h"

#include <KDecoration3/DecoratedWindow>

#include <QPainter>
#include <QPolygonF>

namespace Material
{
using KDecoration3::DecorationButtonType;

Button::Button(DecorationButtonType type, Decoration *decoration, QObject *parent)
    : KDecoration3::DecorationButton(type, decoration, parent)
{
    connect(decoration->window(), &KDecoration3::DecoratedWindow::iconChanged, this, [this]() {
        update();
    });
}

Button::Button(QObject *parent, const QVariantList &args)
    : Button(args.at(0).value<DecorationButtonType>(), args.at(1).value<Decoration *>(), parent)
{
    setGeometry(QRectF(0, 0, ButtonSize, ButtonSize));
}

Button *Button::create(DecorationButtonType type, KDecoration3::Decoration *decoration, QObject *parent)
{
    auto d = qobject_cast<Decoration *>(decoration);
    if (!d) {
        return nullptr;
    }
    auto *b = new Button(type, d, parent);
    const auto c = d->window();
    switch (type) {
    case DecorationButtonType::Close:
        b->setVisible(c->isCloseable());
        connect(c, &KDecoration3::DecoratedWindow::closeableChanged, b, &Button::setVisible);
        break;
    case DecorationButtonType::Maximize:
        b->setVisible(c->isMaximizeable());
        connect(c, &KDecoration3::DecoratedWindow::maximizeableChanged, b, &Button::setVisible);
        break;
    case DecorationButtonType::Minimize:
        b->setVisible(c->isMinimizeable());
        connect(c, &KDecoration3::DecoratedWindow::minimizeableChanged, b, &Button::setVisible);
        break;
    case DecorationButtonType::ContextHelp:
        b->setVisible(c->providesContextHelp());
        connect(c, &KDecoration3::DecoratedWindow::providesContextHelpChanged, b, &Button::setVisible);
        break;
    case DecorationButtonType::Shade:
        b->setVisible(c->isShadeable());
        connect(c, &KDecoration3::DecoratedWindow::shadeableChanged, b, &Button::setVisible);
        break;
    default:
        break;
    }
    return b;
}

void Button::paint(QPainter *painter, const QRectF &repaintRegion)
{
    Q_UNUSED(repaintRegion)
    auto d = qobject_cast<Decoration *>(decoration());
    if (!d) {
        return;
    }
    if (type() == DecorationButtonType::Spacer) {
        return;
    }
    if (type() == DecorationButtonType::Menu) {
        d->window()->icon().paint(painter, geometry().toRect());
        return;
    }

    const bool active = d->window()->isActive();
    const bool down = isPressed();
    const bool hover = isHovered();
    const bool isClose = type() == DecorationButtonType::Close;

    const QRectF g = geometry();
    QColor glyph = d->fontColor();

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // state layer lingkaran: hover 8%, tekan 12%; tombol close memakai warna error.
    // Tombol maximize yang checked (jendela maksimal) TIDAK memakai lingkaran tetap:
    // ikon restore sudah cukup, dan lingkaran permanen membuat tombol tampak
    // "terpotong" oleh tombol di sebelahnya.
    // Inset 2px: lingkaran tidak menyentuh tepi sel tombol, jadi tidak terlihat
    // terpotong oleh tombol tetangga atau sudut titlebar.
    if (hover || down) {
        const QRectF layer = g.adjusted(2, 2, -2, -2);
        if (isClose && (hover || down)) {
            painter->setBrush(Error);
            painter->drawEllipse(layer);
            glyph = OnError;
        } else {
            QColor layerColor = d->fontColor();
            layerColor.setAlphaF(down ? 0.16 : (hover ? 0.10 : 0.08));
            painter->setBrush(layerColor);
            painter->drawEllipse(layer);
        }
    }
    if (!active) {
        glyph.setAlphaF(0.6);
    }
    drawGlyph(painter, g, glyph);
    painter->restore();
}

// simbol: garis 1.5px dengan ujung membulat
void Button::drawGlyph(QPainter *p, const QRectF &inner, const QColor &color) const
{
    const QPointF c = inner.center();
    const qreal s = qMax<qreal>(3.5, inner.width() * 0.19); // setengah sisi simbol
    QPen pen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    switch (type()) {
    case DecorationButtonType::Close:
        p->drawLine(QPointF(c.x() - s, c.y() - s), QPointF(c.x() + s, c.y() + s));
        p->drawLine(QPointF(c.x() + s, c.y() - s), QPointF(c.x() - s, c.y() + s));
        break;
    case DecorationButtonType::Maximize:
        if (isChecked()) {
            p->drawRoundedRect(QRectF(c.x() - s + 2, c.y() - s - 1, 2 * s - 2, 2 * s - 2), 1.5, 1.5);
            p->drawRoundedRect(QRectF(c.x() - s - 1, c.y() - s + 2, 2 * s - 2, 2 * s - 2), 1.5, 1.5);
        } else {
            p->drawRoundedRect(QRectF(c.x() - s, c.y() - s, 2 * s, 2 * s), 1.5, 1.5);
        }
        break;
    case DecorationButtonType::Minimize:
        p->drawLine(QPointF(c.x() - s, c.y()), QPointF(c.x() + s, c.y()));
        break;
    case DecorationButtonType::OnAllDesktops:
        if (isChecked()) {
            p->setPen(Qt::NoPen);
            p->setBrush(color);
        }
        p->drawEllipse(c, s - 0.5, s - 0.5);
        break;
    case DecorationButtonType::Shade:
        p->drawLine(QPointF(c.x() - s, c.y() - s), QPointF(c.x() + s, c.y() - s));
        if (isChecked()) {
            p->drawPolyline(QPolygonF({QPointF(c.x() - s, c.y()), QPointF(c.x(), c.y() + s), QPointF(c.x() + s, c.y())}));
        } else {
            p->drawPolyline(QPolygonF({QPointF(c.x() - s, c.y() + s), QPointF(c.x(), c.y()), QPointF(c.x() + s, c.y() + s)}));
        }
        break;
    case DecorationButtonType::KeepAbove:
        p->drawPolyline(QPolygonF({QPointF(c.x() - s, c.y() + s / 2), QPointF(c.x(), c.y() - s / 2), QPointF(c.x() + s, c.y() + s / 2)}));
        if (isChecked()) {
            p->drawLine(QPointF(c.x() - s, c.y() + s), QPointF(c.x() + s, c.y() + s));
        }
        break;
    case DecorationButtonType::KeepBelow:
        p->drawPolyline(QPolygonF({QPointF(c.x() - s, c.y() - s / 2), QPointF(c.x(), c.y() + s / 2), QPointF(c.x() + s, c.y() - s / 2)}));
        if (isChecked()) {
            p->drawLine(QPointF(c.x() - s, c.y() - s), QPointF(c.x() + s, c.y() - s));
        }
        break;
    case DecorationButtonType::ApplicationMenu:
        for (int i = -1; i <= 1; ++i) {
            p->drawLine(QPointF(c.x() - s, c.y() + i * s * 0.8), QPointF(c.x() + s, c.y() + i * s * 0.8));
        }
        break;
    case DecorationButtonType::ContextHelp: {
        QFont f = p->font();
        f.setBold(true);
        f.setPixelSize(qRound(inner.height() * 0.6));
        p->setFont(f);
        p->drawText(inner, Qt::AlignCenter, QStringLiteral("?"));
        break;
    }
    default:
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawEllipse(c, 2, 2);
        break;
    }
}
}
