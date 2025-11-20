# Baldur's Gate 3 (Linux/Steamdeck) Achievement Enabler (bg3-linux-ae)

Thanks to the work done before at https://github.com/Norbyte/bg3se/ I
was able to use the Windows byte offsets to scrutinize/backtrack some
code in Ghidra until I found similar areas (although quite different,
see discussion here: https://github.com/Norbyte/bg3se/issues/539) that
were patchable.

# Installation

Simply download the latest release from the Releases tab (built under
an Ubuntu 18.04 docker, for maximal linux libc support):

https://github.com/ahungry/bg3-linux-ae/releases

and untar it (`tar xzvf bg3-linux-ae.tar.gz`), this will produce a
directory `bg3-linux-ae` with 2 files in it - the launch script and
the .so file.  The launch script takes care of appending your
LD_PRELOAD as necessary without botching your existing LD_PRELOAD path
(which will have things like the steam overlay).

Update your Steam launch options command similar to either:


```
~/bg3-linux-ae/bg3-linux-ae.sh %command% >/tmp/bg3.log 2>&1
```

or (manual preload vs script):

```
LD_PRELOAD="~/bg3-linux-ae/bg3_linux_ae.so${LD_PRELOAD:+:$LD_PRELOAD}" %command% >/tmp/bg3.log 2>&1
```

You can confirm it worked by checking output in /tmp/bg3.log, you
should see some output like this:

```
❯ cat /tmp/bg3.log

Attempting Patch 1: ls::ModuleSettings::IsModded (Achievements)...
base_address was: 0x5635dd557000
Patch 1 match
Successfully patched memory at: 0x5635def67dee

Attempting Patch 2: esv::SavegameManager::ThrowError (Savegame Warnings)...
Patch 2 match
Successfully patched memory at: 0x5635de513970

Attempting Patch 3: new game
Patch 3 match
Successfully patched memory at: 0x5635dda79a88
```

(among a bunch of Steam output/noise)

In game, you will *NOT* see the blue gear icon to the left of your
Continue/New Game menu options (that usually show when using a
controller).


# Why?

I wanted to play under linux, it runs better than
steam/proton/dx11/vulkan, and
https://github.com/evg-zhabotinsky/libspeedhack actually works great
with it, so you can fast forward through boring enemy turns.

Using that with the windows/proton build stutters badly, and I don't
use windows, so windows options for the same were not an option.

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
