/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "history/view/history_view_context_menu_fork.h"

#include "api/api_common.h"
#include "api/api_editing.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "core/fork_settings.h"
#include "data/data_document.h"
#include "data/data_peer.h"
#include "data/data_photo.h"
#include "data/data_session.h"
#include "history/history_item.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "window/window_session_controller.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/menu/menu_action.h"
#include "ui/widgets/menu/menu_common.h"
#include "ui/widgets/menu/menu_multiline_action.h"
#include "styles/style_chat.h"
#include "styles/style_giveaway.h"

namespace Fork {

namespace MediaReplacement {

struct Entry final {
	crl::time last = crl::time(0);
	MTPInputMedia media;
};

base::flat_map<Main::Session*, Entry> Medias;

bool HasInputMedia(not_null<HistoryItem*> item) {
	const auto it = Medias.find(&item->from()->session());
	return (it != end(Medias)) ? (it->second.last > 0) : false;
}

void UseInputMedia(
		not_null<HistoryItem*> item,
		Api::SendOptions options) {
	const auto it = Medias.find(&item->from()->session());
	if (it != end(Medias)) {
		if ((crl::now() - it->second.last) < 60000) {
			const auto &media = it->second.media;
			Api::Fork::EditMessageMedia(item, options, media, [](...){});
		} else {
			Medias.remove(&item->from()->session());
		}
	}
}

void RememberDocumentAsInputMedia(not_null<DocumentData*> document) {
	Medias[&document->session()] = Entry{
		.last = crl::now(),
		.media = MTP_inputMediaDocument(
			MTP_flags(0),
			document->mtpInput(),
			MTPint(),
			MTPstring()),
	};
}

void RememberPhotoAsInputMedia(not_null<PhotoData*> photo) {
	Medias[&photo->session()] = Entry{
		.last = crl::now(),
		.media = MTP_inputMediaPhoto(
			MTP_flags(0),
			photo->mtpInput(),
			MTPint()),
	};
}

} // namespace MediaReplacement

void AddReplaceMedia(
		not_null<Ui::PopupMenu*> menu,
		not_null<HistoryItem*> item,
		not_null<Window::SessionController*> controller) {
	if (!Core::App().settings().fork().addToMenuRememberMedia()) {
		return;
	}
	const auto media = item->media();
	const auto photo = media ? media->photo() : nullptr;
	const auto document = media ? media->document() : nullptr;

	const auto addAction = [&](QString &&s, Fn<void()> callback) {
		auto item = base::make_unique_q<Ui::Menu::MultilineAction>(
			menu,
			st::defaultMenu,
			st::historyHasCustomEmoji,
			st::historyHasCustomEmojiPosition,
			TextWithEntities{ std::move(s) });
		item->clicks() | rpl::start_with_next(callback, menu->lifetime());
		menu->addAction(std::move(item));
	};
	if (photo) {
		addAction(u"Remember photo"_q, [=] {
			MediaReplacement::RememberPhotoAsInputMedia(photo);
		});
	}
	if (document) {
		addAction(u"Remember document"_q, [=] {
			MediaReplacement::RememberDocumentAsInputMedia(document);
		});
	}
	if ((photo || document) && MediaReplacement::HasInputMedia(item)) {
		addAction(u"Replace media with remembered one"_q, [=] {
			MediaReplacement::UseInputMedia(item, {});
		});
	}
}

void AddSwapMedia(
		not_null<Ui::PopupMenu*> menu,
		not_null<HistoryItem*> item1,
		not_null<HistoryItem*> item2,
		not_null<Window::SessionController*> controller,
		Fn<void()> action) {
	if (!Core::App().settings().fork().addToMenuRememberMedia()) {
		return;
	}
	const auto media1 = item1->media();
	const auto photo1 = media1 ? media1->photo() : nullptr;
	const auto document1 = media1 ? media1->document() : nullptr;

	const auto media2 = item2->media();
	const auto photo2 = media2 ? media2->photo() : nullptr;
	const auto document2 = media2 ? media2->document() : nullptr;

	const auto addAction = [&](QString &&s, Fn<void()> callback) {
		auto item = base::make_unique_q<Ui::Menu::MultilineAction>(
			menu,
			st::defaultMenu,
			st::historyHasCustomEmoji,
			st::historyHasCustomEmojiPosition,
			TextWithEntities{ std::move(s) });
		item->clicks() | rpl::start_with_next(callback, menu->lifetime());
		menu->addAction(std::move(item));
	};
	if ((photo1 || document1) && (photo2 || document2)) {
		addAction(u"Try to swap media"_q, [=] {
			auto inputMedia1 = document1
				? MTP_inputMediaDocument(
					MTP_flags(0),
					document1->mtpInput(),
					MTPint(),
					MTPstring())
				: photo1
				? MTP_inputMediaPhoto(
					MTP_flags(0),
					photo1->mtpInput(),
					MTPint())
				: MTP_inputMediaEmpty();
			auto inputMedia2 = document2
				? MTP_inputMediaDocument(
					MTP_flags(0),
					document2->mtpInput(),
					MTPint(),
					MTPstring())
				: photo2
				? MTP_inputMediaPhoto(
					MTP_flags(0),
					photo2->mtpInput(),
					MTPint())
				: MTP_inputMediaEmpty();
			action();
			const auto o1 = Api::SendOptions{
				.scheduled = item1->isScheduled() ? item1->date() : 0,
			};
			const auto o2 = Api::SendOptions{
				.scheduled = item2->isScheduled() ? item2->date() : 0,
			};
			Api::Fork::EditMessageMedia(item1, o1, inputMedia2, [=](auto e) {
				controller->showToast("Message #1: " + e);
			});
			Api::Fork::EditMessageMedia(item2, o2, inputMedia1, [=](auto e) {
				controller->showToast("Message #2: " + e);
			});
		});
	}
}

crl::time DurationFromItem(HistoryItem *item) {
	const auto media = item ? item->media() : nullptr;
	const auto document = media ? media->document() : nullptr;
	return document ? document->duration() : crl::time(0);
}

crl::time DurationFromItem(
		FullMsgId itemId,
		not_null<Window::SessionController*> controller) {
	return DurationFromItem(controller->session().data().message(itemId));
}

void AddGroupSelected(
		not_null<Ui::PopupMenu*> menu,
		Fn<void(bool)> callback) {
	auto item = base::make_unique_q<Ui::Menu::Action>(
		menu.get(),
		menu->menu()->st(),
		Ui::Menu::CreateAction(
			menu.get(),
			tr::lng_context_group_items(tr::now),
			[=] { callback(false); }),
		&st::menuIconDockBounce,
		&st::menuIconDockBounce);

	{
		const auto rightButton = Ui::CreateChild<Ui::IconButton>(
			item.get(),
			st::startGiveawayBoxTitleClose);
		item->sizeValue(
		) | rpl::take(1) | rpl::start_with_next([=](const QSize &s) {
			rightButton->moveToLeft(
				s.width() - rightButton->width(),
				(s.height() - rightButton->height()) / 2);
			qDebug() << rightButton->geometry() << s;
		}, rightButton->lifetime());
		rightButton->setClickedCallback([=] {
			callback(true);
			menu->hideMenu(false);
		});
	}

	menu->addAction(std::move(item));
}

} // namespace Fork
