/*
Author: 23rd.
*/
#include "core/fork_settings.h"

#include "storage/serialize_common.h"

namespace Core {

namespace {

constexpr auto kDefaultStickerSize = 256;

bool StaticPrimaryUnmutedMessages = false;

} // namespace

ForkSettings::ForkSettings() {
}

bool ForkSettings::PrimaryUnmutedMessages() {
	return StaticPrimaryUnmutedMessages;
}

QByteArray ForkSettings::serialize() const {

	auto size = sizeof(qint32) * 4
		+ Serialize::stringSize(_uriScheme)
		+ Serialize::stringSize(_searchEngineUrl)
		+ sizeof(qint32) * 12
		+ Serialize::stringSize(_platformBot);

	auto result = QByteArray();
	result.reserve(size);
	{
		QDataStream stream(&result, QIODevice::WriteOnly);
		stream.setVersion(QDataStream::Qt_5_1);
		stream
			<< qint32(_squareUserpics ? 1 : 0)
			<< qint32(_audioFade ? 1 : 0)
			<< qint32(_askUriScheme ? 1 : 0)
			<< qint32(_lastSeenInDialogs ? 1 : 0)
			<< _uriScheme
			<< _searchEngineUrl
			<< qint32(_searchEngine ? 1 : 0)
			<< qint32(_allRecentStickers ? 1 : 0)
			<< qint32(_customStickerSize)
			<< qint32(_useBlackTrayIcon ? 1 : 0)
			<< qint32(_useOriginalTrayIcon ? 1 : 0)
			<< qint32(_autoSubmitPasscode ? 1 : 0)
			<< qint32(_emojiPopupOnClick ? 1 : 0)
			<< qint32(_mentionByNameDisabled ? 1 : 0)
			<< qint32(_primaryUnmutedMessages ? 1 : 0)
			<< qint32(_addToMenuRememberMedia ? 1 : 0)
			<< qint32(_hideAllChatsTab ? 1 : 0)
			<< qint32(_globalSearchDisabled ? 1 : 0)
			<< qint32(_thirdButtonTopBar ? 1 : 0)
			<< qint32(_skipShareFromBot ? 1 : 0)
			<< _platformBot
			;
	}
	return result;
}

void ForkSettings::addFromSerialized(const QByteArray &serialized) {
	if (serialized.isEmpty()) {
		return;
	}

	QDataStream stream(serialized);
	stream.setVersion(QDataStream::Qt_5_1);

	qint32 squareUserpics = _squareUserpics;
	qint32 audioFade = _audioFade;
	qint32 askUriScheme = _askUriScheme;
	qint32 lastSeenInDialogs = _lastSeenInDialogs;
	QString uriScheme = _uriScheme;
	QString searchEngineUrl = _searchEngineUrl;
	qint32 searchEngine = _searchEngine;
	qint32 allRecentStickers = _allRecentStickers;
	qint32 customStickerSize = _customStickerSize;
	qint32 useBlackTrayIcon = _useBlackTrayIcon;
	qint32 useOriginalTrayIcon = _useOriginalTrayIcon;
	qint32 autoSubmitPasscode = _autoSubmitPasscode;
	qint32 emojiPopupOnClick = _emojiPopupOnClick;
	qint32 mentionByNameDisabled = _mentionByNameDisabled;
	qint32 primaryUnmutedMessages = _primaryUnmutedMessages;
	qint32 addToMenuRememberMedia = _addToMenuRememberMedia;
	qint32 hideAllChatsTab = _hideAllChatsTab;
	qint32 globalSearchDisabled = _globalSearchDisabled;
	qint32 thirdButtonTopBar = _thirdButtonTopBar;
	qint32 skipShareFromBot = _skipShareFromBot;
	QString platformBot = _platformBot;

	if (!stream.atEnd()) {
		stream
			>> squareUserpics
			>> audioFade
			>> askUriScheme
			>> lastSeenInDialogs
			>> uriScheme
			>> searchEngineUrl
			>> searchEngine
			>> allRecentStickers
			>> customStickerSize
			>> useBlackTrayIcon
			>> useOriginalTrayIcon
			>> autoSubmitPasscode
			>> emojiPopupOnClick
			>> mentionByNameDisabled
			>> primaryUnmutedMessages
			;
	}
	if (!stream.atEnd()) {
		stream >> addToMenuRememberMedia;
	}
	if (!stream.atEnd()) {
		stream >> hideAllChatsTab;
	}
	if (!stream.atEnd()) {
		stream >> globalSearchDisabled;
	}
	if (!stream.atEnd()) {
		stream >> thirdButtonTopBar;
	}
	if (!stream.atEnd()) {
		stream >> skipShareFromBot;
	}
	if (!stream.atEnd()) {
		stream >> platformBot;
	}
	if (stream.status() != QDataStream::Ok) {
		LOG(("App Error: "
			"Bad data for Core::ForkSettings::constructFromSerialized()"));
		return;
	}
	_squareUserpics = (squareUserpics == 1);
	_audioFade = (audioFade == 1);
	_askUriScheme = (askUriScheme == 1);
	_lastSeenInDialogs = (lastSeenInDialogs == 1);
	_uriScheme = uriScheme;
	_searchEngineUrl = searchEngineUrl;
	_searchEngine = (searchEngine == 1);
	_allRecentStickers = (allRecentStickers == 1);
	_customStickerSize = customStickerSize
		? std::clamp(customStickerSize, 50, kDefaultStickerSize)
		: kDefaultStickerSize;
	_useBlackTrayIcon = (useBlackTrayIcon == 1);
	_useOriginalTrayIcon = (useOriginalTrayIcon == 1);
	_autoSubmitPasscode = (autoSubmitPasscode == 1);
	_emojiPopupOnClick = (emojiPopupOnClick == 1);
	_mentionByNameDisabled = (mentionByNameDisabled == 1);
	setPrimaryUnmutedMessages(primaryUnmutedMessages == 1);
	_addToMenuRememberMedia = (addToMenuRememberMedia == 1);
	_hideAllChatsTab = (hideAllChatsTab == 1);
	_globalSearchDisabled = (globalSearchDisabled == 1);
	_thirdButtonTopBar = (thirdButtonTopBar == 1);
	_skipShareFromBot = (skipShareFromBot == 1);
	_platformBot = std::move(platformBot);
}

void ForkSettings::resetOnLastLogout() {
	_squareUserpics = false;
	_audioFade = true;
	_askUriScheme = false;
	_uriScheme = QString();
	_lastSeenInDialogs = false;
	_searchEngineUrl = qsl("https://dgg.gg/%q");
	_searchEngine = false;
	_allRecentStickers = true;
	_customStickerSize = kDefaultStickerSize;
	_useOriginalTrayIcon = false;
	_autoSubmitPasscode = false;
	_emojiPopupOnClick = false;
	_mentionByNameDisabled = false;
	setPrimaryUnmutedMessages(false);
	_addToMenuRememberMedia = false;
	_hideAllChatsTab = false;
	_globalSearchDisabled = false;
	_thirdButtonTopBar = false;
	_skipShareFromBot = false;
	_platformBot = QString();
}

[[nodiscard]] bool ForkSettings::primaryUnmutedMessages() const {
	return _primaryUnmutedMessages;
}
void ForkSettings::setPrimaryUnmutedMessages(bool newValue) {
	StaticPrimaryUnmutedMessages = newValue;
	_primaryUnmutedMessages = newValue;
}

[[nodiscard]] bool ForkSettings::addToMenuRememberMedia() const {
	return _addToMenuRememberMedia;
}
void ForkSettings::setAddToMenuRememberMedia(bool newValue) {
	_addToMenuRememberMedia = newValue;
}

[[nodiscard]] bool ForkSettings::hideAllChatsTab() const {
	return _hideAllChatsTab;
}
void ForkSettings::setHideAllChatsTab(bool newValue) {
	_hideAllChatsTab = newValue;
}

[[nodiscard]] bool ForkSettings::globalSearchDisabled() const {
	return _globalSearchDisabled;
}
void ForkSettings::setGlobalSearchDisabled(bool newValue) {
	_globalSearchDisabled = newValue;
}

[[nodiscard]] bool ForkSettings::thirdButtonTopBar() const {
	return _thirdButtonTopBar;
}
void ForkSettings::setThirdButtonTopBar(bool newValue) {
	_thirdButtonTopBar = newValue;
}

[[nodiscard]] bool ForkSettings::skipShareFromBot() const {
	return _skipShareFromBot;
}
void ForkSettings::setSkipShareFromBot(bool newValue) {
	_skipShareFromBot = newValue;
}

[[nodiscard]] QString ForkSettings::platformBot() const {
	if (_platformBot.isEmpty()) {
		return u"tdesktop"_q;
	}
	return _platformBot;
}
void ForkSettings::setPlatformBot(QString newValue) {
	_platformBot = newValue;
}

} // namespace Core
