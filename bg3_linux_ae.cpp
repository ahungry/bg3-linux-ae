#include <cstring>
#include <fstream>
#include <iostream>
#include <link.h>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <vector>

const char *EXPECTED_EXE_NAME = "bg3";

// Function to find the base and size of the main executable's code section
// (r-xp)
std::pair<uintptr_t, size_t>
get_exe_code_range ()
{
  std::ifstream maps ("/proc/self/maps");
  std::string line;

  // We look for the first 'r-xp' entry that does NOT have a full path (i.e.,
  // the main executable). Or, more reliably, the first r-xp entry, which is
  // usually the main executable.
  while (std::getline (maps, line))
    {
      if (line.find (" r-xp ") != std::string::npos)
        {
          // Found a read-execute private section (code)

          uintptr_t start_addr, end_addr;
          char dash;

          // Extract the start and end addresses from the line format:
          // START-END
          std::stringstream ss (line);
          ss >> std::hex >> start_addr >> dash >> end_addr;

          if (dash == '-')
            {
              size_t size = end_addr - start_addr;
              return { start_addr, size };
            }
        }
    }
  // Return a failed range if not found
  return { 0, 0 };
}

// --- Utility: Pattern Scanning Function ---
// Scans a memory range for the given byte pattern.
void *
find_pattern (const unsigned char *base, size_t size,
              const unsigned char *pattern, const unsigned char *mask,
              size_t pattern_size)
{
  for (size_t i = 0; i <= size - pattern_size; ++i)
    {
      bool found = true;
      for (size_t j = 0; j < pattern_size; ++j)
        {
          // Check if the current byte matches, or if the mask says to skip it
          // (0x00)
          if (mask[j] != 0x00 && base[i + j] != pattern[j])
            {
              found = false;
              break;
            }
        }
      if (found)
        {
          return (void *)&base[i];
        }
    }
  return nullptr;
}

// --- Core Patching Logic ---
void
apply_patch (void *address, const unsigned char *patch_bytes,
             size_t patch_size)
{
  if (!address)
    {
      std::cerr << "Error: Target address not found." << std::endl;
      return;
    }

  // 1. Get the page size for mprotect alignment
  long pagesize = sysconf (_SC_PAGESIZE);
  if (pagesize == -1)
    {
      std::cerr << "Error getting page size." << std::endl;
      return;
    }

  // 2. Calculate the start of the memory page containing the address
  void *page_start = (void *)((uintptr_t)address & ~(pagesize - 1));

  // 3. Change memory protection to allow writing (PROT_READ | PROT_WRITE |
  // PROT_EXEC)
  if (mprotect (page_start, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC)
      == -1)
    {
      perror ("mprotect (writeable) failed");
      return;
    }

  // 4. Apply the patch (overwrite the original bytes)
  memcpy (address, patch_bytes, patch_size);

  // 5. Restore memory protection to read/execute only
  if (mprotect (page_start, pagesize, PROT_READ | PROT_EXEC) == -1)
    {
      perror ("mprotect (read/exec) failed");
    }

  std::cout << "Successfully patched memory at: " << address << std::endl;
}

// ----------------------------------------------------------------------
// --- PATCH 1: ls::ModuleSettings::IsModded (Achievement Enabling) ---
// ----------------------------------------------------------------------

//  find in ghidra: 0c 1b 04 0f 94 c0
// simd comparison of a lot of DAT fields
// second segment is the SETZ AL
// 0c 1b 04        0f 94 c0
// 20251120 hotfix: 62 1a 04 0f 94 c0
// We want it to always be 1 for now, so changed to B0 01 90 (MOV AL,0x1 NOP)
const unsigned char MODDED_ORIGINAL_BYTES[]
    = { 0x62, 0x1a, 0x04, 0x0f, 0x94, 0xc0 };
const unsigned char MODDED_PATCH_BYTES[]
    = { 0x62, 0x1a, 0x04, 0xb0, 0x01, 0x90 };
const size_t MODDED_PATCH_SIZE = sizeof (MODDED_ORIGINAL_BYTES);
const unsigned char MODDED_MASK[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// // ----------------------------------------------------------------------
// // --- PATCH 2: esv::SavegameManager::ThrowError (Savegame Warnings) ---
// // ----------------------------------------------------------------------

// Some long jump - that's the one - flipping the JZ to JNZ avoids superflous
// "mod changed" prompts: a6 00 84 c0 0f 84 c3 00 00 00 - go 0f 84 -> 0f 85
// 20251120 hotfix:       a5 00 84 c0 0f 84 c3 00 00 00
const unsigned char MODDED2_ORIGINAL_BYTES[]
    = { 0xa5, 0x00, 0x84, 0xc0, 0x0f, 0x84, 0xc3, 0x00, 0x00, 0x00 };
const unsigned char MODDED2_PATCH_BYTES[]
    = { 0xa5, 0x00, 0x84, 0xc0, 0x0f, 0x85, 0xc3, 0x00, 0x00, 0x00 };
const size_t MODDED2_PATCH_SIZE = sizeof (MODDED2_ORIGINAL_BYTES);
const unsigned char MODDED2_MASK[]
    = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// // ----------------------------------------------------------------------
// // --- PATCH 3: new game (ensure mods are not disabled on start) ---
// // ----------------------------------------------------------------------
// This segment controls whether the icon (game was modded) shows up or not
// with the blue gear on main menu - inverting flag 74 -> 75 (JZ -> JNZ)
// 84 c0 74 13 4d 39

// Duds (unknown effect, including start screen)
// af ff 84 c0 74 27

// Controls if mods are active during character creation/new game
//                  4e 01 84 c0 74 0b -> flip JZ to JNZ (74) since first patch inverted result
// 20251120 hotfix: 4e 01 84 c0 74 0b (no change?)
const unsigned char MODDED3_ORIGINAL_BYTES[]
= { 0x4e, 0x01, 0x84, 0xc0, 0x74, 0x0b };
const unsigned char MODDED3_PATCH_BYTES[]
    = { 0x4e, 0x01, 0x84, 0xc0, 0x75, 0x0b };
const size_t MODDED3_PATCH_SIZE = sizeof (MODDED3_ORIGINAL_BYTES);
const unsigned char MODDED3_MASK[]
    = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


// --- Entry Point for LD_PRELOAD ---
__attribute__ ((constructor)) void
init_patch ()
{
  // 1. Get the path of the running executable
  char exe_path[1024];
  ssize_t len = readlink ("/proc/self/exe", exe_path, sizeof (exe_path) - 1);

  if (len != -1)
    {
      // Null-terminate the string
      exe_path[len] = '\0';

      // 2. Check if the executable path contains the target name
      if (strstr (exe_path, EXPECTED_EXE_NAME) == NULL)
        {
          // The running process is NOT the Baldur's Gate 3 executable.
          // Do nothing and exit the constructor gracefully.
          // fprintf (stderr, "Non-bg3 pid, exiting\n");
          return;
        }
    }
  else
    {
      // Failed to read the link (e.g., permission error, though rare for
      // /proc/self/exe) For safety, we can choose to bail out or continue
      // depending on desired robustness. Continuing here might lead to crashes
      // if we scan non-bg3 memory.
      // fprintf (stderr, "Non-bg3 pid, exiting\n");
      return;
    }
  std::cout
      << "--- BG3 Achievement Patcher (Dual Patch) loaded via LD_PRELOAD. ---"
      << std::endl;
  fprintf (
      stderr,
      "--- BG3 Achievement Patcher (Dual Patch) loaded via LD_PRELOAD. ---\n");

  // Get safe search range
  auto range = get_exe_code_range ();
  uintptr_t base_address = range.first;
  size_t search_size = range.second;

  if (base_address == 0 || search_size == 0)
    {
      std::cerr << "Error: Could not determine the executable code range from "
                   "/proc/self/maps."
                << std::endl;
      return;
    }

  fprintf (stderr, "Code Base Address: %p, Size: 0x%zx\n",
           (void *)base_address, search_size);

  // ---------------------------------
  // --- Patch 1: IsModded  ---
  // ---------------------------------
  std::cout
      << "\nAttempting Patch 1: ls::ModuleSettings::IsModded (Achievements)..."
      << std::endl;
  fprintf (stderr, "base_address was: %p\n", base_address);
  void *found_address_1
      = find_pattern ((unsigned char *)base_address, search_size,
                      MODDED_ORIGINAL_BYTES, MODDED_MASK, MODDED_PATCH_SIZE);

  if (found_address_1)
    {
      std::cout << "Patch 1 match" << std::endl;
      apply_patch (found_address_1, MODDED_PATCH_BYTES, MODDED_PATCH_SIZE);
    }
  else
    {
      std::cerr
          << "Patch 1 failed: Pattern not found. Game version mismatch likely."
          << std::endl;
    }

  // // ---------------------------------
  // // --- Patch 2: ThrowError  ---
  // // ---------------------------------
  std::cout << "\nAttempting Patch 2: esv::SavegameManager::ThrowError "
               "(Savegame Warnings)..."
            << std::endl;

  void *found_address_2 = find_pattern ((unsigned char *)base_address,
                                        search_size, MODDED2_ORIGINAL_BYTES,
                                        MODDED2_MASK, MODDED2_PATCH_SIZE);

  if (found_address_2)
    {
      std::cout << "Patch 2 match" << std::endl;
      apply_patch (found_address_2, MODDED2_PATCH_BYTES, MODDED2_PATCH_SIZE);
    }
  else
    {
      std::cerr
          << "Patch 2 failed: Pattern not found. Game version mismatch likely."
          << std::endl;
    }

  // // ---------------------------------
  // // --- Patch 3: New game  ---
  // // ---------------------------------
  std::cout << "\nAttempting Patch 3: new game" << std::endl;

  void *found_address_3 = find_pattern ((unsigned char *)base_address,
                                        search_size, MODDED3_ORIGINAL_BYTES,
                                        MODDED3_MASK, MODDED3_PATCH_SIZE);

  if (found_address_3)
    {
      std::cout << "Patch 3 match" << std::endl;
      apply_patch (found_address_3, MODDED3_PATCH_BYTES, MODDED3_PATCH_SIZE);
    }
  else
    {
      std::cerr
          << "Patch 3 failed: Pattern not found. Game version mismatch likely."
          << std::endl;
    }
}
