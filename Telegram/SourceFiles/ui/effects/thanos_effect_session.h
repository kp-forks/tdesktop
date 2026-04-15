/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

namespace Main {
class Session;
} // namespace Main

namespace Ui {

// Schedules `ThanosEffect::WarmUp()` to run once the main dialog list
// has finished its initial load (or immediately if already loaded).
// Kept in a separate file so the Ui-only `ThanosEffect` does not need
// to depend on `Main::Session` / `Data::Session`.
void ScheduleThanosEffectWarmUp(
	not_null<Main::Session*> session,
	rpl::lifetime &lifetime);

} // namespace Ui
