// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::{ffi_to_c_string, physis_Buffer};
use physis::Platform;
use physis::ReadableFile;
use physis::tmb::{Attribute, C013, Tmdh};
use physis::tmb::{TimelineNodeData, Tmb, TmfcData, TmfcRow};
use std::ffi::c_char;
use std::ptr::null;
use std::slice;

#[repr(C)]
#[derive(Clone)]
pub struct physis_Tmb {
    node_count: u32,
    nodes: *const physis_TimelineNode,
}

impl Default for physis_Tmb {
    fn default() -> Self {
        Self {
            node_count: 0,
            nodes: null(),
        }
    }
}

#[repr(C)]
#[derive(Clone)]
pub struct physis_TimelineNode {
    pub data: physis_TimelineNodeData,
}

#[repr(C)]
#[derive(Clone)]
#[allow(dead_code)]
pub enum physis_TimelineNodeData {
    Tmdh(Tmdh),
    Tmac(physis_Tmac),
    Tmtr(physis_Tmtr),
    Tmfc(physis_Tmfc),
    C013(C013),
    Unknown,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Tmac {
    pub id: u16,
    pub time: u16,
    pub tmtr_id_count: u32,
    pub tmtr_ids: *const u16,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Tmtr {
    pub id: u16,
    pub animation_id_count: u32,
    pub animation_ids: *const u16,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Tmfc {
    pub id: u16,
    pub data_count: u32,
    pub data: *const physis_TmfcData,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_TmfcData {
    pub attribute: physis_Attribute,
    pub row_count: u32,
    pub rows: *const TmfcRow,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub enum physis_Attribute {
    PositionX,
    PositionY,
    PositionZ,
    RotationX,
    RotationY,
    RotationZ,
    Unknown,
}

pub fn to_c_tmb(tmb: &Tmb) -> physis_Tmb {
    let mut c_nodes = Vec::new();
    for node in &tmb.nodes {
        c_nodes.push(physis_TimelineNode {
            data: to_c_data(&node.data),
        });
    }

    let tmb = physis_Tmb {
        node_count: c_nodes.len() as u32,
        nodes: c_nodes.as_ptr(),
    };

    std::mem::forget(c_nodes);

    tmb
}

fn to_c_data(tmb: &TimelineNodeData) -> physis_TimelineNodeData {
    match tmb {
        TimelineNodeData::Tmdh(tmdh) => physis_TimelineNodeData::Tmdh(tmdh.clone()),
        TimelineNodeData::Tmac(tmac) => {
            let mut c_tmtr_ids = Vec::new();
            for id in &tmac.tmtr_ids {
                c_tmtr_ids.push(*id);
            }

            let c_tmac = physis_Tmac {
                id: tmac.id,
                time: tmac.time,
                tmtr_id_count: c_tmtr_ids.len() as u32,
                tmtr_ids: c_tmtr_ids.as_ptr(),
            };

            std::mem::forget(c_tmtr_ids);

            physis_TimelineNodeData::Tmac(c_tmac)
        }
        TimelineNodeData::Tmtr(tmtr) => {
            let mut c_animation_ids = Vec::new();
            for id in &tmtr.animation_ids {
                c_animation_ids.push(*id);
            }

            let c_tmfc = physis_Tmtr {
                id: tmtr.id,
                animation_id_count: c_animation_ids.len() as u32,
                animation_ids: c_animation_ids.as_ptr(),
            };

            std::mem::forget(c_animation_ids);

            physis_TimelineNodeData::Tmtr(c_tmfc)
        }
        TimelineNodeData::C013(c013) => physis_TimelineNodeData::C013(c013.clone()),
        TimelineNodeData::Tmfc(tmfc) => {
            let mut c_data = Vec::new();
            for data in &tmfc.data {
                c_data.push(to_c_tmfc_data(data));
            }

            let c_tmfc = physis_Tmfc {
                id: tmfc.id,
                data_count: c_data.len() as u32,
                data: c_data.as_ptr(),
            };

            std::mem::forget(c_data);

            physis_TimelineNodeData::Tmfc(c_tmfc)
        }
        _ => physis_TimelineNodeData::Unknown,
    }
}

fn to_c_tmfc_data(data: &TmfcData) -> physis_TmfcData {
    let attribute = match data.attribute {
        Attribute::PositionX => physis_Attribute::PositionX,
        Attribute::PositionY => physis_Attribute::PositionY,
        Attribute::PositionZ => physis_Attribute::PositionZ,
        Attribute::RotationX => physis_Attribute::RotationX,
        Attribute::RotationY => physis_Attribute::RotationY,
        Attribute::RotationZ => physis_Attribute::RotationZ,
        _ => physis_Attribute::Unknown,
    };

    let c_rows = data.rows.clone();

    let c_data = physis_TmfcData {
        attribute,
        row_count: c_rows.len() as u32,
        rows: c_rows.as_ptr(),
    };

    std::mem::forget(c_rows);

    c_data
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_tmb_parse(platform: Platform, buffer: physis_Buffer) -> physis_Tmb {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(tmb) = Tmb::from_existing(platform, data) {
        to_c_tmb(&tmb)
    } else {
        physis_Tmb::default()
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_tmb_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(tmb) = Tmb::from_existing(platform, data) {
        ffi_to_c_string(&format!("{tmb:#?}"))
    } else {
        null()
    }
}
