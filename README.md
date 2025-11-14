# Baldur's Gate 3 (Linux/Steamdeck) Achievement Enabler (bg3-linux-ae)

Thanks to the work done before at https://github.com/Norbyte/bg3se/ I
was able to use the Windows byte offsets to scrutinize/backtrack some
code in Ghidra until I found similar areas (although quite different,
see discussion here: https://github.com/Norbyte/bg3se/issues/539) that
were patchable.

# Installation

Clone repo, run "make", add the generated ".so" file into your
LD_PRELOAD in your Steam game launch options:

```
LD_PRELOAD="~/bg3_linux_ae.so${LD_PRELOAD:+:$LD_PRELOAD}" %command% >/tmp/bg3.log 2>&1
```

if you chose to hold the produced .so file in your home dir.

You can confirm it worked by checking output in /tmp/bg3.log, you
should see some output like this:

```
❯ cat /tmp/bg3.log

Attempting Patch 1: ls::ModuleSettings::IsModded (Achievements)...
base_address was: 0x55ce027a7000
Patch 1 match
Successfully patched memory at: 0x55ce041abb9e

Attempting Patch 2: esv::SavegameManager::ThrowError (Savegame Warnings)...
Patch 2 match
Successfully patched memory at: 0x55ce037440d0
```

(among a bunch of Steam output/noise)

# WARNING

This is lightly tested - I was able to get an achievement (hire a
merc) while my mods were still active, however per standard GPLv3 and
most FOSS licenses, I assume no liability and guarantee nothing.

I can't imagine there is a chance of anything actually being screwed
up, especially as it doesn't permanently patch the binary (so if
things behave oddly, just remove the LD_PRELOAD from your game launch
command).

# Copyright

Matthew Carter <m@ahungry.com>

# License

GPLv3 or later

Distributed under the GNU General Public License either version 3.0 or (at
your option) any later version.
