// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::scd::Scd;
use std::ffi::c_char;
use std::ptr::null;
use std::slice;

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_scd_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(scd) = Scd::from_existing(platform, data) {
        ffi_to_c_string(&format!("{scd:#?}"))
    } else {
        null()
    }
}
