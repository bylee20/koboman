#include "themeapi.hpp"
#include "utility.hpp"
#include <QGuiApplication>

static inline qreal d2p(qreal dp) { return Utility::dpToPx(dp); }

const Theme::ThemeData *Theme::d = nullptr;

struct AndroidTheme : public Theme::ThemeData {
	AndroidTheme() {
		textSizeMicro = d2p(12);
		textSizeSmall = d2p(14);
		textSizeMedium = d2p(18);
		textSizeLarge = d2p(22);

		font = qApp->font();
		font.setPixelSize(textSizeMedium);
#ifdef Q_OS_ANDROID
		const auto families = QFontDatabase().families(QFontDatabase::Korean);
		if (!families.isEmpty())
			font.setFamily(families.last());
#endif

		padding = d2p(16);
		spacing = d2p(8);
		lineHeight = d2p(48);
		iconSize = d2p(32);
		separator = d2p(1);
		separatorThick = d2p(2);

		textColorPrimary = "#000000";
		textColorSecondary = "#323232";
		background = "#fff3f3f3";
		backgroundLight = "#ffffff";
		backgroundDark = "#828282";
		highlight = "#ff33b5e5";
		highlightDark = "#ff0099cc";
		highlightLight = "#ff95d6ee";
		separatorColor = "#d2d2d2";
	}
};

const Theme::ThemeData *Theme::androidHoloLight() {
	static AndroidTheme theme; return &theme;
}

Theme::Theme(QObject *parent)
: QObject(parent) {

}

void Theme::initialize(const ThemeData *d) {
	Theme::d = d;
	qApp->setFont(d->font);
}
