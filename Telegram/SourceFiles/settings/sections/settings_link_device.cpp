/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/sections/settings_link_device.h"

#include "settings/settings_common_session.h"

#include "apiwrap.h"
#include "base/timer.h"
#include "calls/calls_instance.h"
#include "calls/calls_video_bubble.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "settings/sections/settings_main.h"
#include "settings/settings_builder.h"
#include "tgcalls/VideoCaptureInterface.h"
#include "ui/boxes/confirm_box.h"
#include "ui/layers/generic_box.h"
#include "ui/text/text_utilities.h"
#include "ui/toast/toast.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "webrtc/webrtc_video_track.h"
#include "window/window_session_controller.h"
#include "styles/style_boxes.h"
#include "styles/style_info.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"

#include <zbar.h>

#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>

namespace Settings {
namespace {

using namespace Builder;

using VideoState = Webrtc::VideoState;
using VideoTrack = Webrtc::VideoTrack;

constexpr auto kQrScanInterval = crl::time(500);

struct QrResult {
	QString text;
	int qrCount = 0;
	int decodeErrors = 0;
	QString lastError;
};

[[nodiscard]] QrResult TryDecodeQr(const QImage &frame, bool mirror) {
	if (frame.isNull()) {
		return {};
	}
	const auto flipped = mirror
		? frame.mirrored(true, false)
		: frame;
	const auto gray = flipped.convertToFormat(QImage::Format_Grayscale8);
	const auto w = gray.width();
	const auto h = gray.height();

	auto *scanner = zbar::zbar_image_scanner_create();
	if (!scanner) {
		return {};
	}
	const auto scannerGuard = gsl::finally([&] {
		zbar::zbar_image_scanner_destroy(scanner);
	});
	zbar::zbar_image_scanner_set_config(
		scanner,
		zbar::ZBAR_QRCODE,
		zbar::ZBAR_CFG_ENABLE,
		1);

	auto *zimg = zbar::zbar_image_create();
	if (!zimg) {
		return {};
	}
	const auto imgGuard = gsl::finally([&] {
		zbar::zbar_image_destroy(zimg);
	});
	zbar::zbar_image_set_format(
		zimg,
		zbar_fourcc('Y', '8', '0', '0'));
	zbar::zbar_image_set_size(zimg, w, h);

	auto packed = QByteArray(w * h, Qt::Uninitialized);
	const auto stride = gray.bytesPerLine();
	if (stride == w) {
		memcpy(packed.data(), gray.constBits(), w * h);
	} else {
		for (auto y = 0; y < h; ++y) {
			memcpy(packed.data() + y * w, gray.constScanLine(y), w);
		}
	}
	zbar::zbar_image_set_data(
		zimg,
		packed.constData(),
		packed.size(),
		nullptr);

	zbar::zbar_scan_image(scanner, zimg);

	auto result = QrResult();
	const auto *sym = zbar::zbar_image_first_symbol(zimg);
	while (sym) {
		++result.qrCount;
		if (zbar::zbar_symbol_get_type(sym) == zbar::ZBAR_QRCODE) {
			auto text = QString::fromUtf8(
				zbar::zbar_symbol_get_data(sym),
				zbar::zbar_symbol_get_data_length(sym));
			if (text.startsWith(u"tg://login?token="_q)) {
				result.text = text;
				return result;
			}
			if (result.text.isEmpty()) {
				result.text = text;
			}
		}
		sym = zbar::zbar_symbol_next(sym);
	}
	return result;
}

[[nodiscard]] QByteArray ParseLoginToken(const QString &url) {
	const auto prefix = u"tg://login?token="_q;
	if (!url.startsWith(prefix)) {
		return {};
	}
	return QByteArray::fromBase64(
		url.mid(prefix.size()).toLatin1(),
		QByteArray::Base64UrlEncoding);
}

void AcceptLoginToken(
		not_null<Main::Session*> session,
		const QByteArray &token,
		Fn<void()> success,
		Fn<void(QString)> fail) {
	session->api().request(MTPauth_AcceptLoginToken(
		MTP_bytes(token)
	)).done([=](const MTPAuthorization &result) {
		success();
	}).fail([=](const MTP::Error &error) {
		fail(error.type());
	}).send();
}

void HandleQrToken(
		const QByteArray &token,
		not_null<Window::SessionController*> controller,
		not_null<Ui::VerticalLayout*> container,
		not_null<Ui::FlatLabel*> debugLabel,
		Fn<void()> stopScanning) {
	const auto session = &controller->session();
	const auto weak = QPointer<Ui::VerticalLayout>(container.get());
	controller->show(Ui::MakeConfirmBox({
		.text = tr::lng_settings_link_device_confirm(tr::now),
		.confirmed = [=](Fn<void()> close) {
			AcceptLoginToken(session, token, [=] {
				close();
				if (weak) {
					stopScanning();
					Ui::Toast::Show(
						weak.data(),
						tr::lng_settings_link_device_success(tr::now));
				}
			}, [=](const QString &error) {
				close();
				if (weak) {
					stopScanning();
					Ui::Toast::Show(
						weak.data(),
						tr::lng_settings_link_device_error(
							tr::now,
							lt_error,
							error));
				}
			});
		},
		.cancelled = [=](Fn<void()> close) {
			close();
			stopScanning();
		},
	}));
}

void SetupLinkDeviceContent(
		not_null<Ui::VerticalLayout*> container,
		not_null<Window::SessionController*> controller) {
	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			tr::lng_settings_link_device_privacy(),
			st::boxDividerLabel),
		st::boxRowPadding);

	Ui::AddSkip(container);
	Ui::AddDivider(container);
	Ui::AddSkip(container);

	auto &lifetime = container->lifetime();

	const auto scanning = lifetime.make_state<rpl::variable<bool>>(false);
	const auto capturerOwner = lifetime.make_state<
		std::shared_ptr<tgcalls::VideoCaptureInterface>>();
	const auto track = lifetime.make_state<VideoTrack>(
		VideoState::Inactive);
	const auto lastDecoded = lifetime.make_state<QString>();
	const auto scanTimer = lifetime.make_state<base::Timer>();

	const auto bubbleWrap = container->add(
		object_ptr<Ui::RpWidget>(container));
	const auto bubble = lifetime.make_state<::Calls::VideoBubble>(
		bubbleWrap,
		track);

	const auto padding = st::settingsButtonNoIcon.padding.left();
	const auto top = st::boxRoundShadow.extend.top();
	const auto bottom = st::boxRoundShadow.extend.bottom();

	auto frameSize = track->renderNextFrame(
	) | rpl::map([=] {
		return track->frameSize();
	}) | rpl::filter([=](QSize size) {
		return !size.isEmpty();
	});
	auto bubbleWidth = bubbleWrap->widthValue(
	) | rpl::filter([=](int width) {
		return width > 2 * padding + 1;
	});
	rpl::combine(
		std::move(bubbleWidth),
		std::move(frameSize)
	) | rpl::on_next([=](int width, QSize frame) {
		const auto useWidth = (width - 2 * padding);
		const auto useHeight = std::min(
			((useWidth * frame.height()) / frame.width()),
			(useWidth * 480) / 640);
		bubbleWrap->resize(width, top + useHeight + bottom);
		bubble->updateGeometry(
			::Calls::VideoBubble::DragMode::None,
			QRect(padding, top, useWidth, useHeight));
		bubbleWrap->update();
	}, bubbleWrap->lifetime());

	bubbleWrap->resize(bubbleWrap->width(), 0);

	Ui::AddSkip(container);

	const auto statusLabel = container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			tr::lng_settings_link_device_scanning(),
			st::boxDividerLabel),
		st::boxRowPadding);
	statusLabel->setVisible(false);

	Ui::AddSkip(container);

	const auto buttonPadding = st::boxRowPadding;
	const auto buttonPaddingH = buttonPadding.left() + buttonPadding.right();
	const auto setupFullWidth = [=](not_null<Ui::RoundButton*> btn) {
		btn->setFullRadius(true);
		rpl::combine(
			container->widthValue(),
			btn->sizeValue()
		) | rpl::on_next([=](int w, QSize) {
			const auto target = w - buttonPaddingH;
			if (btn->width() != target) {
				btn->resize(target, btn->height());
			}
		}, btn->lifetime());
	};

	const auto debugWrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::FlatLabel>>(
			container,
			object_ptr<Ui::FlatLabel>(
				container,
				rpl::single(QString()),
				st::boxDividerLabel),
			st::boxRowPadding));
	debugWrap->toggle(false, anim::type::instant);
	const auto debugLabel = debugWrap->entity();

	const auto debugButton = container->add(
		object_ptr<Ui::LinkButton>(
			container,
			QString("debug")),
		st::boxRowPadding);

	debugButton->setClickedCallback([=] {
		debugWrap->toggle(
			!debugWrap->toggled(),
			anim::type::normal);
	});

	Ui::AddSkip(container);

	const auto toggleButton = container->add(
		object_ptr<Ui::RoundButton>(
			container,
			tr::lng_settings_link_device_start(),
			st::defaultActiveButton),
		buttonPadding);
	setupFullWidth(toggleButton);

	Ui::AddSkip(container);

	const auto clipboardButton = container->add(
		object_ptr<Ui::RoundButton>(
			container,
			tr::lng_settings_link_device_clipboard(),
			st::defaultActiveButton),
		buttonPadding);
	setupFullWidth(clipboardButton);

	Ui::AddSkip(container);
	Ui::AddSkip(container);

	const auto stopScanning = [=] {
		scanTimer->cancel();
		track->setState(VideoState::Inactive);
		*capturerOwner = nullptr;
		*scanning = false;
		bubbleWrap->resize(bubbleWrap->width(), 0);
	};

	const auto startScanning = [=] {
		if (*capturerOwner) {
			return;
		}
		*capturerOwner = Core::App().calls().getVideoCapture(
			Core::App().settings().cameraDeviceId(),
			false);
		(*capturerOwner)->setPreferredAspectRatio(0.);
		track->setState(VideoState::Active);
		(*capturerOwner)->setState(tgcalls::VideoState::Active);
		(*capturerOwner)->setOutput(track->sink());
		*scanning = true;
		*lastDecoded = QString();

		scanTimer->setCallback([=] {
			if (!scanning->current()) {
				return;
			}
			const auto frame = track->frame({});
			if (frame.isNull()) {
				debugLabel->setText(
					QString("Debug: frame is null"));
				return;
			}

			auto result = TryDecodeQr(frame, true);
			if (result.text.isEmpty() && result.qrCount == 0) {
				result = TryDecodeQr(frame, false);
			}
			track->markFrameShown();

			debugLabel->setText(QString(
				"Debug: %1x%2 fmt=%3 qrs=%4 err=%5 decoded=\"%6\" lastErr=\"%7\""
			).arg(frame.width()
			).arg(frame.height()
			).arg(frame.format()
			).arg(result.qrCount
			).arg(result.decodeErrors
			).arg(result.text.left(60)
			).arg(result.lastError));

			const auto decoded = result.text;
			if (decoded.isEmpty() || decoded == *lastDecoded) {
				return;
			}
			if (!decoded.startsWith(u"tg://login?token="_q)) {
				return;
			}
			*lastDecoded = decoded;

			const auto token = ParseLoginToken(decoded);
			if (token.isEmpty()) {
				return;
			}

			scanTimer->cancel();
			HandleQrToken(token, controller, container, debugLabel, stopScanning);
		});
		scanTimer->callEach(kQrScanInterval);
	};

	scanning->value(
	) | rpl::on_next([=](bool active) {
		statusLabel->setVisible(active);
		if (active) {
			toggleButton->setText(tr::lng_settings_link_device_stop());
		} else {
			toggleButton->setText(tr::lng_settings_link_device_start());
		}
		container->resizeToWidth(container->width());
	}, container->lifetime());

	toggleButton->setClickedCallback([=] {
		if (scanning->current()) {
			stopScanning();
		} else {
			startScanning();
		}
	});

	clipboardButton->setClickedCallback([=] {
		const auto clipboard = QGuiApplication::clipboard();
		const auto image = clipboard->image();
		if (image.isNull()) {
			debugLabel->setText(
				QString("Debug: clipboard has no image"));
			return;
		}

		auto result = TryDecodeQr(image, false);
		debugLabel->setText(QString(
			"Debug: clip %1x%2 fmt=%3 qrs=%4 err=%5 decoded=\"%6\" lastErr=\"%7\""
		).arg(image.width()
		).arg(image.height()
		).arg(image.format()
		).arg(result.qrCount
		).arg(result.decodeErrors
		).arg(result.text.left(60)
		).arg(result.lastError));

		if (!result.text.startsWith(u"tg://login?token="_q)) {
			return;
		}

		const auto token = ParseLoginToken(result.text);
		if (token.isEmpty()) {
			return;
		}

		HandleQrToken(token, controller, container, debugLabel, stopScanning);
	});
}

class LinkDevice : public Section<LinkDevice> {
public:
	LinkDevice(
		QWidget *parent,
		not_null<Window::SessionController*> controller);
	~LinkDevice();

	[[nodiscard]] rpl::producer<QString> title() override;

private:
	void setupContent();

};

LinkDevice::LinkDevice(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

LinkDevice::~LinkDevice() = default;

rpl::producer<QString> LinkDevice::title() {
	return tr::lng_settings_link_device();
}

void LinkDevice::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

	const SectionBuildMethod buildMethod = [](
			not_null<Ui::VerticalLayout*> container,
			not_null<Window::SessionController*> controller,
			Fn<void(Type)> showOther,
			rpl::producer<> showFinished) {
		auto builder = SectionBuilder(WidgetContext{
			.container = container,
			.controller = controller,
			.showOther = std::move(showOther),
		});

		builder.addSkip();
		builder.add([](const BuildContext &ctx) {
			if (const auto *widget = std::get_if<WidgetContext>(&ctx)) {
				SetupLinkDeviceContent(
					widget->container,
					widget->controller);
			}
		});
	};

	build(content, buildMethod);
	Ui::ResizeFitChild(this, content);
}

const auto kMeta = BuildHelper({
	.id = LinkDevice::Id(),
	.parentId = MainId(),
	.title = &tr::lng_settings_link_device,
	.icon = &st::menuIconQrCode,
}, [](SectionBuilder &builder) {
	builder.addSkip();
	builder.addDividerText(tr::lng_settings_link_device_privacy());
});

} // namespace

Type LinkDeviceId() {
	return LinkDevice::Id();
}

} // namespace Settings
