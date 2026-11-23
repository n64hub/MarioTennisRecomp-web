// Bonus characters: Shy Guy and Donkey Kong Jr. are normally unlocked by winning Star Cup tournaments
// (singles and doubles). The game keeps their unlock bits in one byte of its save data in RAM,
// 0x800653C8 (bit 0 = DK Jr., bit 1 = Shy Guy; the GameShark code "Have D.K. Jr. & Shy Guy" is
// 800653C8 0003). Setting both bits every frame makes the two characters always selectable; the game
// then also writes them to the save like any other unlock.
//
// Transfer Pak characters (issue #10): Alex, Harry, Nina and Kate normally come from a Mario Tennis
// Game Boy Color cartridge in a Transfer Pak. The N64 game keeps them in its own save data, in two
// slots (one per Game Boy save, two characters each):
//   0x80065468       byte: Game Boy characters present
//   0x800653D8 + s*0x48  word: slot ID (any non-zero value), +4 byte: slot in use
//   0x800653E0 + s*0x48 + c*0x20  character record (0x20 bytes):
//       0x00 name (ASCII, up to 11 bytes), 0x0B type (which of the four characters),
//       0x0C 0, 0x0D level, 0x0E..0x18 stats
// The game has a developer routine (func_8003CB74) that fills these with test characters; this does
// the same with the real names when no Game Boy characters are present yet. The menus then build the
// characters' match data from the records themselves (func_8003C930 when the menu overlay starts).
#include <cstdint>
#include <cstring>

// Cheats (Settings > General, all off by default, src/game/config.cpp), from the well-known GameShark codes for Mario
// Tennis (USA), applied every graphics task like the unlock above:
//   All courts            810653CC FFFF        unlock bits for the courts (0x800653CC)
//   Star characters       810653CA FFFF        "Star" versions of every character (0x800653CA)
//   All tournaments       50000802 0000 + 81065278 EEEE   eight cup records from 0x80065278 marked won
//   Infinite bounces      8015B16B 0000        the bounce counter in the match state, kept at 0
// The first three are save data: the game writes them to the save like normal unlocks, so they stay
// after the cheat is switched off. Infinite bounces only lasts while it is on.
#include "mt_game.h"

namespace {
    void apply_cheats(uint8_t* rdram) {
        auto b = [&](uint32_t addr, uint8_t v) { rdram[addr ^ 3] = v; };
        using mt::Cheat; using mt::cheat_on;
        if (cheat_on(Cheat::AllCourts)) { b(0x653CC, 0xFF); b(0x653CD, 0xFF); }
        if (cheat_on(Cheat::StarCharacters)) { b(0x653CA, 0xFF); b(0x653CB, 0xFF); }
        if (cheat_on(Cheat::AllTournaments)) {
            for (uint32_t i = 0; i < 8; i++) { b(0x65278 + 2 * i, 0xEE); b(0x65279 + 2 * i, 0xEE); }
        }
        if (cheat_on(Cheat::InfiniteBounces)) b(0x15B16B, 0x00);
    }
}

namespace {
    constexpr uint32_t gb_present = 0x65468;
    constexpr uint32_t gb_slot = 0x653D8;
    constexpr uint32_t gb_slot_size = 0x48;

    void wr8(uint8_t* rdram, uint32_t addr, uint8_t v) { rdram[addr ^ 3] = v; }
    uint8_t rd8(uint8_t* rdram, uint32_t addr) { return rdram[addr ^ 3]; }
    void wr32(uint8_t* rdram, uint32_t addr, uint32_t v) { std::memcpy(rdram + addr, &v, 4); }

    uint32_t rd32(uint8_t* rdram, uint32_t addr) { uint32_t v; std::memcpy(&v, rdram + addr, 4); return v; }

    struct GbCharacter { const char* name; uint8_t type; };
    // Types (checked on the character select screen): 0 Alex, 1 Nina, 2 Harry, 3 Kate.
    // Each slot is one Game Boy duo: slot 0 Alex & Harry, slot 1 Nina & Kate.
    constexpr GbCharacter gb_characters[4] = {
        { "ALEX", 0 }, { "HARRY", 2 }, { "NINA", 1 }, { "KATE", 3 },
    };
    constexpr uint32_t our_slot_id = 0x4D540000u;   // "MT" + slot number: slots written by this code

    void add_transfer_pak_characters(uint8_t* rdram) {
        // Leave real Game Boy characters alone; (re)write when there are none, or when they are ours.
        const bool present = rd8(rdram, gb_present) != 0;
        const bool ours = (rd32(rdram, gb_slot) == our_slot_id) && (rd32(rdram, gb_slot + gb_slot_size) == (our_slot_id | 1));
        if (present && !ours) return;
        for (int i = 0; i < 4; i++) {
            const uint32_t slot = gb_slot + (i / 2) * gb_slot_size;
            wr32(rdram, slot, our_slot_id | (uint32_t)(i / 2));
            wr8(rdram, slot + 4, 1);
            const uint32_t rec = slot + 8 + (i % 2) * 0x20;
            const char* name = gb_characters[i].name;
            for (uint32_t k = 0; k < 11; k++) wr8(rdram, rec + k, (k < std::strlen(name)) ? (uint8_t)name[k] : 0);
            wr8(rdram, rec + 0x0B, gb_characters[i].type);
            if (!present) {                                       // new: starting level and stats
                wr8(rdram, rec + 0x0C, 0);
                wr8(rdram, rec + 0x0D, 0x19);                     // level 25, as the developer routine
                for (uint32_t k = 0x0E; k <= 0x18; k++) wr8(rdram, rec + k, 5);
            }
        }
        wr8(rdram, gb_present, 1);
    }
}

extern "C" void mt_bonus_characters(uint8_t* rdram) {
    // RDRAM is stored as native-endian 32-bit words, so byte addresses are XORed with 3.
    uint8_t& unlocks = rdram[0x653C8 ^ 3];
    unlocks |= 0x03;
    add_transfer_pak_characters(rdram);
    apply_cheats(rdram);
}
