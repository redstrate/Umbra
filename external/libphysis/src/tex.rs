// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::physis_Buffer;
use physis::Platform;
use physis::ReadableFile;
use physis::tex::{Texture, TextureFormat};
use physis::tex::TextureType;
use std::ptr::null_mut;
use std::{mem, slice};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Texture {
    texture_type: TextureType,
    format: TextureFormat,
    width: u32,
    height: u32,
    depth: u32,
    rgba_size: u32,
    rgba: *mut u8,
}

impl Default for physis_Texture {
    fn default() -> Self {
        Self {
            texture_type: TextureType::TwoDimensional,
            format: TextureFormat::A8_UNORM,
            width: 0,
            height: 0,
            depth: 0,
            rgba_size: 0,
            rgba: null_mut(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_texture_parse(
    platform: Platform,
    buffer: physis_Buffer,
) -> physis_Texture {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(mut texture) = Texture::from_existing(platform, data) {
        let tex = physis_Texture {
            texture_type: texture.texture_type,
            format: texture.format,
            width: texture.width,
            height: texture.height,
            depth: texture.depth,
            rgba_size: texture.rgba.len() as u32,
            rgba: texture.rgba.as_mut_ptr(),
        };

        mem::forget(texture.rgba);

        tex
    } else {
        physis_Texture::default()
    }
}
