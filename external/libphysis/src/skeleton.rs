// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::skeleton::Skeleton;
use std::os::raw::c_char;
use std::ptr::{null, null_mut};
use std::{mem, slice};

#[repr(C)]
pub struct physis_Bone {
    pub index: u32,
    pub name: *const c_char,
    pub parent_bone: *mut physis_Bone,
    pub parent_index: u32,

    pub position: [f32; 3],
    pub rotation: [f32; 4],
    pub scale: [f32; 3],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Skeleton {
    num_bones: u32,
    bones: *mut physis_Bone,
    root_bone: *mut physis_Bone,
}

impl Default for physis_Skeleton {
    fn default() -> Self {
        Self {
            num_bones: 0,
            bones: null_mut(),
            root_bone: null_mut(),
        }
    }
}

fn convert_skeleton(skeleton: &Skeleton) -> physis_Skeleton {
    let mut c_bones = vec![];

    for (i, bone) in skeleton.bones.iter().enumerate() {
        c_bones.push(physis_Bone {
            index: i as u32,
            name: ffi_to_c_string(&bone.name),
            parent_bone: null_mut(),
            parent_index: bone.parent_index as u32,
            position: bone.position,
            rotation: bone.rotation,
            scale: bone.scale,
        })
    }

    for (i, bone) in skeleton.bones.iter().enumerate() {
        if bone.parent_index != -1 {
            c_bones[i].parent_bone = &mut c_bones[bone.parent_index as usize] as *mut physis_Bone;
        }
    }

    let skel = physis_Skeleton {
        num_bones: c_bones.len() as u32,
        bones: c_bones.as_mut_ptr(),
        root_bone: &mut c_bones[0] as *mut physis_Bone,
    };

    mem::forget(c_bones);

    skel
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_skeleton_parse(
    platform: Platform,
    buffer: physis_Buffer,
) -> physis_Skeleton {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(skeleton) = Skeleton::from_existing(platform, data) {
        convert_skeleton(&skeleton)
    } else {
        physis_Skeleton::default()
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_skeleton_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(sklb) = Skeleton::from_existing(platform, data) {
        ffi_to_c_string(&format!("{sklb:#?}"))
    } else {
        null()
    }
}
