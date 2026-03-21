// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::ffi_from_c_string;
use physis::patch::ZiPatch;
use std::ffi::c_char;

#[unsafe(no_mangle)]
pub extern "C" fn physis_patch_apply(data_dir: *const c_char, patch_path: *const c_char) -> bool {
    let Some(data_dir) = ffi_from_c_string(data_dir) else {
        return false;
    };

    let Some(patch_path) = ffi_from_c_string(patch_path) else {
        return false;
    };

    ZiPatch::apply(&data_dir, &patch_path).is_ok()
}
