AVRMIDI-TRIGGER
==========

This is an implementation of a simple MIDI to Trigger converter using a Atmel Attiny2313 microprocessor. Each Note-On Event in a given MIDI Note Range is converted into a Trigger Signal of 5 V.

The current Note Range is 6 Notes from middle C - because my SDS5 clone utilizes 6 Voices and therefor needs 6 Trigger inputs.
I was able to reuse a lot of sourcecode from my AVR MIDI-CV converter here... which is nice.

It also supports multitrigger/flames via velocity: there are 16 pre-defined flame pattern and velocity of 127 gives you a simple trigger.

Teststatus
==========
The code has been tested with an oscilloscope and seems to work fine.

