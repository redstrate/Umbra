// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::savedata::chardat::{CharacterData, CustomizeData};
use std::ffi::c_char;
use std::slice;

#[repr(C)]
pub struct physis_CharacterData {
    version: u32,
    customize: CustomizeData,
    timestamp: u32,
    comment: *const c_char,
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_chardat_parse(buffer: physis_Buffer) -> physis_CharacterData {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    let chardat = CharacterData::from_existing(data).unwrap();

    physis_CharacterData {
        version: chardat.version,
        customize: chardat.customize,
        timestamp: chardat.timestamp,
        comment: ffi_to_c_string(&chardat.comment),
    }
}
