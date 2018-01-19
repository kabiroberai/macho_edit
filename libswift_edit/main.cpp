//
//  main.cpp
//  libswift_edit
//
//  Created by Kabir Oberai on 19/01/18.
//  Copyright © 2018 Kabir Oberai. All rights reserved.
//

#include <iostream>

#include "macho.h"

#define VERSION(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))

static void edit_command(LoadCommand load_command, MachO macho) {
	// Skip all except dylib_commands
	switch (load_command.cmd) {
		case LC_ID_DYLIB:
		case LC_LOAD_DYLIB:
		case LC_LOAD_WEAK_DYLIB:
		case LC_REEXPORT_DYLIB: break;
		default: return;
	}
	
	// get the load command's dylib struct
	auto lc = (dylib_command *)load_command.raw_lc;
	auto &dylib = lc->dylib;
	
	// Skip commands that don't start with "@rpath/libswift"
	auto dylib_name = load_command.get_lc_str(dylib.name);
	std::string prefix = "@rpath/libswift";
	if (dylib_name.compare(0, prefix.length(), prefix) != 0) return;
	
	// Set the command's compatibility version to 1.0.0
	dylib.compatibility_version = VERSION(1, 0, 0);
	
	// Write the edited command to disk
	macho.write_load_command(load_command);
}

static void edit_file(const char *filename) {
	auto macho = MachO(filename);
	
	// Loop through all archs
	for (uint32_t i = 0; i < macho.n_archs; i++) {
		auto arch = macho.archs[i];
		auto load_commands = arch.load_commands;
		// Loop through all load commands
		for (auto load_command : load_commands) {
			// Edit them if necessary
			edit_command(load_command, macho);
		}
	}
}

int main(int argc, const char *argv[]) {
	if(argc < 2) {
		std::cout << "Usage: " << argv[0] << " binary_path...\n";
		exit(1);
	}
	
	// Edit each file passed in
	for (int i = 1; i < argc; i++) {
		edit_file(argv[i]);
	}
	
	return 0;
}


