// Minimal stub for the gt2reloc CLI build — supplies the empty datafile[]
// symbol and the stereo / dual-SID runtime flags introduced for the Qt
// build. Defaults match the historical mono behaviour, so gt2reloc keeps
// producing single-SID PRG / SID exports.
unsigned char datafile[1] = {0};
int stereo_mode = 0;
unsigned sid2model = 0;
