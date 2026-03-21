// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::cmp::{CMP, RacialScalingParameters};
use physis::race::{Race, Tribe};
use std::ffi::c_char;
use std::ptr::{null, null_mut};
use std::slice;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_CMP {
    p_ptr: *mut CMP,
}

impl Default for physis_CMP {
    fn default() -> Self {
        Self { p_ptr: null_mut() }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_cmp_parse(platform: Platform, buffer: physis_Buffer) -> physis_CMP {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(cmp) = CMP::from_existing(platform, data) {
        physis_CMP {
            p_ptr: Box::leak(Box::new(cmp)),
        }
    } else {
        physis_CMP::default()
    }
}

// adapted from https://github.com/xivdev/Penumbra/blob/master/Penumbra/Meta/Files/CmpFile.cs#L50
fn get_rsp_index(tribe: Tribe) -> i32 {
    match tribe {
        Tribe::Midlander => 0,
        Tribe::Highlander => 1,
        Tribe::Wildwood => 10,
        Tribe::Duskwight => 11,
        Tribe::Plainsfolk => 20,
        Tribe::Dunesfolk => 21,
        Tribe::Seeker => 30,
        Tribe::Keeper => 31,
        Tribe::SeaWolf => 40,
        Tribe::Hellsguard => 41,
        Tribe::Raen => 50,
        Tribe::Xaela => 51,
        Tribe::Hellion => 60,
        Tribe::Lost => 61,
        Tribe::Rava => 70,
        Tribe::Veena => 71,
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_cmp_get_racial_scaling_parameters(
    cmp: physis_CMP,
    _: Race,
    tribe: Tribe,
) -> RacialScalingParameters {
    unsafe { (&(*cmp.p_ptr).parameters)[get_rsp_index(tribe) as usize] }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_cmp_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(cmp) = CMP::from_existing(platform, data) {
        ffi_to_c_string(&format!("{cmp:#?}"))
    } else {
        null()
    }
}
