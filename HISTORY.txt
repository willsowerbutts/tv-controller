Version 1 hardware + software (2017 -- Jan 2025)
 - Monitor amp power status via optocoupler
 - Control amp power using 5V relay, coil driven via transistor
 - Receive IR codes using 3-pin IR receiver
 - Power control: Set amp power on/off in response to specific RC5 IR codes

The switch from the Plasma to the LG OLED necessitated a rebuild and rewrite...

Version 2 hardware + software (Jan 2025 onwards)
 - Monitor amp power status via optocoupler
 - Control amp power using 5V relay, coil driven via transistor
 - removed: 3-pin IR receiver hardware
 - new: Receive IR codes directly from TV IR blaster port via optocoupler
 - new: Monitor Topping E70 DAC 12V trigger output via optocoupler
 - Power control: Set amp power on/off when the DAC is powered on/off (the DAC does this automatically based on the SPDIF data feed from the TV)
 - Volume control: Receive RC5 IR codes from TV and relay as NEC IR codes to Topping E70 for volume up/down.

On LG C4 OLED TV go to setup, "External Devices", then set the Optical Output as if it were connected to a Marantz (or Philips) Soundbar. Test different models until you receive RC5 codes.
Connecting to the IR blaster socket on the TV stops the remote handset from producing IR signals itself.
