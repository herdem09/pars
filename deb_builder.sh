#!/bin/bash
# Pars uygulaması için otomatik .deb paket oluşturucu

APP_NAME="pars"
VERSION="1.0"
ARCH=$(dpkg --print-architecture)
MAINTAINER="Moussa Sow <moussa@example.com>"
DESC="Pars Asistanı - Akıllı Uygulama"

# Çalışma dizini kontrolü
if [[ ! -f "$APP_NAME" ]]; then
    echo "❌ Hata: '$APP_NAME' adlı çalıştırılabilir dosya bulunamadı!"
    echo "Lütfen bu scripti, derlenmiş '$APP_NAME' dosyasının bulunduğu klasörde çalıştır."
    exit 1
fi

if [[ ! -f "/home/herdem/Masaüstü/pars_1.0/images/logo.png" ]]; then
    echo "❌ Hata: logo.png bulunamadı!"
    echo "Lütfen uygulama simgesi olarak kullanılacak logo.png dosyasını bu klasöre ekle."
    exit 1
fi

# Temizlik
rm -rf ${APP_NAME}_deb ${APP_NAME}_${VERSION}_${ARCH}.deb

echo "📦 Paket klasörü oluşturuluyor..."
mkdir -p ${APP_NAME}_deb/DEBIAN
mkdir -p ${APP_NAME}_deb/usr/bin
mkdir -p ${APP_NAME}_deb/usr/share/applications
mkdir -p ${APP_NAME}_deb/usr/share/icons/hicolor/256x256/apps

# Çalıştırılabilir dosya kopyala
cp $APP_NAME ${APP_NAME}_deb/usr/bin/
chmod 755 ${APP_NAME}_deb/usr/bin/$APP_NAME

# İkon kopyala
cp logo.png ${APP_NAME}_deb/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png

# .desktop dosyası oluştur
cat <<EOF > ${APP_NAME}_deb/usr/share/applications/${APP_NAME}.desktop
[Desktop Entry]
Name=${APP_NAME^}
Comment=${DESC}
Exec=${APP_NAME}
Icon=${APP_NAME}
Terminal=false
Type=Application
Categories=Utility;Application;
EOF

# control dosyası oluştur
cat <<EOF > ${APP_NAME}_deb/DEBIAN/control
Package: ${APP_NAME}
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: ${MAINTAINER}
Description: ${DESC}
EOF

# Paket oluştur
echo "🔧 Paket oluşturuluyor..."
dpkg-deb --build ${APP_NAME}_deb > /dev/null

if [[ -f "${APP_NAME}_deb.deb" ]]; then
    mv ${APP_NAME}_deb.deb ${APP_NAME}_${VERSION}_${ARCH}.deb
    echo "✅ Başarılı: ${APP_NAME}_${VERSION}_${ARCH}.deb oluşturuldu!"
    echo "Kurmak için:"
    echo "  sudo dpkg -i ${APP_NAME}_${VERSION}_${ARCH}.deb"
else
    echo "❌ Paket oluşturulamadı!"
    exit 1
fi
