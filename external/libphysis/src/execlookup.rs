// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_from_c_string, ffi_to_c_string};
use core::ffi::c_char;
use core::ptr::null;
use physis::execlookup::extract_frontier_url;

#[unsafe(no_mangle)]
pub extern "C" fn physis_extract_frontier_url(launcher_path: *const c_char) -> *const c_char {
    let launcher_path = ffi_from_c_string(launcher_path).unwrap();
    if let Some(frontier_url) = extract_frontier_url(&launcher_path) {
        ffi_to_c_string(&frontier_url)
    } else {
        null()
    }
}
