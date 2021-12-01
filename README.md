# S8DS V1.1.7

This is a SEGA 8Bit emulator for the NDS, it support the following systems:

	SEGA SG-1000 / SG-1000 II
	SEGA SC-3000
	SEGA Mark III
	SEGA Master System J, 1 & 2
	SEGA Game Gear
	SEGA System-E
	Coleco
	MSX 1
	Sord M5

Some systems require you to add their Bios'es to function correctly.

Features:

	Most things you'd expect from an SMS emulator.
	Except these...

Missing:

	Correct sprite collision.
	Speech samples.
	Good YM2413 emulation.
	EEPROM save for the few GG games that use it.

Check your roms!
https://www.smspower.org/maxim/Software/SMSChecker

## How to use:

Depending on your flashcart you might have to DLDI patch the emulator.
You should also create a "S8DS" directory either in the root of your card or in
the data directory (eg h:\data\S8DS).

Put your games on your flash card, max 768 games per folder (though shouldn't be
a problem if you separate SMS, GG, SG & Coleco games into different folders).
Filenames must not be longer than 127 chars.
You can use zipped files (as long as they use the deflate compression).
The emulator should also recognise SMS games for SEGAs MegaTech arcade machine,
3 of the SG-1000 arcade games and System-E (using MAME zip files).

The GUI is accessed by pressing L+R (shoulder buttons) or by touching the
screen, tabs are changed by pressing the L or R button, going to the left most
tab exits the GUI. Closing your DS puts the emulator in sleep mode, just open
it to resume.

When you first run the emulator I suggest you take a peak through the options
and change the settings to your liking and then save them.
Now load up a game and you should be good to go.

When playing SMS 3D games you might want to turn down the color a bit for
better 3D effect, I have only tested with red/cyan glasses.

## Menu:
### File:
	Load Game:
	Load State: Select which state you want to load.
	Save State: Select if you want to overwrite an old state or create a new.
	Save SRAM:
	Save Settings:
	Eject Game:
	Power On/Off:
	Reset Game:
	Exit: (If your card supports it.)

### Options:
	Controller:
		Autofire: Select if you want autofire.
		Controller: 2P control player 2.
		Swap A/B: Swap which NDS button is mapped to which SMS/GG button.
		Joypad type: You can select 3 button Megadrive/Genesis pad.
		Use Select as Reset: Map the NDS SELECT button to the SMS Reset button.
		Use R as FastForward: Select turbo speed as long as R button is held.

	Display:
		Display: Here you can select if you want scaled or unscaled screenmode.
		Scaling: Here you can select if you want flicker or barebones lineskip.
		Gamma: Lets you change the gamma ("brightness").
		Color: Lets you change the color.
		GG Border: Lets you change between black, bordercolor and none.
		Perfect Sprites: Uses a bit more cpu but is worth it.
		3D Display: Terminator vs Robocop needs this off.
		Disable Background: Turn on/off background rendering.
		Disable Sprites: Turn on/off sprite rendering.

	Machine:
		Region: Change the region of the SMS and video standard.
		Machine: Here you can select the hardware, Auto should work for most games.
		Bios Settings:
			Use Bios: Here you can select if you want to use the selected BIOSes.
			Select Export Bios: Browse for export bios.
			Select Japanese Bios: Browse for japanese bios.
			Select GameGear Bios: Browse for GameGear bios.
			Select Coleco Bios: Browse for Coleco bios.
			Select MSX Bios: Browse for MSX bios.
		YM2413: Enable YM2413 emulation for SMS1 & Mark 3.

	Settings:
		Speed: Switch between speed modes, can also be toggled with L+START.
			Normal: Standard, 100% speed.
			Fast: Double, 200% speed.
			Max: Fastest, 400% speed.
			Slowmo: Slow, 50% speed.
		Autoload State: Toggle Savestate autoloading.
			Automagicaly load the savestate associated with the selected game.
		Autosave Settings: This will save settings when
			leaving menu if any changes are made.
		Autopause Game: Toggle if the game should pause when opening the menu.
		Powersave 2nd Screen: If graphics/light should be turned off for the
			GUI screen when menu is not active.
		Emulator on Bottom: Select if top or bottom screen should be used for
			emulator, when menu is active emulator screen is allways on top.
		Debug Output: Toggle fps meter & more.
		Autosleep: Change the autosleep time, also see Sleep. !!!DoesntWork!!!

	Dipswitches: For arcade games

### About:
	Some dumb info...

## Arcade roms
	After Burner MT - mt_aftrb.zip
	Alien Syndrom MT - mt_asyn.zip
	Astro Warrior MT - mt_astro.zip
	Fantasy Zone MT - mt_fz.zip
	Great Football MT - mt_gfoot.zip
	Great Golf MT - mt_ggolf.zip
	Great Soccer MT - mt_gsocr.zip
	Out Run MT - mt_orun.zip
	Parlour Games MT - mt_parlg.zip
	Shinobi MT - mt_shnbi.zip

	Champion Boxing SG-AC - chboxing.zip
	Champion Wrestling SG-AC - chwrestl.zip (encrypted)
	Doki Doki Penguin SG-AC - dokidoki.zip

	Astro Flash System-E - astrofl.zip (encrypted)
	Fantasy Zone 2 System-E - fantzn2.zip (encrypted)
	Hang On Jr System-E - hangonjr.zip
	Opa Opa System-E - opaopa.zip (encrypted)
	Riddle Of Pythagoras System-E - ridleofp.zip
	Tetris System-E - tetrisse.zip
	Transformer System-E - transfrm.zip


## Credits:

Thanks to:
Reesy for help with the Z80 emu core.
Some MAME people + Maxim for the SN76496 info.
Charles MacDonald (http://cgfm2.emuviews.com/) for VDP info.
Omar Cornut (http://www.smspower.org/) for help with various SMS stuff.
The crew at PocketHeaven for their support.

Fredrik Olsson

Twitter @TheRealFluBBa

http://www.github.com/FluBBaOfWard
