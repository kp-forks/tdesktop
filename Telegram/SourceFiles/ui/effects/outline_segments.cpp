/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "ui/effects/outline_segments.h"

namespace Ui {
namespace {

constexpr std::array<float, 90> Radiuses = {{ 1, 1, 1, 1.001, 1.002, 1.003, 1.005, 1.007, 1.009, 1.012, 1.015, 1.018, 1.022, 1.026, 1.03, 1.035, 1.04, 1.045, 1.051, 1.057, 1.064, 1.071, 1.078, 1.086, 1.094, 1.103, 1.11201, 1.12201, 1.13201, 1.14301, 1.15401, 1.16601, 1.17801, 1.19201, 1.20601, 1.22001, 1.23501, 1.25101, 1.26801, 1.28601, 1.30501, 1.32402, 1.34502, 1.36602, 1.38902, 1.41302, 1.38902, 1.36602, 1.34502, 1.32402, 1.30501, 1.28601, 1.26801, 1.25101, 1.23501, 1.22001, 1.20601, 1.19201, 1.17801, 1.16601, 1.15401, 1.14301, 1.13201, 1.12201, 1.11201, 1.103, 1.094, 1.086, 1.078, 1.071, 1.064, 1.057, 1.051, 1.045, 1.04, 1.035, 1.03, 1.026, 1.022, 1.018, 1.015, 1.012, 1.009, 1.007, 1.005, 1.003, 1.002, 1.001, 1, 1 }};
struct Corner {
	short xSign = 0;
	short ySign = 0;
	int angle = 0;
};
constexpr std::array<Corner, 4> Corners = {{
	{ -1, -1, 225 },
	{ -1 ,1, 315 },
	{ 1, 1, 405 },
	{ 1, -1, 495 },
}};

void DrawArcFork(
		QPainter &p,
		const QRectF &oval,
		int startAngle,
		int spanAngle) {
	startAngle /= 16;
	spanAngle /= 16;
	startAngle += 90;
	const auto a = float64(startAngle);
	const auto b = float64(startAngle + spanAngle);
	const auto r = oval.width() / 2.;

	const auto aRad = M_PI / 2. - a * M_PI / 180.0;
	const auto bRad = M_PI / 2. - b * M_PI / 180.0;

	const auto aR = Radiuses[(int)(a - ((int)(a / 90.0) * 90))] * r;
	const auto aX = aR * std::cos(aRad);
	const auto aY = aR * std::sin(aRad);

	const auto bR = Radiuses[(int)(b - ((int)(b / 90.0) * 90))] * r;
	const auto bX = bR * std::cos(bRad);
	const auto bY = bR * std::sin(bRad);

	auto path = QPainterPath();

	const auto shiftX = oval.x() + r;
	const auto shiftY = oval.y() + r;

	path.moveTo(shiftX + aX, shiftY + aY);
	for (const auto &corner : Corners) {
		if (corner.angle > a && corner.angle < b) {
			path.lineTo(shiftX + corner.xSign * r, shiftY + corner.ySign * r);
		}
	}
	path.lineTo(shiftX + bX, shiftY + bY);

	p.drawPath(path);
}

} // namespace

void PaintOutlineSegments(
		QPainter &p,
		QRectF ellipse,
		const std::vector<OutlineSegment> &segments,
		float64 fromFullProgress) {
	Expects(!segments.empty());

	p.setBrush(Qt::NoBrush);
	const auto count = std::min(int(segments.size()), kOutlineSegmentsMax);
	if (count == 1) {
		p.setPen(QPen(segments.front().brush, segments.front().width));
		p.drawEllipse(ellipse);
		return;
	}
	const auto small = 160;
	const auto full = arc::kFullLength;
	const auto separator = (full > 1.1 * small * count)
		? small
		: (full / (count * 1.1));
	const auto left = full - (separator * count);
	const auto length = left / float64(count);
	const auto spin = separator * (1. - fromFullProgress);

	auto start = 0. + (arc::kQuarterLength + (separator / 2)) + (3. * spin);
	auto pen = QPen(
		segments.back().brush,
		segments.back().width,
		Qt::SolidLine,
		Qt::RoundCap);
	p.setPen(pen);
	for (auto i = 0; i != count;) {
		const auto &segment = segments[count - (++i)];
		if (!segment.width) {
			start += length + separator;
			continue;
		} else if (pen.brush() != segment.brush
			|| pen.widthF() != segment.width) {
			pen = QPen(
				segment.brush,
				segment.width,
				Qt::SolidLine,
				Qt::RoundCap);
			p.setPen(pen);
		}
		const auto from = int(base::SafeRound(start));
		const auto till = start + length;
		auto added = spin;
		for (; i != count;) {
			start += length + separator;
			const auto &next = segments[count - (++i)];
			if (next.width) {
				--i;
				break;
			}
			added += (separator + length) * (1. - fromFullProgress);
		}
		if (style::SquareUserpics()) {
		DrawArcFork(p, ellipse, from, int(base::SafeRound(till + added)) - from);
		} else {
		p.drawArc(ellipse, from, int(base::SafeRound(till + added)) - from);
		}
	}
}

QLinearGradient UnreadStoryOutlineGradient(QRectF rect) {
	auto result = QLinearGradient(rect.topRight(), rect.bottomLeft());
	result.setStops({
		{ 0., st::groupCallLive1->c },
		{ 1., st::groupCallMuted1->c },
	});
	return result;
}

} // namespace Ui
