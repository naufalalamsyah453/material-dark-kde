// Material Dark - Qt widget style (application style)
// SPDX-License-Identifier: MIT
#include "materialstyle.h"

#include <QAbstractButton>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QSettings>
#include <QSlider>
#include <QStandardPaths>
#include <QStyleOption>
#include <QTabBar>

#include <climits>

namespace
{
constexpr int R_FIELD = 10;  // radius text field / combobox / spinbox (M3: small)
constexpr int R_CARD = 16;   // radius group box (M3: medium)
constexpr int R_TOOL = 10;   // radius tool button
constexpr int ARROW_W = 28;  // lebar area panah combobox
constexpr int SPIN_W = 22;   // lebar tombol spinbox
constexpr int FIELD_H = 32;  // tinggi minimum field
constexpr int BTN_H = 36;    // tinggi minimum tombol

// Menu (M3 menu: container tonal, item dengan state layer, tanpa isi primary penuh)
constexpr int R_MENU = 12;         // radius container menu
constexpr int MENU_PAD_X = 12;     // padding kiri / kanan di dalam item
constexpr int MENU_ITEM_H = 32;    // tinggi minimum item
constexpr int MENU_CHECK_W = 24;   // kolom tanda centang
constexpr int MENU_ICON_GAP = 8;   // jarak ikon ke teks
constexpr int MENU_TAB_GAP = 24;   // jarak teks ke pintasan keyboard
constexpr int MENU_ARROW_W = 20;   // area panah submenu
constexpr int MENU_SEP_H = 9;      // tinggi pemisah (1px garis + 4px di atas dan bawah)

QColor mix(const QColor &a, const QColor &b, qreal t)
{
    return QColor::fromRgbF(a.redF() * (1 - t) + b.redF() * t, a.greenF() * (1 - t) + b.greenF() * t, a.blueF() * (1 - t) + b.blueF() * t);
}

QColor withAlpha(QColor c, qreal a)
{
    c.setAlphaF(a);
    return c;
}

qreal luma(const QColor &c)
{
    return 0.299 * c.redF() + 0.587 * c.greenF() + 0.114 * c.blueF();
}

// "outline" M3: garis field / checkbox; "outline variant": garis halus (frame, divider)
QColor outline(const QPalette &pal)
{
    return mix(pal.color(QPalette::WindowText), pal.color(QPalette::Window), 0.6);
}

QColor outlineVariant(const QPalette &pal)
{
    return mix(pal.color(QPalette::WindowText), pal.color(QPalette::Window), 0.8);
}

// permukaan menu: window + 6% warna teks (surface container)
QColor menuSurface(const QPalette &pal)
{
    return mix(pal.color(QPalette::Window), pal.color(QPalette::WindowText), 0.06);
}

// menu yang latarnya tembus (sudut bulat); combobox popup dan lainnya tetap persegi
bool isRoundMenu(const QWidget *w)
{
    return qobject_cast<const QMenu *>(w) && w->testAttribute(Qt::WA_TranslucentBackground);
}

bool parseRgb(const QVariant &v, QColor *out)
{
    QStringList parts = v.toStringList(); // QSettings memecah nilai "r,g,b"
    if (parts.size() == 1) {
        parts = parts.first().split(QLatin1Char(','));
    }
    if (parts.size() < 3) {
        return false;
    }
    bool ok1, ok2, ok3;
    const int r = parts[0].trimmed().toInt(&ok1);
    const int g = parts[1].trimmed().toInt(&ok2);
    const int b = parts[2].trimmed().toInt(&ok3);
    if (!(ok1 && ok2 && ok3)) {
        return false;
    }
    *out = QColor(r, g, b);
    return true;
}

bool isEnabled(const QStyleOption *o)
{
    return o->state & QStyle::State_Enabled;
}

void fillRound(QPainter *p, const QRectF &r, const QColor &c, qreal radius)
{
    p->setPen(Qt::NoPen);
    p->setBrush(c);
    p->drawRoundedRect(r, radius, radius);
}

void strokeRound(QPainter *p, const QRectF &r, const QColor &c, qreal w, qreal radius)
{
    p->setPen(QPen(c, w));
    p->setBrush(Qt::NoBrush);
    p->drawRoundedRect(r.adjusted(w / 2, w / 2, -w / 2, -w / 2), radius, radius);
}

// state layer M3: hover 8%, fokus 10%, tekan 10%
qreal stateOpacity(bool down, bool hover, bool focus)
{
    if (down) {
        return 0.10;
    }
    if (hover) {
        return 0.08;
    }
    if (focus) {
        return 0.10;
    }
    return 0.0;
}

bool isInteractive(const QWidget *w)
{
    return qobject_cast<const QAbstractButton *>(w) || qobject_cast<const QComboBox *>(w) || qobject_cast<const QAbstractSpinBox *>(w)
        || qobject_cast<const QScrollBar *>(w) || qobject_cast<const QSlider *>(w) || qobject_cast<const QTabBar *>(w);
}
} // namespace

MaterialStyle::MaterialStyle()
    : QProxyStyle(QStringLiteral("Fusion"))
    , m_primary(242, 184, 198)
    , m_onPrimary(43, 18, 25)
{
    loadColors();
}

// Ambil warna aksen dari kdeglobals supaya style ikut color scheme yang aktif.
// MATERIALDARK_COLOR_SCHEME menunjuk file .colors lain; dipakai oleh alat preview
// agar bisa merender tema tanpa mengubah color scheme sesi.
void MaterialStyle::loadColors()
{
    const QByteArray overridePath = qgetenv("MATERIALDARK_COLOR_SCHEME");
    const QString path = overridePath.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/kdeglobals")
        : QString::fromLocal8Bit(overridePath);
    QSettings s(path, QSettings::IniFormat);
    QColor c;
    if (parseRgb(s.value(QStringLiteral("Colors:Window/DecorationFocus")), &c)) {
        m_primary = c;
    }
    // teks di atas warna primary: versi sangat gelap dari primary, atau putih bila primary gelap
    m_onPrimary = luma(m_primary) > 0.55 ? mix(m_primary, Qt::black, 0.82) : QColor(255, 255, 255);
}

void MaterialStyle::polish(QPalette &palette)
{
    QProxyStyle::polish(palette);
    loadColors();
}

void MaterialStyle::polish(QWidget *w)
{
    QProxyStyle::polish(w);
    if (isInteractive(w)) {
        w->setAttribute(Qt::WA_Hover, true);
    }
    if (qobject_cast<QMenu *>(w)) {
        // latar tembus supaya sudut menu bisa dibulatkan (perlu compositing, ada di KWin)
        w->setAttribute(Qt::WA_TranslucentBackground, true);
    }
}

void MaterialStyle::unpolish(QWidget *w)
{
    if (isInteractive(w)) {
        w->setAttribute(Qt::WA_Hover, false);
    }
    if (qobject_cast<QMenu *>(w)) {
        w->setAttribute(Qt::WA_TranslucentBackground, false);
    }
    QProxyStyle::unpolish(w);
}

// ---------------------------------------------------------------------------
// Tombol: filled (default), tonal (normal), text (flat). Bentuk pill.
// ---------------------------------------------------------------------------
void MaterialStyle::drawButtonFace(QPainter *p, const QStyleOption *o, bool flat, bool isDefault, bool tool) const
{
    const bool en = isEnabled(o);
    const bool down = en && (o->state & State_Sunken);
    const bool on = en && (o->state & State_On);
    const bool hov = en && (o->state & State_MouseOver);
    const bool focus = en && (o->state & State_HasFocus);

    const QColor text = o->palette.color(QPalette::ButtonText);
    QColor fill = o->palette.color(QPalette::Button);
    QColor content = text;

    if (!en) {
        fill = flat ? QColor(Qt::transparent) : withAlpha(text, 0.12);
        content = text;
    } else if (isDefault) {
        fill = m_primary;
        content = m_onPrimary;
    } else if (flat) {
        // tombol datar / auto-raise: transparan; tombol toggle aktif memakai warna tonal
        fill = on ? o->palette.color(QPalette::Button) : QColor(Qt::transparent);
    }

    const qreal layer = stateOpacity(down, hov, focus);
    if (fill.alpha() == 0 && layer == 0.0 && !on) {
        return;
    }

    const QRectF r = QRectF(o->rect).adjusted(0.5, 0.5, -0.5, -0.5);
    const qreal radius = tool ? R_TOOL : r.height() / 2;

    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    if (fill.alpha() > 0) {
        fillRound(p, r, fill, radius);
    }
    if (en && layer > 0.0) {
        fillRound(p, r, withAlpha(content, layer), radius);
    }
    p->restore();
}

void MaterialStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    switch (element) {
    case PE_FrameFocusRect:
        return; // fokus ditandai lewat state layer, bukan garis putus-putus

    case PE_PanelButtonCommand: {
        const auto *b = qstyleoption_cast<const QStyleOptionButton *>(opt);
        const bool flat = b && (b->features & QStyleOptionButton::Flat);
        const bool def = b && (b->features & QStyleOptionButton::DefaultButton);
        drawButtonFace(p, opt, flat, def, false);
        return;
    }
    case PE_PanelButtonTool:
    case PE_IndicatorButtonDropDown: // bagian panah tombol-menu: datar juga, bukan pil tonal
        drawButtonFace(p, opt, true, false, true);
        return;

    case PE_PanelLineEdit:
    case PE_FrameLineEdit: {
        const auto *f = qstyleoption_cast<const QStyleOptionFrame *>(opt);
        const QWidget *parent = w ? w->parentWidget() : nullptr;
        if (parent && (qobject_cast<const QAbstractSpinBox *>(parent) || qobject_cast<const QComboBox *>(parent))) {
            // bagian dalam spinbox / combobox editable: bingkainya digambar oleh kontrol induk
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Base));
            return;
        }
        if (f && f->lineWidth == 0) {
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Base));
            return;
        }
        const bool en = isEnabled(opt);
        QColor line = outline(opt->palette);
        qreal width = 1;
        if (en && (opt->state & State_HasFocus)) {
            line = m_primary;
            width = 2;
        } else if (en && (opt->state & State_MouseOver)) {
            line = opt->palette.color(QPalette::Text);
        } else if (!en) {
            line = withAlpha(opt->palette.color(QPalette::Text), 0.12);
        }
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        fillRound(p, QRectF(opt->rect), opt->palette.color(QPalette::Base), R_FIELD);
        strokeRound(p, QRectF(opt->rect), line, width, R_FIELD);
        p->restore();
        return;
    }
    case PE_FrameGroupBox: {
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        strokeRound(p, QRectF(opt->rect), outlineVariant(opt->palette), 1, R_CARD);
        p->restore();
        return;
    }
    case PE_FrameMenu:
        if (isRoundMenu(w)) {
            return; // garis tepi digambar bersama panel (PE_PanelMenu), supaya sudut ikut bulat
        }
        [[fallthrough]];
    case PE_Frame:
    case PE_FrameTabWidget: {
        const QColor line = outlineVariant(opt->palette);
        const QRect r = opt->rect;
        p->fillRect(QRect(r.left(), r.top(), r.width(), 1), line);
        p->fillRect(QRect(r.left(), r.bottom(), r.width(), 1), line);
        p->fillRect(QRect(r.left(), r.top(), 1, r.height()), line);
        p->fillRect(QRect(r.right(), r.top(), 1, r.height()), line);
        return;
    }
    case PE_PanelMenu: {
        // surface container: sedikit lebih terang dari window
        const QColor surface = menuSurface(opt->palette);
        if (isRoundMenu(w)) {
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            p->setCompositionMode(QPainter::CompositionMode_Source);
            fillRound(p, QRectF(opt->rect), surface, R_MENU);
            p->setCompositionMode(QPainter::CompositionMode_SourceOver);
            strokeRound(p, QRectF(opt->rect), outlineVariant(opt->palette), 1, R_MENU);
            p->restore();
        } else {
            p->fillRect(opt->rect, surface);
        }
        return;
    }
    case PE_PanelMenuBar:
        p->fillRect(opt->rect, opt->palette.color(QPalette::Window));
        return;
    case PE_PanelTipLabel: {
        // inverse surface: terang di tema gelap
        p->fillRect(opt->rect, opt->palette.color(QPalette::ToolTipBase));
        return;
    }
    case PE_IndicatorCheckBox:
    case PE_IndicatorItemViewItemCheck: {
        const bool en = isEnabled(opt);
        const bool checked = opt->state & (State_On | State_NoChange);
        QRectF r(0, 0, 18, 18);
        r.moveCenter(QRectF(opt->rect).center());
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        const QColor line = en ? outline(opt->palette) : withAlpha(opt->palette.color(QPalette::Text), 0.38);
        if (checked) {
            const QColor fill = en ? m_primary : withAlpha(opt->palette.color(QPalette::Text), 0.38);
            fillRound(p, r, fill, 2);
            QPen pen(en ? m_onPrimary : opt->palette.color(QPalette::Window), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            p->setPen(pen);
            p->setBrush(Qt::NoBrush);
            if (opt->state & State_NoChange) {
                p->drawLine(QPointF(r.center().x() - 4, r.center().y()), QPointF(r.center().x() + 4, r.center().y()));
            } else {
                QPolygonF poly;
                poly << r.topLeft() + QPointF(4.5, 9.3) << r.topLeft() + QPointF(7.7, 12.4) << r.topLeft() + QPointF(13.5, 5.8);
                p->drawPolyline(poly);
            }
        } else {
            strokeRound(p, r, (en && (opt->state & State_MouseOver)) ? opt->palette.color(QPalette::Text) : line, 2, 2);
        }
        p->restore();
        return;
    }
    case PE_IndicatorRadioButton: {
        const bool en = isEnabled(opt);
        const bool checked = opt->state & State_On;
        QRectF r(0, 0, 18, 18);
        r.moveCenter(QRectF(opt->rect).center());
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        const QColor line = checked ? m_primary : outline(opt->palette);
        const QColor ring = en ? line : withAlpha(opt->palette.color(QPalette::Text), 0.38);
        p->setPen(QPen(ring, 2));
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(r.adjusted(1, 1, -1, -1));
        if (checked) {
            p->setPen(Qt::NoPen);
            p->setBrush(ring);
            p->drawEllipse(r.adjusted(5, 5, -5, -5));
        }
        p->restore();
        return;
    }
    case PE_PanelItemViewItem: {
        const auto *vi = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
        if (vi && (vi->features & QStyleOptionViewItem::Alternate)) {
            p->fillRect(opt->rect, opt->palette.brush(QPalette::AlternateBase));
        }
        if (vi && vi->backgroundBrush.style() != Qt::NoBrush) {
            p->fillRect(opt->rect, vi->backgroundBrush);
        }
        if (isEnabled(opt) && (opt->state & State_Selected)) {
            p->fillRect(opt->rect, opt->palette.color(QPalette::Highlight));
            if (opt->state & State_MouseOver) {
                p->fillRect(opt->rect, withAlpha(opt->palette.color(QPalette::HighlightedText), 0.08));
            }
        } else if (isEnabled(opt) && (opt->state & State_MouseOver)) {
            p->fillRect(opt->rect, withAlpha(opt->palette.color(QPalette::Text), 0.08));
        }
        return;
    }
    default:
        break;
    }
    QProxyStyle::drawPrimitive(element, opt, p, w);
}

// ---------------------------------------------------------------------------
// Item menu: tanpa isi primary penuh. Hover / fokus = state layer 10% dari warna teks.
// Kolom (kiri ke kanan): centang | ikon | teks ... pintasan | panah submenu
// ---------------------------------------------------------------------------
void MaterialStyle::drawMenuItem(const QStyleOptionMenuItem *mi, QPainter *p, const QWidget *w) const
{
    const bool en = mi->state & State_Enabled;
    const bool sel = en && (mi->state & State_Selected);
    const QRect r = mi->rect;
    const QColor text = mi->palette.color(en ? QPalette::Active : QPalette::Disabled, QPalette::WindowText);
    const QColor muted = en ? mix(text, mi->palette.color(QPalette::Window), 0.30) : text;

    // pemisah (atau judul bagian bila ada teks)
    if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
        p->save();
        if (mi->text.isEmpty()) {
            p->fillRect(QRect(r.left() + 4, r.center().y(), r.width() - 8, 1), outlineVariant(mi->palette));
        } else {
            QFont f = mi->font;
            f.setBold(true);
            p->setFont(f);
            QPalette pal = mi->palette;
            pal.setColor(QPalette::WindowText, muted);
            drawItemText(p, visualRect(mi->direction, r, r.adjusted(MENU_PAD_X, 0, -MENU_PAD_X, 0)),
                         Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine | Qt::TextHideMnemonic, pal, true, mi->text, QPalette::WindowText);
        }
        p->restore();
        return;
    }

    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    if (sel) {
        fillRound(p, QRectF(r), withAlpha(text, 0.10), R_FIELD);
    }

    const auto rtl = [&](const QRect &sub) { return visualRect(mi->direction, r, sub); };
    const int cy = r.center().y();
    int x = r.left() + MENU_PAD_X;

    // centang / titik radio (kolom hanya disediakan bila ada item yang bisa dicentang)
    if (mi->menuHasCheckableItems) {
        if (mi->checkType != QStyleOptionMenuItem::NotCheckable && mi->checked) {
            const QRectF box = QRectF(rtl(QRect(x, cy - 9, 18, 18)));
            const QColor c = en ? m_primary : text;
            if (mi->checkType == QStyleOptionMenuItem::Exclusive) {
                p->setPen(Qt::NoPen);
                p->setBrush(c);
                p->drawEllipse(box.center(), 4.0, 4.0);
            } else {
                p->setPen(QPen(c, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                p->setBrush(Qt::NoBrush);
                QPolygonF poly;
                poly << box.topLeft() + QPointF(4.0, 9.3) << box.topLeft() + QPointF(7.5, 12.8) << box.topLeft() + QPointF(14.0, 5.5);
                if (mi->direction == Qt::RightToLeft) {
                    poly.clear();
                    poly << box.topRight() + QPointF(-4.0, 9.3) << box.topRight() + QPointF(-7.5, 12.8) << box.topRight() + QPointF(-14.0, 5.5);
                }
                p->drawPolyline(poly);
            }
        }
        x += MENU_CHECK_W;
    }

    // ikon (popup combobox mengirim maxIconWidth tetap, jadi kolomnya hanya dipakai bila item punya ikon)
    const int iconCol = qobject_cast<const QMenu *>(w) ? mi->maxIconWidth : (mi->icon.isNull() ? 0 : pixelMetric(PM_SmallIconSize, mi, w));
    if (iconCol > 0) {
        if (!mi->icon.isNull()) {
            const int ext = pixelMetric(PM_SmallIconSize, mi, w);
            const QIcon::Mode mode = !en ? QIcon::Disabled : (sel ? QIcon::Active : QIcon::Normal);
            const QPixmap pm = mi->icon.pixmap(QSize(ext, ext), w ? w->devicePixelRatio() : 1.0, mode, mi->checked ? QIcon::On : QIcon::Off);
            const QSizeF logical = pm.deviceIndependentSize();
            const QRect col = rtl(QRect(x, r.top(), iconCol, r.height()));
            p->drawPixmap(QPointF(col.center().x() - logical.width() / 2.0 + 0.5, cy - logical.height() / 2.0 + 0.5), pm);
        }
        x += iconCol + MENU_ICON_GAP;
    }

    // teks dan pintasan
    QString label = mi->text;
    QString shortcut;
    const int tab = label.indexOf(QLatin1Char('\t'));
    if (tab >= 0) {
        shortcut = label.mid(tab + 1);
        label = label.left(tab);
    }
    const bool sub = mi->menuItemType == QStyleOptionMenuItem::SubMenu;
    const int right = r.right() + 1 - MENU_PAD_X - (sub ? MENU_ARROW_W - 4 : 0);

    int flags = Qt::AlignVCenter | Qt::TextSingleLine | Qt::TextDontClip | Qt::TextShowMnemonic;
    if (!styleHint(SH_UnderlineShortcut, mi, w, nullptr)) {
        flags |= Qt::TextHideMnemonic;
    }
    p->setFont(mi->font);
    if (mi->menuItemType == QStyleOptionMenuItem::DefaultItem) {
        QFont f = mi->font;
        f.setBold(true);
        p->setFont(f);
    }
    drawItemText(p, rtl(QRect(x, r.top(), qMax(0, right - x), r.height())), flags | Qt::AlignLeft, mi->palette, en, label, QPalette::WindowText);

    if (!shortcut.isEmpty()) {
        p->setFont(mi->font);
        QPalette pal = mi->palette;
        if (en) {
            pal.setColor(QPalette::WindowText, muted);
        }
        drawItemText(p, rtl(QRect(x, r.top(), qMax(0, right - x), r.height())), flags | Qt::AlignRight, pal, en, shortcut, QPalette::WindowText);
    }

    // panah submenu: chevron
    if (sub) {
        const qreal ax = mi->direction == Qt::RightToLeft ? r.left() + MENU_PAD_X + 2.5 : r.right() + 1 - MENU_PAD_X - 2.5;
        const qreal dir = mi->direction == Qt::RightToLeft ? -1 : 1;
        p->setPen(QPen(en ? muted : text, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->setBrush(Qt::NoBrush);
        QPolygonF chevron;
        chevron << QPointF(ax - 2.5 * dir, cy - 4.0) << QPointF(ax + 1.5 * dir, cy) << QPointF(ax - 2.5 * dir, cy + 4.0);
        p->drawPolyline(chevron);
    }
    p->restore();
}

// ---------------------------------------------------------------------------
QRect MaterialStyle::progressFill(const QStyleOptionProgressBar *pb, const QRect &contents) const
{
    const qint64 range = qint64(pb->maximum) - pb->minimum;
    if (range <= 0) {
        return QRect();
    }
    const qreal v = qBound<qreal>(0, qreal(qint64(pb->progress) - pb->minimum) / range, 1);
    const bool horizontal = pb->state & State_Horizontal;
    QRect r = contents;
    if (horizontal) {
        r.setWidth(int(contents.width() * v));
        if ((pb->direction == Qt::RightToLeft) != pb->invertedAppearance) {
            r.moveRight(contents.right());
        }
    } else {
        r.setHeight(int(contents.height() * v));
        if (!pb->invertedAppearance) {
            r.moveBottom(contents.bottom());
        }
    }
    return r;
}

void MaterialStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    switch (element) {
    case CE_PushButtonBevel:
        drawPrimitive(PE_PanelButtonCommand, opt, p, w);
        return;

    case CE_PushButton: {
        const auto *b = qstyleoption_cast<const QStyleOptionButton *>(opt);
        if (!b) {
            break;
        }
        drawControl(CE_PushButtonBevel, opt, p, w);
        QStyleOptionButton sub(*b);
        sub.rect = subElementRect(SE_PushButtonContents, opt, w);
        if (isEnabled(opt) && (b->features & QStyleOptionButton::DefaultButton)) {
            sub.palette.setColor(QPalette::ButtonText, m_onPrimary);
            sub.palette.setColor(QPalette::WindowText, m_onPrimary);
        }
        drawControl(CE_PushButtonLabel, &sub, p, w);
        return;
    }
    case CE_TabBarTabShape: {
        const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(opt);
        if (!tab) {
            break;
        }
        const bool sel = tab->state & State_Selected;
        const bool hov = isEnabled(opt) && (tab->state & State_MouseOver);
        const QRect r = tab->rect;
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        if (hov) {
            p->fillRect(r, withAlpha(tab->palette.color(QPalette::WindowText), 0.08));
        }
        if (sel) {
            // indikator 3px di sisi yang menghadap panel
            constexpr int T = 3;
            QRectF bar;
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                bar = QRectF(r.left() + 6, r.bottom() - T + 1, r.width() - 12, T);
                break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                bar = QRectF(r.left() + 6, r.top(), r.width() - 12, T);
                break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                bar = QRectF(r.right() - T + 1, r.top() + 6, T, r.height() - 12);
                break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                bar = QRectF(r.left(), r.top() + 6, T, r.height() - 12);
                break;
            }
            fillRound(p, bar, m_primary, 1.5);
        }
        p->restore();
        return;
    }
    case CE_TabBarTabLabel: {
        const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(opt);
        if (tab && (tab->state & State_Selected) && isEnabled(opt)) {
            QStyleOptionTab t(*tab);
            t.state &= ~State_MouseOver;
            t.palette.setColor(QPalette::WindowText, m_primary);
            t.palette.setColor(QPalette::ButtonText, m_primary);
            t.palette.setColor(QPalette::Text, m_primary);
            QProxyStyle::drawControl(element, &t, p, w);
            return;
        }
        break;
    }
    case CE_MenuItem: {
        const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
        if (mi && mi->menuItemType != QStyleOptionMenuItem::Scroller && mi->menuItemType != QStyleOptionMenuItem::TearOff
            && mi->menuItemType != QStyleOptionMenuItem::EmptyArea && mi->menuItemType != QStyleOptionMenuItem::Margin) {
            drawMenuItem(mi, p, w);
            return;
        }
        break;
    }
    case CE_MenuEmptyArea:
        return; // latar sudah digambar oleh PE_PanelMenu

    case CE_MenuBarEmptyArea:
        return;

    case CE_MenuBarItem: {
        const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
        if (!mi) {
            break;
        }
        const bool en = isEnabled(opt);
        const bool down = en && (opt->state & State_Sunken);
        const bool sel = en && (opt->state & State_Selected);
        if (down || sel) {
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            fillRound(p, QRectF(mi->rect).adjusted(0, 2, 0, -2), withAlpha(mi->palette.color(QPalette::WindowText), down ? 0.12 : 0.08), R_TOOL);
            p->restore();
        }
        int flags = Qt::AlignCenter | Qt::TextSingleLine | Qt::TextDontClip | Qt::TextShowMnemonic;
        if (!styleHint(SH_UnderlineShortcut, opt, w, nullptr)) {
            flags |= Qt::TextHideMnemonic;
        }
        drawItemText(p, mi->rect, flags, mi->palette, en, mi->text, QPalette::WindowText);
        return;
    }
    case CE_HeaderSection: {
        QColor fill = opt->palette.color(QPalette::Window);
        p->fillRect(opt->rect, fill);
        if (isEnabled(opt) && (opt->state & (State_Sunken | State_MouseOver))) {
            p->fillRect(opt->rect, withAlpha(opt->palette.color(QPalette::WindowText), (opt->state & State_Sunken) ? 0.10 : 0.08));
        }
        p->fillRect(QRect(opt->rect.left(), opt->rect.bottom(), opt->rect.width(), 1), outlineVariant(opt->palette));
        return;
    }
    case CE_ProgressBarGroove: {
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        const QRectF r(opt->rect);
        fillRound(p, r, withAlpha(opt->palette.color(QPalette::WindowText), 0.16), qMin(r.height() / 2, 8.0));
        p->restore();
        return;
    }
    case CE_ProgressBarContents: {
        const auto *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(opt);
        if (!pb || pb->minimum == pb->maximum) {
            break; // mode sibuk: biarkan Fusion
        }
        const QRect contents = subElementRect(SE_ProgressBarContents, pb, w);
        const QRect fill = progressFill(pb, contents);
        if (fill.width() <= 0 || fill.height() <= 0) {
            return;
        }
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        const QColor c = isEnabled(opt) ? m_primary : withAlpha(opt->palette.color(QPalette::WindowText), 0.38);
        fillRound(p, QRectF(fill), c, qMin<qreal>(qMin(fill.width(), fill.height()) / 2.0, 8.0));
        p->restore();
        return;
    }
    case CE_ProgressBarLabel: {
        const auto *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(opt);
        if (!pb || !pb->textVisible || pb->text.isEmpty()) {
            return;
        }
        const QRect contents = subElementRect(SE_ProgressBarContents, pb, w);
        const QRect fill = pb->minimum == pb->maximum ? QRect() : progressFill(pb, contents);
        p->save();
        p->setClipRegion(QRegion(pb->rect) - QRegion(fill));
        drawItemText(p, pb->rect, Qt::AlignCenter | Qt::TextSingleLine, pb->palette, isEnabled(pb), pb->text, QPalette::WindowText);
        p->setClipRect(fill);
        QPalette pal = pb->palette;
        pal.setColor(QPalette::WindowText, m_onPrimary);
        drawItemText(p, pb->rect, Qt::AlignCenter | Qt::TextSingleLine, pal, isEnabled(pb), pb->text, QPalette::WindowText);
        p->restore();
        return;
    }
    default:
        break;
    }
    QProxyStyle::drawControl(element, opt, p, w);
}

// Tanda skala slider: titik kecil di sisi alur. (Fusion menggambarnya dengan warna garis yang
// hampir sama dengan latar gelap, jadi nyaris tak terlihat.)
void MaterialStyle::drawSliderTicks(const QStyleOptionSlider *sl, QPainter *p, const QWidget *w) const
{
    const bool horiz = sl->orientation == Qt::Horizontal;
    const int available = pixelMetric(PM_SliderSpaceAvailable, sl, w);
    const int len = pixelMetric(PM_SliderLength, sl, w);
    const int offset = pixelMetric(PM_SliderTickmarkOffset, sl, w);
    if (sl->maximum <= sl->minimum || available <= 0) {
        return;
    }
    int interval = sl->tickInterval;
    if (interval <= 0) {
        interval = sl->singleStep;
        if (sliderPositionFromValue(sl->minimum, sl->maximum, interval, available) - sliderPositionFromValue(sl->minimum, sl->maximum, 0, available) < 3) {
            interval = sl->pageStep;
        }
    }
    if (interval <= 0 || (qint64(sl->maximum) - sl->minimum) / interval > 500) {
        return;
    }
    const bool en = isEnabled(sl);
    const QColor c = withAlpha(sl->palette.color(QPalette::WindowText), en ? 0.60 : 0.24);
    const bool above = sl->tickPosition & QSlider::TicksAbove;
    const bool below = sl->tickPosition & QSlider::TicksBelow;
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(c);
    for (qint64 v = sl->minimum; v <= sl->maximum; v += interval) {
        const qreal pos = sliderPositionFromValue(sl->minimum, sl->maximum, int(v), available, sl->upsideDown) + len / 2.0;
        const qreal gap = offset / 2.0;
        if (horiz) {
            if (above) {
                p->drawEllipse(QPointF(sl->rect.left() + pos, sl->rect.top() + gap), 1.5, 1.5);
            }
            if (below) {
                p->drawEllipse(QPointF(sl->rect.left() + pos, sl->rect.bottom() + 1 - gap), 1.5, 1.5);
            }
        } else {
            if (above) {
                p->drawEllipse(QPointF(sl->rect.left() + gap, sl->rect.top() + pos), 1.5, 1.5);
            }
            if (below) {
                p->drawEllipse(QPointF(sl->rect.right() + 1 - gap, sl->rect.top() + pos), 1.5, 1.5);
            }
        }
    }
    p->restore();
}

// ---------------------------------------------------------------------------
void MaterialStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *w) const
{
    switch (cc) {
    case CC_ScrollBar: {
        const auto *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt);
        if (!sb) {
            break;
        }
        const bool horiz = sb->orientation == Qt::Horizontal;
        const bool en = isEnabled(sb);
        p->fillRect(sb->rect, sb->palette.color(QPalette::Base));
        if (!en || sb->maximum <= sb->minimum) {
            return;
        }
        QRectF slider = subControlRect(CC_ScrollBar, sb, SC_ScrollBarSlider, w);
        slider = horiz ? slider.adjusted(1, 3, -1, -3) : slider.adjusted(3, 1, -3, -1);
        const bool active = sb->activeSubControls & SC_ScrollBarSlider;
        const bool down = active && (sb->state & State_Sunken);
        const QColor text = sb->palette.color(QPalette::WindowText);
        const QColor fill = down ? m_primary : withAlpha(text, active ? 0.60 : 0.38);
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        fillRound(p, slider, fill, qMin(slider.width(), slider.height()) / 2);
        p->restore();
        return;
    }
    case CC_Slider: {
        const auto *sl = qstyleoption_cast<const QStyleOptionSlider *>(opt);
        if (!sl) {
            break;
        }
        const bool horiz = sl->orientation == Qt::Horizontal;
        const bool en = isEnabled(sl);
        const QColor text = sl->palette.color(QPalette::WindowText);
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        if (sl->subControls & SC_SliderGroove) {
            const QRect gr = subControlRect(CC_Slider, sl, SC_SliderGroove, w);
            const QRectF g = horiz ? QRectF(gr.left(), gr.center().y() - 4, gr.width(), 8) : QRectF(gr.center().x() - 4, gr.top(), 8, gr.height());
            fillRound(p, g, mix(sl->palette.color(QPalette::Window), text, 0.18), 4);
            const QRect hr = subControlRect(CC_Slider, sl, SC_SliderHandle, w);
            QRectF gi = g;
            if (horiz) {
                if (sl->upsideDown) {
                    gi.setLeft(hr.center().x());
                } else {
                    gi.setRight(hr.center().x());
                }
            } else {
                if (sl->upsideDown) {
                    gi.setBottom(hr.center().y());
                } else {
                    gi.setTop(hr.center().y());
                }
            }
            if (en) {
                fillRound(p, gi, m_primary, 4);
            }
        }
        p->restore();
        if ((sl->subControls & SC_SliderTickmarks) && sl->tickPosition != QSlider::NoTicks) {
            drawSliderTicks(sl, p, w);
        }
        if (sl->subControls & SC_SliderHandle) {
            const QRectF hr = subControlRect(CC_Slider, sl, SC_SliderHandle, w);
            const bool act = en && (sl->activeSubControls & SC_SliderHandle);
            const bool down = act && (sl->state & State_Sunken);
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            if (act) {
                fillRound(p, hr, withAlpha(m_primary, down ? 0.20 : 0.12), hr.width() / 2);
            }
            // Knob gelap dengan cincin primary, senada slider Plasma.
            const QColor fill = en ? m_onPrimary : withAlpha(text, 0.38);
            const QRectF knob = hr.adjusted(2, 2, -2, -2);
            fillRound(p, knob, fill, qMin(knob.width(), knob.height()) / 2);
            if (en) {
                p->setBrush(Qt::NoBrush);
                p->setPen(QPen(m_primary, 2));
                p->drawEllipse(knob.adjusted(1, 1, -1, -1));
            }
            p->restore();
        }
        return;
    }
    case CC_SpinBox: {
        const auto *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt);
        if (!sb) {
            break;
        }
        const bool en = isEnabled(sb);
        QColor line = outline(sb->palette);
        qreal width = 1;
        if (en && (sb->state & State_HasFocus)) {
            line = m_primary;
            width = 2;
        } else if (en && (sb->state & State_MouseOver)) {
            line = sb->palette.color(QPalette::Text);
        } else if (!en) {
            line = withAlpha(sb->palette.color(QPalette::Text), 0.12);
        }
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        if (sb->frame) {
            fillRound(p, QRectF(sb->rect), sb->palette.color(QPalette::Base), R_FIELD);
            strokeRound(p, QRectF(sb->rect), line, width, R_FIELD);
        } else {
            p->fillRect(sb->rect, sb->palette.color(QPalette::Base));
        }
        p->restore();
        if (sb->buttonSymbols == QAbstractSpinBox::NoButtons) {
            return;
        }
        struct Btn {
            SubControl sc;
            PrimitiveElement arrow;
            bool enabledStep;
        };
        const Btn btns[2] = {{SC_SpinBoxUp, PE_IndicatorSpinUp, bool(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled)},
                             {SC_SpinBoxDown, PE_IndicatorSpinDown, bool(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled)}};
        for (const Btn &b : btns) {
            const QRect r = subControlRect(CC_SpinBox, sb, b.sc, w);
            QStyleOption ao;
            ao.QStyleOption::operator=(*sb);
            const bool act = en && b.enabledStep && (sb->activeSubControls & b.sc);
            if (act) {
                p->save();
                p->setRenderHint(QPainter::Antialiasing);
                fillRound(p, QRectF(r).adjusted(1, 1, -1, -1), withAlpha(sb->palette.color(QPalette::Text), (sb->state & State_Sunken) ? 0.10 : 0.08), 3);
                p->restore();
            }
            ao.rect = r;
            if (!(en && b.enabledStep)) {
                ao.state &= ~State_Enabled;
            }
            drawPrimitive(b.arrow, &ao, p, w);
        }
        return;
    }
    case CC_ComboBox: {
        const auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt);
        if (!cb) {
            break;
        }
        const bool en = isEnabled(cb);
        const bool hov = en && (cb->state & State_MouseOver);
        const bool open = en && (cb->state & (State_On | State_Sunken));
        const bool focus = en && (cb->state & State_HasFocus);
        const QRect face = subControlRect(CC_ComboBox, cb, SC_ComboBoxFrame, w);
        const QRect arrow = subControlRect(CC_ComboBox, cb, SC_ComboBoxArrow, w);
        QColor line = outline(cb->palette);
        qreal width = 1;
        if (focus || open) {
            line = m_primary;
            width = 2;
        } else if (hov) {
            line = cb->palette.color(QPalette::Text);
        } else if (!en) {
            line = withAlpha(cb->palette.color(QPalette::Text), 0.12);
        }
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        fillRound(p, QRectF(face), cb->palette.color(QPalette::Base), R_FIELD);
        strokeRound(p, QRectF(face), line, width, R_FIELD);
        p->restore();
        QStyleOption ao;
        ao.QStyleOption::operator=(*cb);
        ao.rect = arrow;
        drawPrimitive(PE_IndicatorArrowDown, &ao, p, w);
        return;
    }
    default:
        break;
    }
    QProxyStyle::drawComplexControl(cc, opt, p, w);
}

// ---------------------------------------------------------------------------
int MaterialStyle::pixelMetric(PixelMetric m, const QStyleOption *opt, const QWidget *w) const
{
    switch (m) {
    case PM_DefaultFrameWidth:
    case PM_ComboBoxFrameWidth:
    case PM_SpinBoxFrameWidth:
    case PM_MenuPanelWidth:
        return 1;
    case PM_MenuHMargin:
        return 4;
    case PM_MenuVMargin:
        return 6;
    case PM_ToolTipLabelFrameWidth:
        return 6;
    case PM_ScrollBarExtent:
        return 12;
    case PM_ScrollBarSliderMin:
        return 32;
    case PM_SliderThickness:
    case PM_SliderControlThickness:
        return 20;
    case PM_SliderLength:
        return 20;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        return 18;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
    case PM_TabBarTabOverlap:
        return 0;
    default:
        break;
    }
    return QProxyStyle::pixelMetric(m, opt, w);
}

QSize MaterialStyle::sizeFromContents(ContentsType t, const QStyleOption *opt, const QSize &sz, const QWidget *w) const
{
    QSize s = QProxyStyle::sizeFromContents(t, opt, sz, w);
    switch (t) {
    case CT_PushButton:
        s += QSize(16, 0);
        s.setHeight(qMax(s.height(), BTN_H));
        break;
    case CT_ComboBox:
        s.rwidth() += 12;
        s.setHeight(qMax(s.height(), FIELD_H));
        break;
    case CT_ToolButton:
        s += QSize(8, 8);
        break;
    case CT_LineEdit:
        s.setHeight(qMax(s.height(), FIELD_H));
        break;
    case CT_SpinBox:
        s.setHeight(qMax(s.height(), FIELD_H));
        s.rwidth() += SPIN_W;
        break;
    case CT_TabBarTab:
        s += QSize(16, 8);
        break;
    case CT_MenuBarItem:
        s = sz + QSize(20, 10);
        break;
    case CT_MenuItem: {
        // ukuran dihitung sendiri dari font (sesuai kolom di drawMenuItem). Argumen sz tidak dipakai:
        // popup QComboBox mengirim ukuran viewport, bukan ukuran teks.
        const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
        if (!mi) {
            break;
        }
        if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
            const int sepW = mi->fontMetrics.horizontalAdvance(mi->text) + 2 * MENU_PAD_X;
            return QSize(sepW, mi->text.isEmpty() ? MENU_SEP_H : mi->fontMetrics.height() + 14);
        }
        QString label = mi->text;
        const int tab = label.indexOf(QLatin1Char('\t'));
        if (tab >= 0) {
            label = label.left(tab);
        }
        QFont font = mi->font;
        if (mi->menuItemType == QStyleOptionMenuItem::DefaultItem) {
            font.setBold(true);
        }
        const QFontMetrics fm(font);
        int width = fm.boundingRect(QRect(), Qt::TextSingleLine | Qt::TextShowMnemonic, label).width() + 2 * MENU_PAD_X;
        int height = fm.height();
        if (!mi->icon.isNull()) {
            height = qMax(height, pixelMetric(PM_SmallIconSize, mi, w));
        }
        if (mi->menuHasCheckableItems) {
            width += MENU_CHECK_W;
        }
        const int iconCol = qobject_cast<const QMenu *>(w) ? mi->maxIconWidth : (mi->icon.isNull() ? 0 : pixelMetric(PM_SmallIconSize, mi, w));
        if (iconCol > 0) {
            width += iconCol + MENU_ICON_GAP;
        }
        if (tab >= 0) {
            width += MENU_TAB_GAP;
        }
        if (mi->menuItemType == QStyleOptionMenuItem::SubMenu) {
            width += MENU_ARROW_W;
        }
        return QSize(qMax(width, 140), qMax(height + 14, MENU_ITEM_H));
    }
    default:
        break;
    }
    return s;
}

QRect MaterialStyle::subElementRect(SubElement e, const QStyleOption *opt, const QWidget *w) const
{
    switch (e) {
    case SE_PushButtonContents:
    case SE_PushButtonFocusRect:
        return opt->rect.adjusted(8, 2, -8, -2);
    case SE_ProgressBarGroove:
    case SE_ProgressBarLabel:
        return opt->rect;
    case SE_ProgressBarContents:
        return w ? w->rect() : opt->rect;
    default:
        break;
    }
    return QProxyStyle::subElementRect(e, opt, w);
}

QRect MaterialStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    switch (cc) {
    case CC_ScrollBar: {
        // tanpa tombol panah: seluruh area adalah groove, slider dihitung sendiri
        const auto *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt);
        if (!sb) {
            break;
        }
        const bool horiz = sb->orientation == Qt::Horizontal;
        const QRect rect = sb->rect;
        const int length = horiz ? rect.width() : rect.height();
        int sliderLen = length;
        if (sb->maximum != sb->minimum) {
            const qint64 range = qint64(sb->maximum) - sb->minimum;
            sliderLen = int((qint64(sb->pageStep) * length) / (range + sb->pageStep));
            const int sliderMin = pixelMetric(PM_ScrollBarSliderMin, sb, w);
            if (sliderLen < sliderMin || range > INT_MAX / 2) {
                sliderLen = sliderMin;
            }
            if (sliderLen > length) {
                sliderLen = length;
            }
        }
        const int sliderStart = sliderPositionFromValue(sb->minimum, sb->maximum, sb->sliderPosition, length - sliderLen, sb->upsideDown);
        switch (sc) {
        case SC_ScrollBarSubLine:
        case SC_ScrollBarAddLine:
            return QRect();
        case SC_ScrollBarGroove:
            return rect;
        case SC_ScrollBarSlider:
            return horiz ? QRect(sliderStart, 0, sliderLen, rect.height()) : QRect(0, sliderStart, rect.width(), sliderLen);
        case SC_ScrollBarSubPage:
            return horiz ? QRect(0, 0, sliderStart, rect.height()) : QRect(0, 0, rect.width(), sliderStart);
        case SC_ScrollBarAddPage:
            return horiz ? QRect(sliderStart + sliderLen, 0, length - sliderStart - sliderLen, rect.height())
                         : QRect(0, sliderStart + sliderLen, rect.width(), length - sliderStart - sliderLen);
        default:
            break;
        }
        break;
    }
    case CC_SpinBox: {
        const auto *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt);
        if (!sb) {
            break;
        }
        const QRect inner = sb->rect.adjusted(1, 1, -1, -1);
        const bool buttons = sb->buttonSymbols != QAbstractSpinBox::NoButtons;
        const int bw = buttons ? SPIN_W : 0;
        const int half = inner.height() / 2;
        switch (sc) {
        case SC_SpinBoxUp:
            return QRect(inner.right() - bw + 1, inner.top(), bw, half);
        case SC_SpinBoxDown:
            return QRect(inner.right() - bw + 1, inner.top() + half, bw, inner.height() - half);
        case SC_SpinBoxEditField:
            return QRect(inner.left() + 4, inner.top(), inner.width() - bw - 4, inner.height());
        case SC_SpinBoxFrame:
            return sb->rect;
        default:
            break;
        }
        break;
    }
    case CC_ComboBox: {
        const auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt);
        if (!cb) {
            break;
        }
        const QRect face = cb->rect;
        const QRect inner = face.adjusted(1, 1, -1, -1);
        switch (sc) {
        case SC_ComboBoxFrame:
            return face;
        case SC_ComboBoxArrow:
            return QRect(inner.right() - ARROW_W + 1, inner.top(), ARROW_W, inner.height());
        case SC_ComboBoxEditField: {
            const QRect r(inner.left(), inner.top(), inner.width() - ARROW_W, inner.height());
            return cb->editable ? r : r.adjusted(8, 0, 0, 0);
        }
        case SC_ComboBoxListBoxPopup:
            return QProxyStyle::subControlRect(cc, opt, sc, w);
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
    return QProxyStyle::subControlRect(cc, opt, sc, w);
}

int MaterialStyle::styleHint(StyleHint h, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *ret) const
{
    switch (h) {
    case SH_EtchDisabledText:
    case SH_DitherDisabledText:
        return 0;
    default:
        break;
    }
    return QProxyStyle::styleHint(h, opt, w, ret);
}
