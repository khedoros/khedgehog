#include "memmap_m68k.h"
/* Mapping
 * Component     |Start Addr| End Addr |
 * Cartridge       0x000000   0x3FFFFF
 * Sega CD         0x400000   0x7FFFFF
 * Sega 32x        0x800000   0x9FFFFF
 * Z80 addr space  0xA00000   0xA0FFFF
 * I/O ports       0xA10000   0xA10FFF
 * Z80 control     0xA11000   0xAFFFFF
 * Unallocated     0xB00000   0xBFFFFF
 * VDP             0xC00000   0xDFFFFF
 * 68K RAM         0xE00000   0xFFFFFF (mirrored every 64KiB)
 */

 memmap_m68k::memmap_m68k(std::shared_ptr<config> cfg) {}
