#ifndef THEMEAPI_HPP
#define THEMEAPI_HPP

#include <QObject>
#include <QFont>
#include <QColor>

class Theme : public QObject {
	Q_OBJECT
	Q_PROPERTY(QFont font READ font CONSTANT FINAL)
	Q_PROPERTY(qreal textSizeMicro READ textSizeMicro CONSTANT FINAL)
	Q_PROPERTY(qreal textSizeSmall READ textSizeSmall CONSTANT FINAL)
	Q_PROPERTY(qreal textSizeMedium READ textSizeMedium CONSTANT FINAL)
	Q_PROPERTY(qreal textSizeLarge READ textSizeLarge CONSTANT FINAL)
	Q_PROPERTY(qreal spacing READ spacing CONSTANT FINAL)
	Q_PROPERTY(qreal padding READ padding CONSTANT FINAL)
	Q_PROPERTY(qreal lineHeight READ lineHeight CONSTANT FINAL)
	Q_PROPERTY(qreal iconSize READ iconSize CONSTANT FINAL)
	Q_PROPERTY(qreal separator READ separator CONSTANT FINAL)
	Q_PROPERTY(qreal separatorThick READ separatorThick CONSTANT FINAL)
	Q_PROPERTY(QColor textColorPrimary READ textColorPrimary CONSTANT FINAL)
	Q_PROPERTY(QColor textColorSecondary READ textColorSecondary CONSTANT FINAL)
	Q_PROPERTY(QColor separatorColor READ separatorColor CONSTANT FINAL)
	Q_PROPERTY(QColor background READ background CONSTANT FINAL)
	Q_PROPERTY(QColor backgroundLight READ backgroundLight CONSTANT FINAL)
	Q_PROPERTY(QColor backgroundDark READ backgroundDark CONSTANT FINAL)
	Q_PROPERTY(QColor highlight READ highlight CONSTANT FINAL)
	Q_PROPERTY(QColor highlightLight READ highlightLight CONSTANT FINAL)
	Q_PROPERTY(QColor highlightDark READ highlightDark CONSTANT FINAL)
public:
	struct ThemeData {
		virtual ~ThemeData() {}
		QFont font;
		qreal textSizeMicro, textSizeSmall, textSizeMedium, textSizeLarge;
		qreal spacing, lineHeight, iconSize, separator, separatorThick, padding;
		QColor textColorPrimary, textColorSecondary;
		QColor backgroundLight, background, backgroundDark, separatorColor, highlight, highlightLight, highlightDark;
	};
	Theme(QObject *parent = nullptr);
#define DEC(p) static decltype(ThemeData::p) p() { return d->p; }
	DEC(font)
	DEC(textSizeMicro)
	DEC(textSizeSmall)
	DEC(textSizeMedium)
	DEC(textSizeLarge)
	DEC(spacing)
	DEC(lineHeight)
	DEC(iconSize)
	DEC(textColorPrimary)
	DEC(textColorSecondary)
	DEC(background)
	DEC(backgroundLight)
	DEC(backgroundDark)
	DEC(highlight)
	DEC(highlightLight)
	DEC(highlightDark)
	DEC(separatorColor)
	DEC(separator)
	DEC(separatorThick)
	DEC(padding)
#undef DEC
	static void initialize(const ThemeData *d);
	static const ThemeData *androidHoloLight();
private:
	static const ThemeData *d;
};

#endif // THEMEAPI_HPP
