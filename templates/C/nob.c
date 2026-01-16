#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

// ================================================================================================
// CONFIGURATION
// ================================================================================================

#define CC   "cc"                     // Compiler alias for any system. The default is usually good enough.
#define EXT  ".c"
#define SRCDIR  "src"

// Shared flags for all profiles
// NOTE: These flags were tested with gcc. If you plan to use another compiler, like say, clang or
//       tcc, it might be a good idea to make sure they are valid for that compiler.
static const char *const cflags[] = {
        "-Wall",                      // Standard warnings
        "-Wextra",                    // Even more warnings
        "-Wshadow",                   // Warn when a variable shadows another
        "-Wformat=2",                 // Check printf/scanf for type mismatches and null arguments
        "-Wundef",                    // Warn if an undefined macro is used in #if
        "-Wwrite-strings",            // Treat string literals as const `char*`
        "-Wimplicit-fallthrough",     // Warn if switch cases fall through without comment
        "-fno-strict-aliasing",       // Prevent dangerous pointer assumptions
        "-Wstrict-prototypes",        // Force `(void)` for empty parameter lists
        "-Wmissing-prototypes",       // Force global functions to be declared before use
};

// Shared flags for RELEASE and TINY profiles
static const char *const pflags[] = {
        "-DNDEBUG",                   // Disable assertions for performance
        "-Werror",                    // Treat warnings as errors for production
        "-flto",                      // Enable link-time optimization
        "-ffunction-sections",        // Put each function in its own bucket for the linker
        "-fdata-sections",            // Put each piece of data in its own bucket
        "-Wl,--gc-sections",          // Tell the linker to throw away dead code (unused buckets)
        "-Wl,--as-needed",            // Only link libraries that are actually used
};

// ================================================================================================


// Subcommands supported by this build system
typedef enum {
        CMD_BUILD,   // Compile the project
        CMD_RUN,     // Compile and execute
        CMD_CLEAN,   // Remove build artifacts
        CMD_UNKNOWN, // INVALID SUBCOMMAND. LEARN TO READ.
} Subcommand;

// Converts a raw command-line argument string into a Subcommand enum.
// Defaults to CMD_BUILD if no argument is provided.
Subcommand parse_subcommand(const char *arg) {
        if (!arg) return CMD_BUILD;
        if (strcmp(arg, "build") == 0) return CMD_BUILD;
        if (strcmp(arg, "run")   == 0) return CMD_RUN;
        if (strcmp(arg, "clean") == 0) return CMD_CLEAN;
        return CMD_UNKNOWN;
}

// Build profiles that determine optimization and debug levels
typedef enum {
        PROFILE_DEBUG,    // No optimization, full debug info (-O0 -g)
        PROFILE_RELEASE,  // Balanced production optimization (-O2)
        PROFILE_TINY,     // Maximum size reduction           (-Os)
        PROFILE_UNKNOWN,  // INVALID PROFILE. LEARN TO READ.
} Profile;

// Converts a raw command-line argument string into a Profile enum.
// Defaults to PROFILE_DEBUG if no profile is specified.
Profile parse_profile(const char *arg) {
        if (!arg) return PROFILE_DEBUG;
        if (strcmp(arg, "debug")   == 0) return PROFILE_DEBUG;
        if (strcmp(arg, "release") == 0) return PROFILE_RELEASE;
        if (strcmp(arg, "tiny")    == 0) return PROFILE_TINY;
        return PROFILE_UNKNOWN;
}

// Convert a Profile enum back into a string for directory names
const char *profile_to_cstr(Profile p) {
        switch (p) {
        case PROFILE_DEBUG:   return "debug";
        case PROFILE_RELEASE: return "release";
        case PROFILE_TINY:    return "tiny";
        default:              return "unknown";
        }
}

// The primary build logic for the project.
// nob_path:  Path to the current build tool, for an extra rebuild check.
// profile:   The desired build profile from the aforementioned Profile enum.
bool build_project(const char *nob_path, Profile profile) {
        // Construct the output directory path based on the profile, like build/debug or build/tiny
        const char *build_dir = temp_sprintf("build/%s", profile_to_cstr(profile));
        if (!mkdir_if_not_exists(build_dir)) return false;

        const char *output_path = temp_sprintf("%s/main", build_dir); // Where the binary will live

        File_Paths src_files = { 0 };    // Gather all files from the source directory
        if (!read_entire_dir(SRCDIR, &src_files)) return false;

        File_Paths input_paths = { 0 };  // Prepare the list of dependencies for the rebuild check

        // Our nob.c is now a build dependency. You changed it? Rebuild yourself.
        nob_da_append(&input_paths, nob_path);

        // Filter the source directory for files matching our defined `EXT`ension
        for (size_t i = 0; i < src_files.count; ++i) {
                const char *name = src_files.items[i];
                if (sv_end_with(sv_from_cstr(name), EXT)) {
                        const char *path = temp_sprintf("%s/%s", SRCDIR, name);
                        nob_da_append(&input_paths, path);
                }
        }

        // Is the binary missing? Is any input file newer than it? Believe it or not, rebuild.
        if (needs_rebuild(output_path, input_paths.items, input_paths.count)) {
                nob_log(INFO, "--- REBUILDING ---");

                Cmd cmd = { 0 };      // Initialize the command builder
                cmd_append(&cmd, CC);

                // Apply shared cflags
                for (size_t i = 0; i < NOB_ARRAY_LEN(cflags); ++i) {
                        cmd_append(&cmd, cflags[i]);
                }

                // Apply shared pflags for performance
                if (profile == PROFILE_RELEASE || profile == PROFILE_TINY) {
                        for (size_t i = 0; i < NOB_ARRAY_LEN(pflags); ++i) {
                                cmd_append(&cmd, pflags[i]);
                        }
                }

                // Apply profile-specific flags
                switch (profile) {
                case PROFILE_DEBUG:
                        nob_log(INFO, "Profile: DEBUG");
                        cmd_append(
                                &cmd,
                                "-O0",
                                "-ggdb3"  // Maximum amount of debug information!
                        );
                        break;
                case PROFILE_RELEASE:
                        nob_log(INFO, "Profile: RELEASE");
                        cmd_append(&cmd, "-O2");
                        break;
                case PROFILE_TINY:
                        nob_log(INFO, "Profile: TINY");
                        cmd_append(
                                &cmd,
                                "-Os",
                                "-Wl,-z,noseparate-code",  // `ld` packs code and data more tightly
                                "-Wl,--build-id=none",     // Remove unique build hash data
                                "-Wl,--strip-all"          // Omit all symbol info from output
                        );
                        break;
                default:
                        nob_log(ERROR, "Unknown profile during build.");
                        return false;
                }

                // Pass source files to the compiler. Remember to filter out the nob binary!
                for (size_t i = 0; i < input_paths.count; ++i) {
                        if (strcmp(input_paths.items[i], nob_path) == 0) continue;
                        cmd_append(&cmd, input_paths.items[i]);
                }

                // Where the binary will live- didn't I say this before?
                cmd_append(&cmd, "-o", output_path);

                // Execute the command and capture the result
                bool ok = cmd_run_sync(cmd);
                cmd_free(cmd);
                return ok;
        } else {
                // If nothing has changed, skip the build
                nob_log(INFO, "Project is up to date (%s).", profile_to_cstr(profile));
                return true;
        }
}

int main(int argc, char *argv[]) {
        GO_REBUILD_URSELF(argc, argv);  // Has `nob.c` changed since last compiled? Yes? Rebuild yourself.

        // Store the program name ./nob for later use
        const char *program_name = shift_args(&argc, &argv);

        // Parse the subcommand
        char *subcommand_str = NULL;
        if (argc > 0) subcommand_str = shift_args(&argc, &argv);
        Subcommand subcommand = parse_subcommand(subcommand_str);

        if (!mkdir_if_not_exists("build")) return 1;  // Ensure the base build directory exists

        // Route the subcommand to the appropriate logic
        switch (subcommand) {
        case CMD_BUILD: {
                // Parse the profile
                char *profile_str = NULL;
                if (argc > 0) profile_str = shift_args(&argc, &argv);
                Profile profile = parse_profile(profile_str);

                if (!build_project(program_name, profile)) return 1;
        } break;

        case CMD_RUN: {
                // Parse the profile, then build and execute the binary
                char *profile_str = NULL;
                if (argc > 0) profile_str = shift_args(&argc, &argv);
                Profile profile = parse_profile(profile_str);

                if (!build_project(program_name, profile)) return 1;

                nob_log(INFO, "--- RUNNING ---");
                Cmd cmd = { 0 };
                cmd_append(&cmd, temp_sprintf("build/%s/main", profile_to_cstr(profile)));
                if (!cmd_run_sync(cmd)) return 1;
                cmd_free(cmd);
        } break;

        case CMD_CLEAN: {
                // Nuke the build artifacts directory
                nob_log(INFO, "JANNY DUTY");
                Cmd cmd = { 0 };
                cmd_append(&cmd, "rm", "-rf", "build");
                if (!cmd_run_sync(cmd)) return 1;
                cmd_free(cmd);
        } break;

        case CMD_UNKNOWN: {
                nob_log(ERROR, "Unknown subcommand: %s", subcommand_str);
                return 1;
        }
        }

        return 0;
}
