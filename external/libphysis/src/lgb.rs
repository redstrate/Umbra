// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::layer::{physis_Layer, to_c_layer};
use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::lgb::Lgb;
use std::ffi::c_char;
use std::ptr::null;
use std::slice;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_LayerChunk {
    layers: *const physis_Layer,
    num_layers: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_LayerGroup {
    chunks: *const physis_LayerChunk,
    num_chunks: u32,
}

impl Default for physis_LayerGroup {
    fn default() -> Self {
        Self {
            chunks: null(),
            num_chunks: 0,
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_lgb_parse(platform: Platform, buffer: physis_Buffer) -> physis_LayerGroup {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(lgb) = Lgb::from_existing(platform, data) {
        let mut c_chunks = vec![];

        for chunk in &lgb.chunks {
            let mut c_layers = vec![];

            for layer in &chunk.layers {
                c_layers.push(to_c_layer(layer));
            }

            c_chunks.push(physis_LayerChunk {
                layers: c_layers.as_ptr(),
                num_layers: c_layers.len() as u32,
            });

            std::mem::forget(c_layers);
        }

        let lgb = physis_LayerGroup {
            chunks: c_chunks.as_ptr(),
            num_chunks: c_chunks.len() as u32,
        };

        std::mem::forget(c_chunks);

        lgb
    } else {
        physis_LayerGroup::default()
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_lgb_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(lgb) = Lgb::from_existing(platform, data) {
        ffi_to_c_string(&format!("{lgb:#?}"))
    } else {
        null()
    }
}
