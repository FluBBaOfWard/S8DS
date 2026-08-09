# S8DS V1.1.9

<img align="right" width="220" src="./logo.png" />

This is a SEGA 8Bit emulator for the Nintendo DS(i)/3DS, it support the following systems:

	SEGA SG-1000 / SG-1000 II
	SEGA SC-3000
	SEGA Mark III
	SEGA Master System J, 1 & 2
	SEGA Game Gear
	SEGA System-E
	Othello Multivision
	Coleco
	MSX 1
	Sord M5

Some systems require you to add their Bios'es to function correctly.

Features:

	Most things you'd expect from an SMS emulator.
	Except these...

Missing:

	Correct sprite collision.
	Good YM2413 emulation.
	EEPROM save for the few GG games that use it.

Check your roms!
<https://www.smspower.org/maxim/Software/SMSChecker>

## How to use

1. Create a "S8DS" directory either in the root of your card or in the data
 directory (eg h:\data\S8DS). This is where settings and save files end up.
2. Now put game/bios files into a folder where you have (SEGA 8-bit) roms, max
 768 games per folder, filenames must not be longer than 127 chars. You can use
 zipped files (as long as they use the deflate compression).
3. Depending on your flashcart you might have to DLDI patch the emulator.

The emulator should also recognise SMS games for SEGAs MegaTech arcade machine,
the SG-1000 arcade games and System-E (using MAME zip files).

The GUI is accessed by pressing L+R (shoulder buttons) or by touching the
screen, tabs are changed by pressing the L or R button, going to the left most
tab exits the GUI. Closing your DS puts the emulator in sleep mode, just open
it to resume.

When you first run the emulator I suggest you take a peak through the options
and change the settings to your liking and then save them.
Now load up a game and you should be good to go.

When playing SMS 3D games you might want to turn down the color a bit for
better 3D effect, I have only tested with red/cyan glasses.

## Menu

### File

* Load Game:
* Load State: Select which state you want to load.
* Save State: Select if you want to overwrite an old state or create a new.
* Save NVRAM:
* Load Patch: Load an IPS patch for the current game.
* Save Settings:
* Eject Game:
* Power On/Off:
* Reset Console:
* Quit Emulator: (If your card/loader supports it.)

### Options

* Controller:
  * Autofire: Select if you want autofire.
  * Controller: 2P control player 2.
  * Swap A-B: Swap which NDS button is mapped to which SMS/GG button.
  * Joypad Type: You can select 3 button Megadrive/Genesis pad.
  * Use Select as Reset: Map the NDS SELECT button to the SMS Reset button.
  * Use R as FastForward: Select turbo speed as long as R button is held.
* Display:
  * Display: Select unscaled, scaled-to-fit or aspect-corrected geometry.
  * GG Upscaler: Select how Game Gear's 160x144 LCD viewport is enlarged, regardless of the Display setting. When enabled, the viewport is expanded to an aspect-correct fullscreen render.
    * Off: Uses the selected Display mode for GG games too.
    * Fast: Scales to 256x192 using nearest-neighbour sampling at 60fps.
    * Smooth: Uses the same physical-aspect scaling with bilinear interpolation. High quality but CPU-dependent, 30+fps on DSi.
    * Smooth2: Upscales via 3D engine at 30fps. Not as high quality as Smooth but performant.
    * Flicker: Alternates nearest-neighbour phases at 60fps for smoother scaling, but may flicker or dim edges.
  * Scaling: Here you can select if you want flicker or barebones lineskip.
  * Gamma: Lets you change the gamma ("brightness").
  * Color: Lets you change the color.
  * GG Border: Lets you change between black, bordercolor and none.
  * Perfect Sprites: Uses a bit more cpu but it is worth it.
  * 3D Display: Terminator vs Robocop needs this off.
* Machine:
  * Region: Change the region of the SMS and video standard.
  * Machine: Here you can select the hardware, Auto should work for most games.
  * Bios Settings:
    * Use Bios: Here you can select if you want to use the selected BIOSes.
    * Select Export Bios: Browse for export bios.
    * Select Japanese Bios: Browse for japanese bios.
    * Select GameGear Bios: Browse for GameGear bios.
    * Select Coleco Bios: Browse for Coleco bios.
    * Select MSX Bios: Browse for MSX bios.
    * Select Sord M5 Bios: Browse for Sord M5 bios.
  * YM2413: Enable YM2413 emulation for SMS1 & Mark 3.
* Settings:
  * Speed: Switch between speed modes, can also be toggled with L+START.
    * Normal: Game runs at its normal speed.
    * 200%: Game can run up to double speed.
    * Max: Games can run up to 4 times normal speed.
    * 50%: Game runs at half speed.
  * Allow Refresh Change: Allow the emulator to change NDS refresh rate.
  * Autoload State: Toggle Savestate autoloading. Automatically load the
   savestate associated with the selected game.
  * Autoload NVRAM: Toggle EEPROM/SRAM autoloading. Automatically load the EEPROM/SRAM associated with the selected game.
  * Autosave NVRAM: Toggle EEPROM/SRAM autosaving. Automatically save NVRAM when entering UI.
  * Autosave Settings: This will save settings when leaving menu if any
   changes are made.
  * Autopause Game: Toggle if the game should pause when opening the menu.
  * Powersave 2nd Screen: If graphics/light should be turned off for the GUI
   screen when menu is not active.
  * Emulator on Bottom: Select if top or bottom screen should be used for
   emulator, when menu is active emulator screen is allways on top.
  * Show Clock: Show current time in the UI.
  * Console Touch: Turn on/off interaction with the console graphics.
* Debug:
  * Debug Output: Toggle fps meter & more.
  * Disable Background: Turn on/off background rendering.
  * Disable Sprites: Turn on/off sprite rendering.
  * Step Frame: Emulate one frame.
* Dipswitches: For arcade games

### About

Some dumb info...

## Controls

### Master System

```text
Dpad is mapped to Up, Down, Left & Right.
B is mapped to Button 1.
A is mapped to Button 2.
X is mapped to Pause on console.
Start is mapped to Pause on console.
Select can be mapped to Reset on console.
```

### Game Gear

```text
X is mapped to Start on console.
Start is mapped to Start on console.
```

### Arcade

```text
Select is mapped to Coin1.
```

## Arcade roms

### MegaTech

* After Burner MT - mt_aftrb.zip
* Alien Syndrom MT - mt_asyn.zip
* Astro Warrior MT - mt_astro.zip
* Fantasy Zone MT - mt_fz.zip
* Great Football MT - mt_gfoot.zip
* Great Golf MT - mt_ggolf.zip
* Great Soccer MT - mt_gsocr.zip (bad dump)
* Out Run MT - mt_orun.zip
* Parlour Games MT - mt_parlg.zip
* Shinobi MT - mt_shnbi.zip

### SG AC

* Champion Boxing SG-AC - chboxing.zip
* Champion Wrestling SG-AC - chwrestl.zip (encrypted)
* Doki Doki Penguin SG-AC - dokidoki.zip
* Super Derby (satellite board) SG-AC - sderbys (not working)
* Super Derby II (satellite board) SG-AC - sderby2s (not working)

### System-E

* Astro Flash System-E - astrofl.zip (encrypted)
* Fantasy Zone 2 System-E - fantzn2.zip (encrypted)
* Hang On Jr System-E - hangonjr.zip
* Megumi Rescue System-E - megrescu.zip
* Opa Opa System-E - opaopa.zip (encrypted)
* Opa Opa System-E - opaopan.zip
* Riddle Of Pythagoras System-E - ridleofp.zip
* Slap Shooter System-E - slapshtr.zip
* Tetris System-E - tetrisse.zip
* Transformer System-E - transfrm.zip

## Credits

```text
Thanks to:
Reesy for help with the Z80 emu core.
Some MAME people + Maxim for the SN76496 info.
Charles MacDonald (http://cgfm2.emuviews.com/) for VDP info.
Omar Cornut (http://www.smspower.org/) for help with various SMS stuff.
The crew at PocketHeaven for their support.
```

Fredrik Ahlström

<https://bsky.app/profile/therealflubba.bsky.social>

<https://www.github.com/FluBBaOfWard>

X/Twitter @TheRealFluBBa
