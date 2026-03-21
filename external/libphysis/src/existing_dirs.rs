// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::ffi_to_c_string;
use physis::existing_dirs::{
    ExistingInstallType, find_existing_game_dirs, find_existing_user_dirs,
};
use std::ffi::c_char;
use std::mem;

/// An existing install location on disk
#[repr(C)]
pub struct physis_ExistingGameDirectory {
    /// The application where this installation was from
    pub install_type: ExistingInstallType,
    /// The path to the "main folder" where "game" and "boot" sits
    pub path: *const c_char,
    /// The latest game/expansion version in this directory
    pub version: *const c_char,
}

#[repr(C)]
pub struct physis_ExistingGameDirectories {
    pub count: u32,
    pub entries: *mut physis_ExistingGameDirectory,
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_find_existing_game_dirs() -> physis_ExistingGameDirectories {
    let dirs = find_existing_game_dirs();

    let mut c_dirs = Vec::new();
    for dir in dirs {
        c_dirs.push(physis_ExistingGameDirectory {
            path: ffi_to_c_string(&dir.path),
            install_type: dir.install_type,
            version: ffi_to_c_string(&dir.version),
        });
    }

    let new_dirs = physis_ExistingGameDirectories {
        count: c_dirs.len() as u32,
        entries: c_dirs.as_mut_ptr(),
    };

    mem::forget(c_dirs);

    new_dirs
}

/// An existing user directory
#[repr(C)]
pub struct physis_ExistingUserDirectory {
    /// The application where this directory was from
    pub install_type: ExistingInstallType,
    /// The path to the user folder
    pub path: *const c_char,
}

#[repr(C)]
pub struct physis_ExistingUserDirectories {
    pub count: u32,
    pub entries: *mut physis_ExistingUserDirectory,
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_find_existing_user_dirs() -> physis_ExistingUserDirectories {
    let dirs = find_existing_user_dirs();

    let mut c_dirs = Vec::new();
    for dir in dirs {
        c_dirs.push(physis_ExistingUserDirectory {
            path: ffi_to_c_string(&dir.path),
            install_type: dir.install_type,
        });
    }

    let new_dirs = physis_ExistingUserDirectories {
        count: c_dirs.len() as u32,
        entries: c_dirs.as_mut_ptr(),
    };

    mem::forget(c_dirs);

    new_dirs
}
