# Embedding GoatTracker music + SFX in a C64 game

This is the answer to most of the questions in the Lemon64 GoatTracker2
thread (https://www.lemon64.com/forum/viewtopic.php?t=84344) and the
recurring confusion that hits anyone who packs a `.sng` with `gt2reloc`
and then tries to call the resulting playroutine from inside their own
6502 main loop.

## 1. Choose an IRQ source

You have two practical choices:

- **CIA-1 timer interrupt (≈ 50 Hz)**. Simple, runs everywhere, fires
  once per frame on a PAL machine without any raster math. Default
  choice for most games and demos.
- **Raster interrupt** triggered from `$D012`. More precise, lets you
  align other VIC effects (screen splits, sprite multiplexing) to a
  specific raster line. Slightly more code.

Pitfalls when you wire it up:

- `$D012` is only the low 8 bits of the raster compare. **Bit 8 lives
  in `$D011` bit 7.** A common bug is writing `#$ff` to `$D012` and
  not clearing `$D011.7`, which makes the IRQ fire twice per frame
  (once on raster 255, once on raster 511 which wraps to a second
  hit). Stick to lines below 256 unless you're doing tricks.
- Do **not** end the IRQ with `JMP $EA31`. That jump runs the KERNAL
  IRQ handler, which scans the keyboard, blinks the cursor, and bumps
  the jiffy clock — all things you've probably already taken over. Use
  `JMP $EA81` if you have the KERNAL ROM mapped in, or restore the
  registers + `RTI` yourself.

## 2. Verify single-call-per-frame timing

The fastest sanity check is a one-byte flicker on the border colour:

```asm
play_irq:
    inc $d020          ; flicker border so you see one event per IRQ
    jsr music+3        ; play one frame of music
    dec $d020
    ; ... acknowledge IRQ + restore + RTI
```

When this is wired correctly, the border has exactly one visible
flicker per PAL frame. If you see two thin stripes per frame, your IRQ
is firing twice (see `$D012` warning above). If music plays at double
speed or runs slow, the player is being called from the wrong rate.

## 3. ZP collision map

`gt2reloc` will tell you which zero-page addresses the player uses
(answer: the two consecutive bytes you supplied with the
`-z<page>` option, default `$FC/$FD`). Pick a ZP pair your game does
not touch.

If you're using the **Zeropage Ghost Registers** playroutine option,
the player also reserves 25 bytes at the zeropage location given by
the `-Z` option to mirror the SID registers. After every `JSR
music+3`, copy those 25 bytes to `$D400`:

```asm
    ldx #$18
copy_loop:
    lda ghostregs,x
    sta $d400,x
    dex
    bpl copy_loop
```

The default ZP map for a tracker tune with all options on:

| Range          | Purpose                                                     |
| -------------- | ----------------------------------------------------------- |
| `$FC/$FD`      | Player's two-byte work area (configurable with `-z`)        |
| `$F0..$F8`     | Ghost-reg copy buffer when `-Z` is on                       |
| `$01`          | You will probably want to set this to `$35` (see §4)        |

Most homebrew uses `$FC/$FD` for the player and the bottom half of
`$F0..$FB` for game state. If you need every byte of ZP, route the
player work area to `$02/$03` (`-z$02`) — those two bytes are normally
KERNAL scratch.

## 4. Setting `$01 = $35`

The C64's processor port (`$01`) selects which ROMs are visible at
`$A000-$BFFF` (BASIC) and `$E000-$FFFF` (KERNAL). The default after a
hard reset is `$37` — everything visible.

If you want a **stable** raster IRQ that never gets nudged by KERNAL
activity (cursor blink, keyboard scan, jiffy update) and you want
every byte of RAM accessible for music data:

```asm
    sei
    lda #$35           ; BASIC out, KERNAL out, I/O in
    sta $01
    ; install your own IRQ vectors at $FFFE/$FFFF and you're done
```

`$36` is the "I/O out, BASIC out, KERNAL in" variant — pick that if
you still want the KERNAL ROM accessible for printing chars. Most
production tunes ship with `$35`.

## 5. Calling the player safely

The minimum boilerplate around each `JSR music+3` is to preserve A/X/Y
and to acknowledge the IRQ source:

```asm
play_irq:
    pha
    txa : pha
    tya : pha

    jsr music+3        ; play one frame

    lda $dc0d          ; acknowledge CIA-1 IRQ
    pla : tay
    pla : tax
    pla
    rti
```

For a raster IRQ, replace `lda $dc0d` with `asl $d019` (ack VIC IRQ).

If you chain to a second IRQ handler (e.g. game logic that runs at
half rate), preserve all three registers before the `JSR` and make
sure your handler doesn't clobber the player's ZP pair.

## 6. SFX engine quirks

GoatTracker2's SFX format is **deliberately minimal**. The `.asm`
file `ins2snd2` produces from an instrument is often only 5–20 bytes;
a "3-byte" output is not a bug, it's an instrument with no audible
content.

Real constraints (`readme.txt` §6.3):

- **Absolute notes only.** Relative notes (`$00..$5F` in the
  wavetable right column) cannot be expressed. If your instrument's
  wavetable uses relative notes, `ins2snd2` will fail conversion.
- **No pulse / filter modulation.** The SFX engine is wavetable +
  ADSR + a single pulse-width write. Anything that needs the
  pulsetable or filtertable is dropped.
- **Release must finish before the next note.** SFX play with a
  priority based on memory address — a higher-memory SFX won't be
  interrupted by a lower-memory one, even on the same channel. If
  your SFX is long and you trigger it again too soon, the new call
  is silently dropped.
- **Engine size cap: 128 bytes per sound.** `ins2snd2` enforces this.

To call an SFX from your game:

```asm
    lda #<sfx_jump
    ldy #>sfx_jump
    ldx #0             ; channel 0, 7 for ch2, 14 for ch3
    jsr music+6        ; +9 if you also enabled volume-change support
```

## 7. Recommended `gt2reloc` flags for game embedding

| Flag                  | When you want it                                          |
| --------------------- | --------------------------------------------------------- |
| `-b<addr>`            | Set music base address (default `$1000`)                  |
| `-z<page>`            | Player ZP work area (default `$FC`)                       |
| `-Z<page>`            | Enable Zeropage Ghost Registers + base addr               |
| `-s`                  | Enable SFX support (implies buffered writes)              |
| `-v`                  | Enable volume-change call (master-vol jumptable slot)     |
| `-a`                  | Store author string in `+$20..+$3F`                       |
| `-o`                  | Disable relocator optimizations (debug only)              |
| `-d`                  | Disable buffered writes                                   |

Run `gt2reloc` with no arguments for the full table.

## 8. Common follow-up questions

- **"Music plays faster in-game than in the editor."** Your IRQ is
  firing more than once per frame. Re-check §1 (`$D012` bit 8) and
  §2 (one flicker per frame).
- **"Player call clobbers my game variables."** Check §3, then
  switch the ZP pair via `-z`.
- **"SFX silences the music."** Buffered writes (`-s` flag) or
  Ghost Regs (`-Z`) is required for SFX to coexist with music
  without skipping SID frames. The default unbuffered build won't.
- **"How do I export to .sid?"** Run `gt2reloc song.sng song.sid`.
  The first two bytes of the chosen base address become the load
  address; the playroutine sits there too. Drop the `.sid` into
  HVSC-compatible players for verification.

## Quick reference

| Thing you want                    | How                                  |
| --------------------------------- | ------------------------------------ |
| One play call per PAL frame       | CIA-1 timer IRQ at `$DC04/$DC05 = ~17095` |
| Player base address               | `gt2reloc -b<addr>`                  |
| SID register dump under IRQ       | `-Z<zp>` then 25-byte copy loop      |
| Stable raster IRQ                 | `$01 = $35`, install own vectors     |
| Trigger an SFX                    | A/Y = pointer, X = channel*7, `jsr music+6` |
