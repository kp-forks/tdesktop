/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "window/themes/window_themes_embedded.h"

enum class SendFilesWay;
enum class RectPart;

namespace Ui {
enum class InputSubmitSettings;
} // namespace Ui

namespace Window {
enum class Column;
} // namespace Window

namespace Core {

class Settings final {
public:
	enum class ScreenCorner {
		TopLeft = 0,
		TopRight = 1,
		BottomRight = 2,
		BottomLeft = 3,
	};

	static constexpr auto kDefaultVolume = 0.9;

	Settings();

	[[nodiscard]] static bool IsLeftCorner(ScreenCorner corner) {
		return (corner == ScreenCorner::TopLeft)
			|| (corner == ScreenCorner::BottomLeft);
	}
	[[nodiscard]] static bool IsTopCorner(ScreenCorner corner) {
		return (corner == ScreenCorner::TopLeft)
			|| (corner == ScreenCorner::TopRight);
	}

	[[nodiscard]] QByteArray serialize() const;
	void addFromSerialized(const QByteArray &serialized);

	[[nodiscard]] bool chatWide() const;
	[[nodiscard]] bool adaptiveForWide() const {
		return _adaptiveForWide;
	}
	void setAdaptiveForWide(bool value) {
		_adaptiveForWide = value;
	}
	[[nodiscard]] bool moderateModeEnabled() const {
		return _moderateModeEnabled;
	}
	void setModerateModeEnabled(bool value) {
		_moderateModeEnabled = value;
	}
	[[nodiscard]] float64 songVolume() const {
		return _songVolume.current();
	}
	[[nodiscard]] rpl::producer<float64> songVolumeChanges() const {
		return _songVolume.changes();
	}
	void setSongVolume(float64 value) {
		_songVolume = value;
	}
	[[nodiscard]] float64 videoVolume() const {
		return _videoVolume.current();
	}
	[[nodiscard]] rpl::producer<float64> videoVolumeChanges() const {
		return _videoVolume.changes();
	}
	void setVideoVolume(float64 value) {
		_videoVolume = value;
	}
	[[nodiscard]] bool askDownloadPath() const {
		return _askDownloadPath;
	}
	void setAskDownloadPath(bool value) {
		_askDownloadPath = value;
	}
	[[nodiscard]] QString downloadPath() const {
		return _downloadPath.current();
	}
	[[nodiscard]] rpl::producer<QString> downloadPathValue() const {
		return _downloadPath.value();
	}
	void setDownloadPath(const QString &value) {
		_downloadPath = value;
	}
	[[nodiscard]] QByteArray downloadPathBookmark() const {
		return _downloadPathBookmark;
	}
	void setDownloadPathBookmark(const QByteArray &value) {
		_downloadPathBookmark = value;
	}
	[[nodiscard]] bool voiceMsgPlaybackDoubled() const {
		return _voiceMsgPlaybackDoubled;
	}
	void setVoiceMsgPlaybackDoubled(bool value) {
		_voiceMsgPlaybackDoubled = value;
	}
	[[nodiscard]] bool soundNotify() const {
		return _soundNotify;
	}
	void setSoundNotify(bool value) {
		_soundNotify = value;
	}
	[[nodiscard]] bool desktopNotify() const {
		return _desktopNotify;
	}
	void setDesktopNotify(bool value) {
		_desktopNotify = value;
	}
	[[nodiscard]] bool flashBounceNotify() const {
		return _flashBounceNotify;
	}
	void setFlashBounceNotify(bool value) {
		_flashBounceNotify = value;
	}
	[[nodiscard]] DBINotifyView notifyView() const {
		return _notifyView;
	}
	void setNotifyView(DBINotifyView value) {
		_notifyView = value;
	}
	[[nodiscard]] bool nativeNotifications() const {
		return _nativeNotifications;
	}
	void setNativeNotifications(bool value) {
		_nativeNotifications = value;
	}
	[[nodiscard]] int notificationsCount() const {
		return _notificationsCount;
	}
	void setNotificationsCount(int value) {
		_notificationsCount = value;
	}
	[[nodiscard]] ScreenCorner notificationsCorner() const {
		return _notificationsCorner;
	}
	void setNotificationsCorner(ScreenCorner corner) {
		_notificationsCorner = corner;
	}
	[[nodiscard]] bool includeMutedCounter() const {
		return _includeMutedCounter;
	}
	void setIncludeMutedCounter(bool value) {
		_includeMutedCounter = value;
	}
	[[nodiscard]] bool countUnreadMessages() const {
		return _countUnreadMessages;
	}
	void setCountUnreadMessages(bool value) {
		_countUnreadMessages = value;
	}
	void setNotifyAboutPinned(bool notify) {
		_notifyAboutPinned = notify;
	}
	[[nodiscard]] bool notifyAboutPinned() const {
		return _notifyAboutPinned.current();
	}
	[[nodiscard]] rpl::producer<bool> notifyAboutPinnedChanges() const {
		return _notifyAboutPinned.changes();
	}
	[[nodiscard]] int autoLock() const {
		return _autoLock;
	}
	void setAutoLock(int value) {
		_autoLock = value;
	}
	[[nodiscard]] QString callOutputDeviceID() const {
		return _callOutputDeviceID;
	}
	void setCallOutputDeviceID(const QString &value) {
		_callOutputDeviceID = value;
	}
	[[nodiscard]] QString callInputDeviceID() const {
		return _callInputDeviceID;
	}
	void setCallInputDeviceID(const QString &value) {
		_callInputDeviceID = value;
	}
	[[nodiscard]] int callOutputVolume() const {
		return _callOutputVolume;
	}
	void setCallOutputVolume(int value) {
		_callOutputVolume = value;
	}
	[[nodiscard]] int callInputVolume() const {
		return _callInputVolume;
	}
	void setCallInputVolume(int value) {
		_callInputVolume = value;
	}
	[[nodiscard]] bool callAudioDuckingEnabled() const {
		return _callAudioDuckingEnabled;
	}
	void setCallAudioDuckingEnabled(bool value) {
		_callAudioDuckingEnabled = value;
	}
	[[nodiscard]] Window::Theme::AccentColors &themesAccentColors() {
		return _themesAccentColors;
	}
	void setThemesAccentColors(Window::Theme::AccentColors &&colors) {
		_themesAccentColors = std::move(colors);
	}
	void setLastSeenWarningSeen(bool lastSeenWarningSeen) {
		_lastSeenWarningSeen = lastSeenWarningSeen;
	}
	[[nodiscard]] bool lastSeenWarningSeen() const {
		return _lastSeenWarningSeen;
	}
	void setSendFilesWay(SendFilesWay way) {
		_sendFilesWay = way;
	}
	[[nodiscard]] SendFilesWay sendFilesWay() const {
		return _sendFilesWay;
	}
	void setSendSubmitWay(Ui::InputSubmitSettings value) {
		_sendSubmitWay = value;
	}
	[[nodiscard]] Ui::InputSubmitSettings sendSubmitWay() const {
		return _sendSubmitWay;
	}
	void setSoundOverride(const QString &key, const QString &path) {
		_soundOverrides.emplace(key, path);
	}
	void clearSoundOverrides() {
		_soundOverrides.clear();
	}
	[[nodiscard]] QString getSoundPath(const QString &key) const;

	[[nodiscard]] bool exeLaunchWarning() const {
		return _exeLaunchWarning;
	}
	void setExeLaunchWarning(bool warning) {
		_exeLaunchWarning = warning;
	}
	[[nodiscard]] bool loopAnimatedStickers() const {
		return _loopAnimatedStickers;
	}
	void setLoopAnimatedStickers(bool value) {
		_loopAnimatedStickers = value;
	}
	void setLargeEmoji(bool value) {
		_largeEmoji = value;
	}
	[[nodiscard]] bool largeEmoji() const {
		return _largeEmoji.current();
	}
	[[nodiscard]] rpl::producer<bool> largeEmojiValue() const {
		return _largeEmoji.value();
	}
	[[nodiscard]] rpl::producer<bool> largeEmojiChanges() const {
		return _largeEmoji.changes();
	}
	void setReplaceEmoji(bool value) {
		_replaceEmoji = value;
	}
	[[nodiscard]] bool replaceEmoji() const {
		return _replaceEmoji.current();
	}
	[[nodiscard]] rpl::producer<bool> replaceEmojiValue() const {
		return _replaceEmoji.value();
	}
	[[nodiscard]] rpl::producer<bool> replaceEmojiChanges() const {
		return _replaceEmoji.changes();
	}
	[[nodiscard]] bool suggestEmoji() const {
		return _suggestEmoji;
	}
	void setSuggestEmoji(bool value) {
		_suggestEmoji = value;
	}
	[[nodiscard]] bool suggestStickersByEmoji() const {
		return _suggestStickersByEmoji;
	}
	void setSuggestStickersByEmoji(bool value) {
		_suggestStickersByEmoji = value;
	}

	void setSpellcheckerEnabled(bool value) {
		_spellcheckerEnabled = value;
	}
	bool spellcheckerEnabled() const {
		return _spellcheckerEnabled.current();
	}
	rpl::producer<bool> spellcheckerEnabledValue() const {
		return _spellcheckerEnabled.value();
	}
	rpl::producer<bool> spellcheckerEnabledChanges() const {
		return _spellcheckerEnabled.changes();
	}

	void setDictionariesEnabled(std::vector<int> dictionaries) {
		_dictionariesEnabled = std::move(dictionaries);
	}

	std::vector<int> dictionariesEnabled() const {
		return _dictionariesEnabled.current();
	}

	rpl::producer<std::vector<int>> dictionariesEnabledChanges() const {
		return _dictionariesEnabled.changes();
	}

	void setAutoDownloadDictionaries(bool value) {
		_autoDownloadDictionaries = value;
	}
	bool autoDownloadDictionaries() const {
		return _autoDownloadDictionaries.current();
	}
	rpl::producer<bool> autoDownloadDictionariesValue() const {
		return _autoDownloadDictionaries.value();
	}
	rpl::producer<bool> autoDownloadDictionariesChanges() const {
		return _autoDownloadDictionaries.changes();
	}

	[[nodiscard]] float64 videoPlaybackSpeed() const {
		return _videoPlaybackSpeed.current();
	}
	void setVideoPlaybackSpeed(float64 speed) {
		_videoPlaybackSpeed = speed;
	}
	[[nodiscard]] QByteArray videoPipGeometry() const {
		return _videoPipGeometry;
	}
	void setVideoPipGeometry(QByteArray geometry) {
		_videoPipGeometry = geometry;
	}

	[[nodiscard]] float64 rememberedSongVolume() const {
		return _rememberedSongVolume;
	}
	void setRememberedSongVolume(float64 value) {
		_rememberedSongVolume = value;
	}
	[[nodiscard]] bool rememberedSoundNotifyFromTray() const {
		return _rememberedSoundNotifyFromTray;
	}
	void setRememberedSoundNotifyFromTray(bool value) {
		_rememberedSoundNotifyFromTray = value;
	}
	[[nodiscard]] bool rememberedFlashBounceNotifyFromTray() const {
		return _rememberedFlashBounceNotifyFromTray;
	}
	void setRememberedFlashBounceNotifyFromTray(bool value) {
		_rememberedFlashBounceNotifyFromTray = value;
	}
	[[nodiscard]] bool mainMenuAccountsShown() const {
		return _mainMenuAccountsShown.current();
	}
	[[nodiscard]] rpl::producer<bool> mainMenuAccountsShownValue() const {
		return _mainMenuAccountsShown.value();
	}
	void setMainMenuAccountsShown(bool value) {
		_mainMenuAccountsShown = value;
	}
	[[nodiscard]] bool tabbedSelectorSectionEnabled() const {
		return _tabbedSelectorSectionEnabled;
	}
	void setTabbedSelectorSectionEnabled(bool enabled);
	[[nodiscard]] bool thirdSectionInfoEnabled() const {
		return _thirdSectionInfoEnabled;
	}
	void setThirdSectionInfoEnabled(bool enabled);
	[[nodiscard]] rpl::producer<bool> thirdSectionInfoEnabledValue() const;
	[[nodiscard]] int thirdSectionExtendedBy() const {
		return _thirdSectionExtendedBy;
	}
	void setThirdSectionExtendedBy(int savedValue) {
		_thirdSectionExtendedBy = savedValue;
	}
	[[nodiscard]] bool tabbedReplacedWithInfo() const {
		return _tabbedReplacedWithInfo;
	}
	void setTabbedReplacedWithInfo(bool enabled);
	[[nodiscard]] rpl::producer<bool> tabbedReplacedWithInfoValue() const;
	void setFloatPlayerColumn(Window::Column column) {
		_floatPlayerColumn = column;
	}
	[[nodiscard]] Window::Column floatPlayerColumn() const {
		return _floatPlayerColumn;
	}
	void setFloatPlayerCorner(RectPart corner) {
		_floatPlayerCorner = corner;
	}
	[[nodiscard]] RectPart floatPlayerCorner() const {
		return _floatPlayerCorner;
	}

	// Fork Settings.
	[[nodiscard]] bool squareUserpics() const {
		return _squareUserpics;
	}
	void setSquareUserpics(bool newValue) {
		_squareUserpics = newValue;
	}
	[[nodiscard]] bool audioFade() const {
		return _audioFade;
	}
	void setAudioFade(bool newValue) {
		_audioFade = newValue;
	}
	[[nodiscard]] bool askUriScheme() const {
		return _askUriScheme;
	}
	void setAskUriScheme(bool newValue) {
		_askUriScheme = newValue;
	}
	[[nodiscard]] QString uriScheme() const {
		return _uriScheme;
	}
	void setUriScheme(QString newValue) {
		_uriScheme = newValue;
	}
	[[nodiscard]] bool lastSeenInDialogs() const {
		return _lastSeenInDialogs;
	}
	void setLastSeenInDialogs(bool newValue) {
		_lastSeenInDialogs = newValue;
	}
	[[nodiscard]] QString searchEngineUrl() const {
		return _searchEngineUrl;
	}
	void setSearchEngineUrl(QString newValue) {
		_searchEngineUrl = newValue;
	}
	[[nodiscard]] bool searchEngine() const {
		return _searchEngine;
	}
	void setSearchEngine(bool newValue) {
		_searchEngine = newValue;
	}
	[[nodiscard]] bool allRecentStickers() const {
		return _allRecentStickers;
	}
	void setAllRecentStickers(bool newValue) {
		_allRecentStickers = newValue;
	}
	[[nodiscard]] int customStickerSize() const {
		return _customStickerSize;
	}
	void setCustomStickerSize(int newValue) {
		_customStickerSize = newValue;
	}
	[[nodiscard]] bool useBlackTrayIcon() const {
		return _useBlackTrayIcon;
	}
	void setUseBlackTrayIcon(bool newValue) {
		_useBlackTrayIcon = newValue;
	}
	[[nodiscard]] bool useOriginalTrayIcon() const {
		return _useOriginalTrayIcon;
	}
	void setUseOriginalTrayIcon(bool newValue) {
		_useOriginalTrayIcon = newValue;
	}
	//

	void setDialogsWidthRatio(float64 ratio);
	[[nodiscard]] float64 dialogsWidthRatio() const;
	[[nodiscard]] rpl::producer<float64> dialogsWidthRatioChanges() const;
	void setThirdColumnWidth(int width);
	[[nodiscard]] int thirdColumnWidth() const;
	[[nodiscard]] rpl::producer<int> thirdColumnWidthChanges() const;
	void setNotifyFromAll(bool value) {
		_notifyFromAll = value;
	}
	[[nodiscard]] bool notifyFromAll() const {
		return _notifyFromAll;
	}

	[[nodiscard]] static bool ThirdColumnByDefault();
	[[nodiscard]] float64 DefaultDialogsWidthRatio();
	[[nodiscard]] static qint32 SerializePlaybackSpeed(float64 speed) {
		return int(std::round(std::clamp(speed * 4., 2., 8.))) - 2;
	}
	[[nodiscard]] static float64 DeserializePlaybackSpeed(qint32 speed) {
		return (std::clamp(speed, 0, 6) + 2) / 4.;
	}

	void resetOnLastLogout();

private:
	static constexpr auto kDefaultThirdColumnWidth = 0;
	static constexpr auto kDefaultDialogsWidthRatio = 5. / 14;
	static constexpr auto kDefaultBigDialogsWidthRatio = 0.275;

	bool _squareUserpics = false;
	bool _audioFade = true;
	bool _askUriScheme = false;
	bool _lastSeenInDialogs = false;
	QString _uriScheme = qsl("");
	QString _searchEngineUrl = qsl("https://dgg.gg/%q");
	bool _searchEngine = false;
	bool _allRecentStickers = true;
	int _customStickerSize = 256;
	bool _useBlackTrayIcon = false;
	bool _useOriginalTrayIcon = false;

	bool _adaptiveForWide = true;
	bool _moderateModeEnabled = false;
	rpl::variable<float64> _songVolume = kDefaultVolume;
	rpl::variable<float64> _videoVolume = kDefaultVolume;
	bool _askDownloadPath = false;
	rpl::variable<QString> _downloadPath;
	QByteArray _downloadPathBookmark;
	bool _voiceMsgPlaybackDoubled = false;
	bool _soundNotify = true;
	bool _desktopNotify = true;
	bool _flashBounceNotify = true;
	DBINotifyView _notifyView = dbinvShowPreview;
	bool _nativeNotifications = false;
	int _notificationsCount = 3;
	ScreenCorner _notificationsCorner = ScreenCorner::BottomRight;
	bool _includeMutedCounter = true;
	bool _countUnreadMessages = true;
	rpl::variable<bool> _notifyAboutPinned = true;
	int _autoLock = 3600;
	QString _callOutputDeviceID = u"default"_q;
	QString _callInputDeviceID = u"default"_q;
	int _callOutputVolume = 100;
	int _callInputVolume = 100;
	bool _callAudioDuckingEnabled = true;
	Window::Theme::AccentColors _themesAccentColors;
	bool _lastSeenWarningSeen = false;
	SendFilesWay _sendFilesWay;
	Ui::InputSubmitSettings _sendSubmitWay;
	base::flat_map<QString, QString> _soundOverrides;
	bool _exeLaunchWarning = true;
	bool _loopAnimatedStickers = true;
	rpl::variable<bool> _largeEmoji = true;
	rpl::variable<bool> _replaceEmoji = true;
	bool _suggestEmoji = true;
	bool _suggestStickersByEmoji = true;
	rpl::variable<bool> _spellcheckerEnabled = true;
	rpl::variable<float64> _videoPlaybackSpeed = 1.;
	QByteArray _videoPipGeometry;
	rpl::variable<std::vector<int>> _dictionariesEnabled;
	rpl::variable<bool> _autoDownloadDictionaries = true;
	rpl::variable<bool> _mainMenuAccountsShown = false;
	bool _tabbedSelectorSectionEnabled = false; // per-window
	Window::Column _floatPlayerColumn; // per-window
	RectPart _floatPlayerCorner; // per-window
	bool _thirdSectionInfoEnabled = true; // per-window
	rpl::event_stream<bool> _thirdSectionInfoEnabledValue; // per-window
	int _thirdSectionExtendedBy = -1; // per-window
	rpl::variable<float64> _dialogsWidthRatio; // per-window
	rpl::variable<int> _thirdColumnWidth = kDefaultThirdColumnWidth; // p-w
	bool _notifyFromAll = true;

	bool _tabbedReplacedWithInfo = false; // per-window
	rpl::event_stream<bool> _tabbedReplacedWithInfoValue; // per-window

	float64 _rememberedSongVolume = kDefaultVolume;
	bool _rememberedSoundNotifyFromTray = false;
	bool _rememberedFlashBounceNotifyFromTray = false;

};

} // namespace Core
