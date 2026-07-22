#!/usr/bin/env python3
"""
Publish one release asset to the public announcement channel.
"""
import os
import sys
import asyncio
from datetime import datetime, timedelta, timezone

from telethon import TelegramClient
from telethon.extensions import markdown
from telethon.sessions import StringSession

CHANNEL = os.environ["TG_PUBLIC_CHANNEL"]
ASSET_PATH = os.environ["ASSET_PATH"]
VERSION = os.environ["ASSET_VERSION"]
DRY_RUN = os.environ.get("TG_DRY_RUN", "") == "1"
SCHEDULE_DAYS = int(os.environ.get("TG_SCHEDULE_DAYS", "350") or "350")

CAPTION = ("— Updated version to "
           "[{version}](https://github.com/telegramdesktop/tdesktop/releases/tag/v{version}).")


async def main():
    if not os.path.isfile(ASSET_PATH):
        sys.exit(f"No asset at {ASSET_PATH!r}.")

    caption = CAPTION.format(version=VERSION)
    size = os.path.getsize(ASSET_PATH) / 1048576
    print(f"File: {ASSET_PATH} ({size:.0f} MiB)")
    print(f"Caption: {caption}")

    if DRY_RUN:
        shown, entities = markdown.parse(caption)
        print(f"Renders as: {shown}")
        for e in entities:
            print(f"  {type(e).__name__}: {shown[e.offset:e.offset + e.length]!r} "
                  f"-> {getattr(e, 'url', '')}")
        print("\n[dry-run] not uploading.")
        return

    if not 0 < SCHEDULE_DAYS < 365:
        sys.exit(f"TG_SCHEDULE_DAYS must be within 1..364, got {SCHEDULE_DAYS}.")
    when = datetime.now(timezone.utc) + timedelta(days=SCHEDULE_DAYS)

    api_id = int(os.environ["TG_API_ID"])
    api_hash = os.environ["TG_API_HASH"]
    session = os.environ["TG_SESSION"]

    async with TelegramClient(StringSession(session), api_id, api_hash) as client:
        channel = await client.get_entity(CHANNEL)
        msg = await client.send_file(
            channel, ASSET_PATH,
            force_document=True,
            caption=caption,
            parse_mode="md",
            schedule=when)
        print(f"\nscheduled message #{msg.id} in {CHANNEL} for {when:%Y-%m-%d} "
              f"({SCHEDULE_DAYS} days out); send it from the Scheduled queue.")


if __name__ == "__main__":
    asyncio.run(main())
