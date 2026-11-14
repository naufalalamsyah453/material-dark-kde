#!/usr/bin/env bash
# Material Dark untuk KDE Plasma 6
# Memasang tema sebagai tema BARU (nama sendiri), tidak menimpa tema yang sudah ada.
#
#   ./install.sh                 -> pasang semuanya (style + dekorasi dikompilasi, pakai sudo)
#   ./install.sh --apply         -> pasang lalu langsung aktifkan semuanya (pink)
#   ./install.sh --apply --blue  -> pasang lalu aktifkan varian biru
#   ./install.sh --variant blue  -> pasang, aktifkan nanti secara manual
#   ./install.sh --user          -> pasang plugin ke ~/.local (tanpa sudo)
#   ./install.sh --no-build      -> hanya file tema (warna, plasma style, konsole)
set -e

DIR="$(cd "$(dirname "$0")" && pwd)"
DATA="${XDG_DATA_HOME:-$HOME/.local/share}"
CONF="${XDG_CONFIG_HOME:-$HOME/.config}"

APPLY=0; USER_INSTALL=0; BUILD=1; VARIANT=pink
while [ $# -gt 0 ]; do
  case "$1" in
    --apply) APPLY=1 ;;
    --user) USER_INSTALL=1 ;;
    --no-build) BUILD=0 ;;
    --blue) VARIANT=blue ;;
    --pink) VARIANT=pink ;;
    --variant) VARIANT="${2:-}"; shift ;;
    --variant=*) VARIANT="${1#--variant=}" ;;
    *) echo "Opsi tidak dikenal: $1"; exit 1 ;;
  esac
  shift
done

case "$VARIANT" in
  pink) SCHEME="MaterialDark"; THEME="material-dark-kde"; NICE="Material Dark" ;;
  blue) SCHEME="MaterialDarkBlue"; THEME="material-dark-kde-blue"; NICE="Material Dark Blue" ;;
  *) echo "Varian tidak dikenal: $VARIANT (pilih: pink, blue)"; exit 1 ;;
esac

# 1. Color scheme -> System Settings > Colors & Themes > Colors
mkdir -p "$DATA/color-schemes"
cp "$DIR/color-schemes/MaterialDark.colors" "$DATA/color-schemes/"
cp "$DIR/color-schemes/MaterialDarkBlue.colors" "$DATA/color-schemes/"

# 2. Plasma Style (app menu, panel, tooltip, popup) — kedua varian dipasang
mkdir -p "$DATA/plasma/desktoptheme"
for t in material-dark-kde material-dark-kde-blue; do
  rm -rf "$DATA/plasma/desktoptheme/$t"
  cp -r "$DIR/plasma/desktoptheme/$t" "$DATA/plasma/desktoptheme/"
done
cp "$DIR/color-schemes/MaterialDark.colors" "$DATA/plasma/desktoptheme/material-dark-kde/colors"
cp "$DIR/color-schemes/MaterialDarkBlue.colors" "$DATA/plasma/desktoptheme/material-dark-kde-blue/colors"
# cache SVG Plasma (tombol, slider) basi bila SVG berubah
rm -f "${XDG_CACHE_HOME:-$HOME/.cache}"/plasma-svgelements-material-dark-kde* 2>/dev/null || true
rm -f "${XDG_CACHE_HOME:-$HOME/.cache}"/plasma_theme_material-dark-kde.kcache 2>/dev/null || true
rm -f "${XDG_CACHE_HOME:-$HOME/.cache}"/plasma-svgelements-material-dark-kde-blue* 2>/dev/null || true
rm -f "${XDG_CACHE_HOME:-$HOME/.cache}"/plasma_theme_material-dark-kde-blue.kcache 2>/dev/null || true

# 3. Konsole — kedua varian
mkdir -p "$DATA/konsole"
cp "$DIR/konsole/MaterialDark.colorscheme" "$DATA/konsole/"
cp "$DIR/konsole/MaterialDarkBlue.colorscheme" "$DATA/konsole/"

echo "File tema terpasang: 'Material Dark' (pink) dan 'Material Dark Blue'. Bila tombol / slider panel belum berubah: kquitapp6 plasmashell; kstart plasmashell"

# 4. Application Style + Window Decoration (C++, dikompilasi)
if [ "$BUILD" = 1 ]; then
  command -v cmake >/dev/null || { echo "cmake tidak ada. Arch: sudo pacman -S --needed base-devel cmake extra-cmake-modules qt6-base kcoreaddons kdecoration"; exit 1; }
  if [ "$USER_INSTALL" = 1 ]; then PREFIX="$HOME/.local"; else PREFIX="/usr"; fi
  BUILD_DIR="$DIR/build"
  cmake -S "$DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX"
  cmake --build "$BUILD_DIR" -j"$(nproc)"
  if [ "$USER_INSTALL" = 1 ] || [ "$(id -u)" = 0 ]; then
    cmake --install "$BUILD_DIR"
  else
    sudo cmake --install "$BUILD_DIR"
  fi

  if [ "$USER_INSTALL" = 1 ]; then
    # plugin di ~/.local tidak ada di path default Qt; daftarkan lewat env Plasma
    mkdir -p "$CONF/plasma-workspace/env"
    cat > "$CONF/plasma-workspace/env/materialdark.sh" <<EOF
export QT_PLUGIN_PATH="\$HOME/.local/lib/qt6/plugins\${QT_PLUGIN_PATH:+:\$QT_PLUGIN_PATH}"
EOF
    cat >> "$CONF/plasma-workspace/env/materialdark.sh" <<EOF
export QML_IMPORT_PATH="\$HOME/.local/lib/qt6/qml\${QML_IMPORT_PATH:+:\$QML_IMPORT_PATH}"
export QML2_IMPORT_PATH="\$HOME/.local/lib/qt6/qml\${QML2_IMPORT_PATH:+:\$QML2_IMPORT_PATH}"
EOF
    echo "Plugin dipasang ke ~/.local. Logout/login diperlukan supaya QT_PLUGIN_PATH terbaca."
  fi
  echo "Application Style 'MaterialDesign' dan Window Decoration 'Material Dark' terpasang."
fi

if [ "$APPLY" = 1 ]; then
  plasma-apply-colorscheme "$SCHEME"
  plasma-apply-desktoptheme "$THEME"
  if [ "$BUILD" = 1 ]; then
    kwriteconfig6 --file kdeglobals --group KDE --key widgetStyle materialdesign
    kwriteconfig6 --file kwinrc --group org.kde.kdecoration2 --key library org.kde.material
    kwriteconfig6 --file kwinrc --group org.kde.kdecoration2 --key theme "Material Dark"
    qdbus6 org.kde.KWin /KWin reconfigure 2>/dev/null || true
  fi
  # style Qt Quick Controls: switch, dropdown, tombol di halaman QML (System Settings > Display, dll.)
  mkdir -p "$CONF/plasma-workspace/env"
  cat > "$CONF/plasma-workspace/env/materialdark-qml.sh" <<'EOF'
export QT_QUICK_CONTROLS_STYLE=MaterialDark
export QT_QUICK_CONTROLS_FALLBACK_STYLE=org.kde.desktop
EOF
  echo "Diaktifkan: $NICE ($SCHEME). Logout lalu login lagi supaya semua app memuat style baru."
else
  cat <<EOF

Aktifkan lewat System Settings:
  1. Colors & Themes > Colors            -> "$NICE"
  2. Colors & Themes > Plasma Style      -> "$NICE"
  3. Colors & Themes > Application Style -> "Material Dark"
  4. Colors & Themes > Window Decorations-> "Material Dark"
  5. Konsole: Settings > Edit Current Profile > Appearance > "$NICE"
Atau jalankan ulang: ./install.sh --apply$([ "$VARIANT" = blue ] && echo " --blue")
EOF
fi
