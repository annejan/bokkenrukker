// Minimal stub for the gt2reloc CLI build — supplies the empty datafile[]
// symbol that bme/io references at startup. Everything else (editstring,
// resettime, incrementtime, etc.) is provided by the SDL UI files we link
// in (gconsole.c, gdisplay.c, gfile.c) since gt2reloc reuses them headlessly.
unsigned char datafile[1] = {0};
