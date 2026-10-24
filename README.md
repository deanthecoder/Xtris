# Xtris

A snowy falling-block game I wrote in C for DOS in **1996**, now playable in a browser at **[deanthecoder.github.io/Xtris](https://deanthecoder.github.io/Xtris/)**.

[![Follow @deanthecoder](https://img.shields.io/twitter/url/https/twitter.com/deanthecoder.svg?style=social&label=Follow%20%40deanthecoder)](https://x.com/deanthecoder)
[![GitHub stars](https://img.shields.io/github/stars/deanthecoder/Xtris?style=social&label=Star)](https://github.com/deanthecoder/Xtris/stargazers)

## Play

Fit falling shapes together to complete horizontal rows. Completed rows disappear and score points; the pieces fall faster as the game goes on. The snow keeps falling too. When the stack reaches the top, the hall of fame appears. High scores are saved in your browser.

| Key | Action |
| --- | --- |
| Left / Right | Move |
| Up | Rotate |
| Down or Space | Drop |
| P | Pause / resume |
| Escape | Quit prompt |

Use **New game** to restart. **CRT aspect ratio** stretches the original 320 × 200 picture to the 4:3 proportions typical of a DOS monitor; turn it off for square pixels. The game also scales to fit the browser window.

## The restoration

The surviving `XTRIS.EXE` is dated April 1996, and the intro screen says “Copyright 1996.” The parent archive happens to be named *1999 DOS Code*; 1996 is the date supported by the game itself. This edition transcribes the 13 piece patterns, shading mask, scoring and keyboard rules from the original C. It decodes the original `.SPR`, PCX, font and sound files in the browser, draws to a 320 × 200 indexed-colour canvas, and stores high scores locally.

The [`original/`](original/) directory preserves the DOS source, executable, art, audio, and the shared C modules it included. Those files are kept byte-for-byte; the browser implementation is in `main.js`, `vga.js`, and `game-data.js`. The source archive on my computer is untouched.

Two personal-data files have been left out of the public copy: `exit.ils` contains an old postal address, and `FAME.SCR` / `FAME.SC_` contain saved player names. The browser edition recreates the default hall of fame from the source code.

The browser version aims to preserve the original look and rules, but timing, snow motion, audio playback and keyboard handling depend on the modern browser and can differ from DOS. The original DOS files are included for study and preservation; run the browser version to play on a current machine.

## Run locally

Run `./run.sh` and open the printed local URL. A static web server is needed because the browser fetches the archived graphics and audio files.

## License

MIT. See [LICENSE](LICENSE).
