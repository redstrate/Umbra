// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use std::mem;
use std::os::raw::c_char;
use std::ptr::null;

use physis::patchlist::{PatchList, PatchListType};

use crate::{ffi_from_c_string, ffi_to_c_string};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_PatchEntry {
    url: *const c_char,
    version: *const c_char,
    hash_count: u64,
    hashes: *const *const c_char,
    hash_block_size: i64,
    length: i64,
    size_on_disk: i64,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_PatchList {
    patch_length: u64,
    num_entries: i32,
    entries: *const physis_PatchEntry,
    total_size_downloaded: i64,
}

impl Default for physis_PatchList {
    fn default() -> Self {
        Self {
            patch_length: 0,
            num_entries: 0,
            entries: null(),
            total_size_downloaded: 0,
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_parse_patchlist(
    patch_type: PatchListType,
    encoded: *const c_char,
) -> physis_PatchList {
    if let Some(r_path) = ffi_from_c_string(encoded) {
        let patch_list = PatchList::from_string(patch_type, &r_path);

        let mut c_patches = vec![];

        for entry in &patch_list.patches {
            let mut c_hashes = vec![];

            for hash in &entry.hashes {
                c_hashes.push(ffi_to_c_string(hash));
            }

            c_patches.push(physis_PatchEntry {
                url: ffi_to_c_string(&entry.url),
                version: ffi_to_c_string(&entry.version),
                hash_count: c_hashes.len() as u64,
                hashes: c_hashes.as_mut_ptr(),
                hash_block_size: entry.hash_block_size,
                length: entry.length,
                size_on_disk: entry.size_on_disk,
            });

            mem::forget(c_hashes);
        }

        let pl = physis_PatchList {
            patch_length: patch_list.patch_length,
            num_entries: c_patches.len() as i32,
            entries: c_patches.as_mut_ptr(),
            total_size_downloaded: patch_list.total_size_downloaded(),
        };

        mem::forget(c_patches);

        return pl;
    }

    physis_PatchList::default()
}
