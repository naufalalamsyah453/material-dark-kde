// Render galeri widget Material Dark ke PNG (offscreen), untuk pratinjau / README.
// SPDX-License-Identifier: MIT
#include "../style/materialstyle.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QActionGroup>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

static QPalette materialPalette()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(26, 22, 25));
    p.setColor(QPalette::WindowText, QColor(240, 228, 231));
    p.setColor(QPalette::Base, QColor(16, 14, 17));
    p.setColor(QPalette::AlternateBase, QColor(33, 28, 31));
    p.setColor(QPalette::Text, QColor(240, 228, 231));
    p.setColor(QPalette::Button, QColor(44, 37, 40));
    p.setColor(QPalette::ButtonText, QColor(240, 228, 231));
    p.setColor(QPalette::Highlight, QColor(242, 184, 198));
    p.setColor(QPalette::HighlightedText, QColor(43, 18, 25));
    p.setColor(QPalette::ToolTipBase, QColor(50, 42, 46));
    p.setColor(QPalette::ToolTipText, QColor(240, 228, 231));
    p.setColor(QPalette::Mid, QColor(58, 48, 52));
    p.setColor(QPalette::PlaceholderText, QColor(205, 185, 191));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 106, 112));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(120, 106, 112));
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(120, 106, 112));
    return p;
}

int main(int argc, char **argv)
{
    // argumen ke-3 (opsional): file .colors untuk mengambil warna aksen
    if (argc > 3) {
        qputenv("MATERIALDARK_COLOR_SCHEME", argv[3]);
    }
    QApplication app(argc, argv);
    app.setStyle(new MaterialStyle);
    app.setPalette(materialPalette());

    QWidget win;
    auto *root = new QVBoxLayout(&win);

    auto *buttons = new QHBoxLayout;
    auto *def = new QPushButton(QStringLiteral("Default"));
    def->setDefault(true);
    auto *normal = new QPushButton(QStringLiteral("Tonal"));
    auto *flat = new QPushButton(QStringLiteral("Flat"));
    flat->setFlat(true);
    auto *off = new QPushButton(QStringLiteral("Disabled"));
    off->setEnabled(false);
    auto *tool = new QToolButton;
    tool->setText(QStringLiteral("Tool"));
    buttons->addWidget(def);
    buttons->addWidget(normal);
    buttons->addWidget(flat);
    buttons->addWidget(off);
    buttons->addWidget(tool);
    root->addLayout(buttons);

    auto *fields = new QHBoxLayout;
    auto *edit = new QLineEdit(QStringLiteral("Text field"));
    auto *combo = new QComboBox;
    combo->addItems({QStringLiteral("Combo box"), QStringLiteral("Item 2")});
    auto *spin = new QSpinBox;
    spin->setValue(42);
    fields->addWidget(edit);
    fields->addWidget(combo);
    fields->addWidget(spin);
    root->addLayout(fields);

    auto *checks = new QHBoxLayout;
    auto *c1 = new QCheckBox(QStringLiteral("Checked"));
    c1->setChecked(true);
    auto *c2 = new QCheckBox(QStringLiteral("Unchecked"));
    auto *r1 = new QRadioButton(QStringLiteral("Radio on"));
    r1->setChecked(true);
    auto *r2 = new QRadioButton(QStringLiteral("Radio off"));
    checks->addWidget(c1);
    checks->addWidget(c2);
    checks->addWidget(r1);
    checks->addWidget(r2);
    root->addLayout(checks);

    auto *slider = new QSlider(Qt::Horizontal);
    slider->setValue(60);
    root->addWidget(slider);
    auto *bar = new QProgressBar;
    bar->setValue(65);
    root->addWidget(bar);
    auto *scroll = new QScrollBar(Qt::Horizontal);
    scroll->setRange(0, 100);
    scroll->setPageStep(30);
    scroll->setValue(20);
    root->addWidget(scroll);

    auto *tabs = new QTabWidget;
    auto *page = new QLabel(QStringLiteral("Tab content"));
    page->setMargin(12);
    tabs->addTab(page, QStringLiteral("Overview"));
    tabs->addTab(new QLabel, QStringLiteral("Details"));
    tabs->addTab(new QLabel, QStringLiteral("Settings"));
    root->addWidget(tabs);

    auto *group = new QGroupBox(QStringLiteral("Group box"));
    auto *gl = new QVBoxLayout(group);
    gl->addWidget(new QLabel(QStringLiteral("Inside the group")));
    root->addWidget(group);

    win.resize(520, 480);
    win.show();
    const QString out = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("preview.png");
    win.grab().save(out);

    // argumen ke-2 (opsional): render context menu ke PNG
    if (argc > 2) {
        const auto dot = [](const QColor &c) {
            QPixmap pm(16, 16);
            pm.fill(Qt::transparent);
            QPainter p(&pm);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setBrush(c);
            p.drawRoundedRect(1, 1, 14, 14, 3, 3);
            return QIcon(pm);
        };
        QMenu menu;
        menu.addAction(dot(QColor(168, 199, 250)), QStringLiteral("&Open"))->setShortcut(QKeySequence(QStringLiteral("Ctrl+O")));
        menu.addAction(dot(QColor(168, 218, 181)), QStringLiteral("&Save"))->setShortcut(QKeySequence(QStringLiteral("Ctrl+S")));
        QAction *active = menu.addAction(dot(QColor(239, 184, 200)), QStringLiteral("Switch &User"));
        menu.addAction(QStringLiteral("Disabled"))->setEnabled(false);
        menu.addSeparator();
        auto *on = menu.addAction(QStringLiteral("Checkable on"));
        on->setCheckable(true);
        on->setChecked(true);
        menu.addAction(QStringLiteral("Checkable off"))->setCheckable(true);
        menu.addSeparator();
        auto *group = new QActionGroup(&menu);
        for (const char *name : {"Radio one", "Radio two"}) {
            auto *a = menu.addAction(QString::fromLatin1(name));
            a->setCheckable(true);
            group->addAction(a);
        }
        group->actions().first()->setChecked(true);
        QMenu sub(QStringLiteral("More tools"));
        sub.addAction(QStringLiteral("Child"));
        menu.addMenu(&sub);
        menu.setActiveAction(active);
        menu.popup(QPoint(0, 0));
        app.processEvents();
        QPixmap shot(menu.size());
        shot.fill(Qt::transparent);
        menu.render(&shot);
        QPixmap canvas(shot.width() + 40, shot.height() + 40);
        canvas.fill(materialPalette().color(QPalette::Window));
        QPainter cp(&canvas);
        cp.drawPixmap(20, 20, shot);
        cp.end();
        canvas.save(QString::fromLocal8Bit(argv[2]));
    }
    return 0;
}
