// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use physis::blowfish::{SqexArgBlowfish, SteamTicketBlowfish};
use std::os::raw::c_uint;
use std::{mem, slice};

#[unsafe(no_mangle)]
pub extern "C" fn physis_blowfish_initialize(
    key: *mut u8,
    key_size: c_uint,
) -> *mut SqexArgBlowfish {
    let data = unsafe { slice::from_raw_parts(key, key_size as usize) };
    Box::into_raw(Box::new(SqexArgBlowfish::new(data)))
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_blowfish_free(blowfish: *mut SqexArgBlowfish) {
    unsafe {
        drop(Box::from_raw(blowfish));
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_blowfish_encrypt(
    blowfish: &SqexArgBlowfish,
    in_data: *mut u8,
    in_data_size: c_uint,
    out_data: &mut *mut u8,
    out_data_size: *mut u32,
) -> bool {
    let in_data = unsafe { slice::from_raw_parts(in_data, in_data_size as usize) };

    let result = blowfish.encrypt(in_data);

    match result {
        Some(mut out_data_vec) => {
            unsafe {
                *out_data = out_data_vec.as_mut_ptr();
                *out_data_size = out_data_vec.len() as u32;
            }

            mem::forget(out_data_vec);

            true
        }
        None => false,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physism_blowfish_decrypt(
    blowfish: &SqexArgBlowfish,
    in_data: *mut u8,
    in_data_size: c_uint,
    out_data: &mut *mut u8,
    out_data_size: *mut u32,
) -> bool {
    let in_data = unsafe { slice::from_raw_parts(in_data, in_data_size as usize) };

    let result = blowfish.decrypt(in_data);

    match result {
        Some(mut out_data_vec) => {
            unsafe {
                *out_data = out_data_vec.as_mut_ptr();
                *out_data_size = out_data_vec.len() as u32;
            }

            mem::forget(out_data_vec);

            true
        }
        None => false,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_steamticket_blowfish_initialize(
    key: *mut u8,
    key_size: c_uint,
) -> *mut SteamTicketBlowfish {
    let data = unsafe { slice::from_raw_parts(key, key_size as usize) };
    Box::into_raw(Box::new(SteamTicketBlowfish::new(data)))
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_steamticket_physis_blowfish_free(blowfish: *mut SteamTicketBlowfish) {
    unsafe {
        drop(Box::from_raw(blowfish));
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_steamticket_blowfish_encrypt(
    blowfish: &SteamTicketBlowfish,
    in_data: *mut u8,
    in_data_size: c_uint,
) {
    let in_data = unsafe { slice::from_raw_parts_mut(in_data, in_data_size as usize) };

    blowfish.encrypt(in_data);
}
