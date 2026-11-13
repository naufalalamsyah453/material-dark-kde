// Material Dark - Qt widget style (application style)
// SPDX-License-Identifier: MIT
#pragma once

#include <QColor>
#include <QProxyStyle>

class QStyleOptionMenuItem;
class QStyleOptionProgressBar;
class QStyleOptionSlider;

// Style Qt widget bergaya Material Design 3 (mode gelap): permukaan tonal,
// bentuk membulat, tanpa outline tebal, dan "state layer" untuk hover / fokus / tekan.
// Dasarnya Fusion (QProxyStyle); elemen utama digambar ulang di sini.
// Warna aksen (primary) dibaca dari color scheme aktif (kdeglobals), jadi ikut skema warna.
class MaterialStyle : public QProxyStyle
{
    Q_OBJECT
public:
    MaterialStyle();

    using QProxyStyle::polish;
    using QProxyStyle::unpolish;
    void polish(QPalette &palette) override;
    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;
    QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const override;
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const override;

private:
    void loadColors();
    void drawSliderTicks(const QStyleOptionSlider *sl, QPainter *p, const QWidget *w) const;
    void drawMenuItem(const QStyleOptionMenuItem *mi, QPainter *p, const QWidget *w) const;
    void drawButtonFace(QPainter *p, const QStyleOption *o, bool flat, bool isDefault, bool tool) const;
    QRect progressFill(const QStyleOptionProgressBar *pb, const QRect &contents) const;

    // warna dari color scheme aktif
    QColor m_primary;   // aksen: tombol utama, fokus, indikator, progress
    QColor m_onPrimary; // teks / ikon di atas m_primary
};
