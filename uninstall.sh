#!/usr/bin/env bash
# Menghapus file Material Dark saja. Tema lain tidak disentuh.
# Sebelum menjalankan ini, pindahkan dulu tema aktif ke tema lain
# (Breeze) di System Settings: Colors, Plasma Style, Application Style, Window Decorations.
set -e

DIR="$(cd "$(dirname "$0")" && pwd)"
DATA="${XDG_DATA_HOME:-$HOME/.local/share}"
CONF="${XDG_CONFIG_HOME:-$HOME/.config}"

rm -f  "$DATA/color-schemes/MaterialDark.colors"
rm -rf "$DATA/plasma/desktoptheme/material-dark-kde"
rm -f  "$DATA/konsole/MaterialDark.colorscheme"
rm -f  "$CONF/plasma-workspace/env/materialdark.sh" "$CONF/plasma-workspace/env/materialdark-qml.sh"

# plugin style + dekorasi: pakai manifest dari build, atau cari di lokasi umum
SUDO=""; [ "$(id -u)" = 0 ] || SUDO="sudo"
if [ -f "$DIR/build/install_manifest.txt" ]; then
  while IFS= read -r f; do
    if [ -w "$(dirname "$f")" ]; then rm -f "$f"; else $SUDO rm -f "$f"; fi
  done < "$DIR/build/install_manifest.txt"
else
  for base in /usr "$HOME/.local"; do
    for f in "$base/lib/qt6/plugins/styles/materialdesign.so" \
             "$base/lib/qt6/plugins/org.kde.kdecoration3/org.kde.material.so" \
             "$base/share/kstyle/themes/materialdesign.themerc"; do
      [ -e "$f" ] || continue
      if [ -w "$(dirname "$f")" ]; then rm -f "$f"; else $SUDO rm -f "$f"; fi
    done
  done
fi
rm -rf "$DIR/build"

echo "Material Dark dihapus."
